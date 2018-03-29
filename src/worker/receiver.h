#ifndef RECEIVER_H
#define RECEIVER_H

#include "controller.h"

class Controller;

class Receiver {
    
private:
    Controller *controller;
    
public:
    Receiver(Controller *controller);
    bool enabled;
    void loop();
};

#endif
