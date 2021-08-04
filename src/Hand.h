#pragma once

#include <assert.h>

#include <string>

//! The Hand enum represents the left or right hand, usually referring to a VR
//! controller.
enum class Hand {
    kLeft,
    kRight,
};

//! Convenience that converts a Hand enum value to an integer to use for
//! indexing, for example.
uint32_t HandIndex(Hand hand) { return static_cast<int>(hand); }

//! Convenience that converts a hand index to a lower-case string.
std::string HandName(int index) {
    assert(index == 0 || index == 1);
    return index == 0 ? "left" : "right";
}

//! Convenience that converts a Hand enum value to a lower-case string.
std::string HandName(Hand hand) {
    return HandName(HandIndex(hand));
}
