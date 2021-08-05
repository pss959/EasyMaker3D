#include "Controller.h"

#include "Event.h"

Controller::Controller(Hand hand) : hand_(hand) {
}

Controller::~Controller() {
}

bool Controller::HandleEvent(const Event &event) {
    // XXXX
    return false;
}
