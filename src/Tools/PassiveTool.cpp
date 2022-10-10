#include "Tools/PassiveTool.h"

#include <ion/math/transformutils.h>

#include "Models/Model.h"
#include "SG/Search.h"
#include "Util/String.h"

void PassiveTool::Attach() {
    // Create the parts first if not already done.
    if (! corners_[0])
        CreateParts_();

    ASSERT(GetModelAttachedTo());
    const Model &model = *GetModelAttachedTo();

    // Make sure the Model's bounds are up to date. This will also ensure that
    // its offset translation is updated if necessary.
    model.GetBounds();

    // Rotate and position to match the Model in stage coordinates.
    const Matrix4f lsm = GetStageCoordConv().GetLocalToRootMatrix();
    SetRotation(model.GetRotation());
    SetTranslation(lsm * model.GetTranslation());

    // Update the scales and positions of the corners and edges based on
    // the Model size.
    const Vector3f scale = .5f * model.GetScaledBounds().GetSize();
    UpdateCorners_(scale);
    UpdateEdges_(scale);
}

void PassiveTool::Detach() {
    // Nothing to do here.
}

void PassiveTool::CreateParts_() {
    // Find the parts.
    auto corner = SG::FindNodeUnderNode(*this, "Corner");
    auto edge   = SG::FindNodeUnderNode(*this, "Edge");

    // Create instances of all corners and edges.
    for (int c = 0; c < 8; ++c) {
        const std::string name = "Corner_" + Util::ToString(c);
        corners_[c] = corner->CloneTyped<SG::Node>(true, name);
        AddChild(corners_[c]);
    }
    for (int e = 0; e < 12; ++e) {
        const std::string name = "Edge_" + Util::ToString(e);
        edges_[e] = edge->CloneTyped<SG::Node>(true, name);
        AddChild(edges_[e]);
    }

    // Save the original edge size for scaling math.
    edge_size_ = edge->GetBounds().GetSize()[0];

    // Turn off the original parts.
    corner->SetEnabled(false);
    edge->SetEnabled(false);
}

void PassiveTool::UpdateCorners_(const Vector3f &model_scale) {
    for (int c = 0; c < 8; ++c) {
        corners_[c]->SetTranslation(
            Vector3f((c & 4) ? model_scale[0] : -model_scale[0],
                     (c & 2) ? model_scale[1] : -model_scale[1],
                     (c & 1) ? model_scale[2] : -model_scale[2]));
    }
}

void PassiveTool::UpdateEdges_(const Vector3f &model_scale) {
    for (int e = 0; e < 12; ++e) {
        const auto &edge = edges_[e];

        // The edge is parallel to one of the 3 axes. The edge is scaled only
        // in that dimension.
        Vector3f edge_scale(1, 1, 1);
        Vector3f edge_trans(0, 0, 0);

        // The first 4 edges are parallel to the X axis.
        if (e < 4) {
            edge_scale[0] = 2 * model_scale[0] / edge_size_;
            edge_trans[1] = (e & 2) ? model_scale[1] : -model_scale[1];
            edge_trans[2] = (e & 1) ? model_scale[2] : -model_scale[2];
        }
        // The next 4 are parallel to the Y axis.
        else if (e < 8) {
            edge_scale[1] = 2 * model_scale[1] / edge_size_;
            edge_trans[0] = (e & 2) ? model_scale[0] : -model_scale[0];
            edge_trans[2] = (e & 1) ? model_scale[2] : -model_scale[2];
        }
        // The last 4 are parallel to the Z axis.
        else {
            edge_scale[2] = 2 * model_scale[2] / edge_size_;
            edge_trans[0] = (e & 2) ? model_scale[0] : -model_scale[0];
            edge_trans[1] = (e & 1) ? model_scale[1] : -model_scale[1];
        }

        edge->SetScale(edge_scale);
        edge->SetTranslation(edge_trans);
    }
}
