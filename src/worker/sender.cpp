#include "sender.h"
#include "common/log.h"
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <limits.h>

Sender::Sender(Controller *controller) {
    this->controller = controller;
    this->enabled = true;
    this->working = false;
    this->started = false;
    this->init = false;
}

bool Sender::start() {
    if (pthread_mutex_init(&mtx_do_task, NULL) != 0 ||
        pthread_mutex_init(&mtx_response, NULL) != 0 ||
        pthread_cond_init(&cond_do_task, NULL) != 0 ||
        pthread_cond_init(&cond_response, NULL) != 0) {
        Log::e("pthread: fail to init mutex or cond");
        return false;
    }
    init = true;
    pthread_attr_t attr;
    if (pthread_attr_init(&attr) != 0) {
        Log::e("pthread: fail to init attr");
        return false;
    }
    if (pthread_attr_setstacksize(&attr, PTHREAD_STACK_MIN) != 0) {
        Log::e("pthread: fail to set stack size");
        pthread_attr_destroy(&attr);
        return false;
    }
    if (pthread_create(&send_thread, &attr, doTask, this) != 0) {
        Log::e("pthread: fail to create send thread");
        pthread_attr_destroy(&attr);
        return false;
    }
    pthread_attr_destroy(&attr);
    while (!started)
        usleep(100 * 1000);
    execute(TASK_SEND_REQUEST);
    return true;
}

void Sender::execute(TaskType task_id) {
    execute(task_id, 0);
}

void Sender::execute(TaskType task_id, unsigned int delay) {
    unsigned int wait = 1;
    long timeout = 0;
    bool prior = false;
    switch (task_id) {
        case TASK_SEND_REQUEST:
            wait = WAIT_TIME_REQUEST;
            timeout = 0;
            prior = true;
            break;
        case TASK_SEND_LOGIN:
            wait = WAIT_TIME_LOGIN;
            timeout = TIME_OUT_LOGIN;
            break;
        case TASK_SEND_LOGOUT:
            wait = WAIT_TIME_LOGOUT;
            timeout = TIME_OUT_LOGOUT;
            prior = true;
            break;
        case TASK_SEND_KEEPALIVE:
            wait = WAIT_TIME_KEEPALIVE;
            timeout = TIME_OUT_KEEPALIVE;
            break;
        default:
            break;
    }
    execute(task_id, delay, wait, timeout, prior);
}

void Sender::execute(TaskType task_id, unsigned int delay, unsigned int wait, long timeout, bool prior) {
    if (prior && working)
        notify();
    pthread_mutex_lock(&mtx_do_task);
    this->task.type = task_id;
    this->task.delay = delay;
    this->task.wait = wait;
    this->task.timeout = timeout == 0 ? 0 : (time(NULL) + timeout);
    pthread_cond_signal(&cond_do_task);
    this->working = true;
    pthread_mutex_unlock(&mtx_do_task);
}

void Sender::notify() {
    pthread_mutex_lock(&mtx_response);
    pthread_cond_signal(&cond_response);
    this->working = false;
    pthread_mutex_unlock(&mtx_response);
}

void* Sender::doTask(void *s) {
    Sender *sender = (Sender *)s;
    Log::i("thread: send thread %d started", sender->send_thread);
    pthread_mutex_lock(&sender->mtx_do_task);
    pthread_mutex_lock(&sender->mtx_response);
    sender->started = true;
    while (true) {
        //Log::i("thread: waiting to do task");
        pthread_cond_wait(&sender->cond_do_task, &sender->mtx_do_task);
        //Log::i("thread: begin to do task");
        if (!sender->enabled) {
            break;
        }
        Task *task = &sender->task;
        if (pthread_cond_timedwait(&sender->cond_response, &sender->mtx_response, sender->wait_sec(task->delay)) == 0) {
            continue;
        }
        // Execute task
        int try_count = 1;
        while (true) {
            if (sender->controller->should_ping()) {
                sender->controller->send_task(TASK_SEND_PING);
                sleep(1);
            }
            sender->controller->send_task(task->type);
            int result = pthread_cond_timedwait(&sender->cond_response,
                                                &sender->mtx_response,
                                                sender->wait_sec(task->wait));
            if (result == 0)
                break;
            else if (result == ETIMEDOUT)
                sender->controller->on_wait_timedout(task->wait, try_count++);
            
            // Task timeout
            if (task->timeout != 0 && task->timeout - time(NULL) <= 0) {
                if (sender->controller->should_exit()) {
                    sender->controller->try_exit();
                    break;
                }
                // We should restart
                sender->controller->restart();
                Log::e("auth: server not responding, try to restart");
                // Make new request task
                task->type = TASK_SEND_REQUEST;
                task->delay = 0;
                task->wait = WAIT_TIME_REQUEST;
                task->timeout = TIME_OUT_REQUEST;
                try_count = 1;
            }
        }
    }
    Log::i("thread: exiting send thread %d", sender->send_thread);
    sender->started = false;
    pthread_mutex_unlock(&sender->mtx_response);
    pthread_mutex_unlock(&sender->mtx_do_task);
    return 0;
}

void Sender::close() {
    if (working)
        notify();
    pthread_mutex_lock(&mtx_do_task);
    this->enabled = false;
    pthread_cond_signal(&cond_do_task);
    pthread_mutex_unlock(&mtx_do_task);
    pthread_join(send_thread, NULL);
}

struct timespec* Sender::wait_sec(unsigned int sec) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    abs_time.tv_sec = tv.tv_sec + sec;
    abs_time.tv_nsec = tv.tv_usec * 1000;
    return &abs_time;
}

Sender::~Sender() {
    if (!init)
        return;
    pthread_mutex_destroy(&mtx_do_task);
    pthread_mutex_destroy(&mtx_response);
    pthread_cond_destroy(&cond_do_task);
    pthread_cond_destroy(&cond_response);
}
