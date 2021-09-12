#include "Math/Animation.h"

float Dampen(float t) {
    // This is a cubic increase from t in (0,.5), then cubic decrease
    // in (.5,1).
    float d;
    if (t <= .5f) {
        // 0 => 0; .5 => .5, cubic between.
        d = 2.f * t;           // d in (0,1).
        d = .5f * d * d * d;
    }
    else {
        // .5 => .5; 1 => 1, cubic between.
        d = 2.f * (1.f - t);       // d reversed in (0,1).
        d = 1.f - .5f * d * d * d;
    }
    return d;
}
