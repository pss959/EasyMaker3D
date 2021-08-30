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
        // const SG::Ray ray(SG::Point3f(0, 0, 60), SG::Vector3f(0, 0, -1)); // XXXX
        std::cerr << "XXXX Ray: " << ray.ToString() << "\n";
        context_->debug_line->SetEndpoints(ray.origin,
                                           ray.origin + 100 * ray.direction);

        SG::Hit hit = SG::Intersector::IntersectScene(*context_->scene, ray);
        if (hit.IsValid()) {
            context_->debug_text->SetText(ray.ToString() + " =>\n" +
                                          hit.path.ToString());
            context_->debug_sphere->SetTranslation(SG::Vector3f(hit.point));
        }
        else {
            context_->debug_text->SetText("No Hit");
        }
    }
    return false;
}
