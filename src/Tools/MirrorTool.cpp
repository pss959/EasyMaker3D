#include "Tools/MirrorTool.h"

#include <ion/math/transformutils.h>

#include "Commands/ChangeMirrorCommand.h"
#include "Items/SessionState.h"
#include "Managers/CommandManager.h"
#include "Math/Linear.h"
#include "Models/MirroredModel.h"
#include "SG/ColorMap.h"
#include "SG/Node.h"
#include "SG/Search.h"
#include "Util/Assert.h"

bool MirrorTool::CanAttach(const Selection &sel) const {
    return AreSelectedModelsOfType<MirroredModel>(sel);
}

void MirrorTool::Attach() {
    ASSERT(Util::IsA<MirroredModel>(GetModelAttachedTo()));

    // Set up planes if not already done.
    if (! planes_[0]) {
        for (int dim = 0; dim < 3; ++dim) {
            std::string name = "XPlane";
            name[0] += dim;
            planes_[dim] =
                SG::FindTypedNodeUnderNode<PushButtonWidget>(*this, name);
            // Use translucent color.
            const float kPlaneOpacity = .2f;
            Color color = SG::ColorMap::SGetColorForDimension(dim);
            color[3] = kPlaneOpacity;
            planes_[dim]->SetInactiveColor(color);

            planes_[dim]->GetClicked().AddObserver(
                this, [&, dim](const ClickInfo &){ PlaneClicked_(dim); });
        }
    }

    // Match the MirroredModel's transform and pass the size of the Model for
    // scaling. Allow axis alignment.
    const Vector3f model_size = MatchModelAndGetSize(true);

    // Make the plane rectangles a little larger than the model size.
    const float kPlaneScale = 1.8f;
    SetScale(kPlaneScale * model_size);
}

void MirrorTool::Detach() {
    // Nothing to do here.
}

void MirrorTool::PlaneClicked_(int dim) {
    const auto &context = GetContext();

    // Get the mirror plane in stage coordinates based on axis-alignment.
    const Matrix4f osm = GetStageCoordConv().GetObjectToRootMatrix();
    const Vector3f axis = GetAxis(dim);
    const Plane stage_plane = IsAxisAligned() ?
        Plane(osm * Point3f::Zero(), axis) :
        Plane(Point3f(GetTranslation()), osm * axis);

    // Add and execute a ChangeMirrorCommand.
    auto command = CreateCommand<ChangeMirrorCommand>();
    command->SetFromSelection(GetSelection());
    command->SetPlane(stage_plane);
    command->SetInPlace(context.is_modified_mode);
    context.command_manager->AddAndDo(command);
}
