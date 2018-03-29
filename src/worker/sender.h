#ifndef SENDER_H
#define SENDER_H

#define WAIT_TIME_REQUEST     5
#define WAIT_TIME_LOGIN       3
#define WAIT_TIME_KEEPALIVE   2
#define WAIT_TIME_LOGOUT      2

#define TIME_OUT_REQUEST      0
#define TIME_OUT_LOGIN        11
#define TIME_OUT_KEEPALIVE    90
#define TIME_OUT_LOGOUT       3

typedef enum _TaskType {
    TASK_SEND_REQUEST,
    TASK_SEND_LOGIN,
    TASK_SEND_LOGOUT,
    TASK_SEND_PING,
    TASK_SEND_KEEPALIVE
} TaskType;

typedef struct {
    TaskType type;
    unsigned int delay;
    unsigned int wait;
    long timeout;
} Task;

#include "controller.h"
#include <pthread.h>
#include <sys/time.h>

class Controller;

class Sender {
private:
    Task task;
    Controller *controller;
    // Thread control
    bool enabled;
    bool started;
    bool working;
    // Mutex control
    bool init;
    struct timespec abs_time;
    pthread_t send_thread;
    pthread_mutex_t mtx_do_task, mtx_response;
    pthread_cond_t cond_do_task, cond_response;
    
public:
    Sender(Controller *controller);
    ~Sender();
    bool start();
    void execute(TaskType task_id);
    void execute(TaskType task_id, unsigned int delay);
    void execute(TaskType task_id, unsigned int delay, unsigned int wait, long timeout, bool prior);
    void notify();
    static void* doTask(void *sender);
    struct timespec* wait_sec(unsigned int sec);
    void close();
};
#endif
