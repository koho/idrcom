/*
 Copyright (C) 2018 Guohao Tan <koho@flyinglight.org>

 This is free software, licensed under the GNU General Public License v3.
 See /LICENSE for more information.
*/

#include "common/log.h"
#include "common/config.h"
#include "service/service.h"
#include "worker/receiver.h"
#include "worker/sender.h"
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <arpa/inet.h>
#include <signal.h>

#define TEXT_HELP "Usage: idrcom [options]\n"\
"Options:\n"\
"  -c FILE    Config file to start authentication\n"\
"             Default is /etc/idrcom.conf\n"\
"  -b         Run in background\n"\
"  -d DELAY   Start service after seconds"

static Controller *_controller = NULL;

void on_exit(int sig) {
    if (_controller)
        _controller->on_interrupt();
}

int main(int argc, char * argv[]) {
    const char *config_file = DEFAULT_CONFIG_FILE;
    bool background = false;
    int delay = 0;
    
    int ch;
    while ((ch = getopt(argc, argv, "c:d:bh")) != -1) {
        switch (ch) {
            case 'c':
                config_file = optarg;
                break;
            case 'd':
                delay = atoi(optarg);
                break;
            case 'b':
                background = true;
                break;
            case 'h':
                Log::i(TEXT_HELP);
                return 0;
            case '?':
                Log::e(TEXT_HELP);
                return 1;
        }
    }
    if (background) {
        if (daemon(0, 1) == -1) {
            Log::e("daemon: %s", strerror(errno));
            return 1;
        }
        if (Log::to(config_file) != 0)
            return 1;
    }
    delay = delay >= 0 ? delay : 0;
    sleep(delay);
    
    struct Config config;
    if (read_config(&config, config_file) != 0 || !ensure_config(&config))
        return 1;
    if (get_interface_info(&config) != 0)
        return 1;
    Log::i("config: on %s, MAC %02x:%02x:%02x:%02x:%02x:%02x",
           config.interface,
           config.mac[0], config.mac[1], config.mac[2],
           config.mac[3], config.mac[4], config.mac[5]);
    struct in_addr serv_ip = { config.serv_ip };
    Log::i("config: user %s, %s:%d -> %s:%d",
           config.user_name,
           config.host_ip_str, config.host_port,
           inet_ntoa(serv_ip), config.serv_port);
    
    // Main process
    Service service(&config);
    Controller controller(&service);
    Receiver receiver(&controller);
    Sender sender(&controller);
    controller.bind(&receiver, &sender);
    
    // Try to start sender thread
    if (!sender.start())
        return 1;
    _controller = &controller;
    signal(SIGINT, on_exit);
    
    // Receiver is ready to loop
    receiver.loop();
    
    sender.close();
    Log::close();
    return 0;
}
