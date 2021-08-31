#include "MainHandler.h"

#include "Event.h"
#include "SG/Intersector.h"
#include "SG/Line.h"

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
        /* XXXX
        SG::Point3f e0 = ray.origin;
        SG::Point3f e1 = ray.origin + 100 * ray.direction;
        context_->debug_line->SetEndpoints(e0, //  + SG::Vector3f::AxisX(),
                                           e1 + SG::Vector3f::AxisX());
        */

        SG::Hit hit = SG::Intersector::IntersectScene(*context_->scene, ray);
        if (hit.IsValid()) {
            context_->debug_sphere->SetTranslation(SG::Vector3f(hit.point));
            // XXXX
            //context_->debug_text->SetText(ray.ToString() + " =>\n" +
            // hit.path.ToString());
        }
        else {
            //context_->debug_text->SetText("No Hit");
        }
    }
    return false;
}
