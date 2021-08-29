#include "MainHandler.h"

#include "Event.h"
#include "SG/Intersector.h"

MainHandler::MainHandler() {
}

MainHandler::~MainHandler() {
}

bool MainHandler::HandleEvent(const Event &event) {
    if (! context_)
        return false;

    // XXXX
    if (event.flags.Has(Event::Flag::kPosition2D)) {
        const SG::Ray ray = context_->frustum.BuildRay(event.position2D);
        SG::Hit hit = SG::Intersector::IntersectScene(*context_->scene, ray);
        if (hit.IsValid()) {
            context_->debug_text->SetText("HIT");
        }
        else {
            context_->debug_text->SetText(".");
        }
    }
    return false;
}
