#include "Controller.h"

#include <ion/gfx/shaderinputregistry.h>
#include <ion/gfx/shape.h>
#include <ion/gfx/uniform.h>
#include <ion/gfxutils/shapeutils.h>
#include <ion/math/matrix.h>
#include <ion/math/vector.h>
#include <ion/math/transformutils.h>

#include "Event.h"
#include "Interfaces/IScene.h"

using ion::gfx::NodePtr;
using ion::math::Matrix4f;
using ion::math::Vector4f;

// ----------------------------------------------------------------------------
// Controller implementation.
// ----------------------------------------------------------------------------

Controller::Controller(Hand hand) : hand_(hand) {
}

Controller::~Controller() {
}

bool Controller::HandleEvent(const Event &event) {
    // Track the corresponding controller.
    Event::Device this_device = hand_ == Hand::kLeft ?
        Event::Device::kLeftController : Event::Device::kRightController;
    if (event.device == this_device &&
        event.flags.Has(Event::Flag::kPosition3D) &&
        event.flags.Has(Event::Flag::kOrientation)) {
        if (event.orientation.IsIdentity()) {
            //  If the orientation is identity, the controller is not active,
            // so hide the model.
            node_->Enable(false);
        }
        else {
            node_->Enable(true);
            const Matrix4f mat =
                ion::math::TranslationMatrix(event.position3D) *
                ion::math::RotationMatrixH(event.orientation);
            node_->SetUniformValue(matrix_index_, mat);
        }
    }
    // No need to trap these events - others may be interested.
    return false;
}

void Controller::AddModelToScene(IScene &scene) {
    BuildShape();
    scene.GetRoot()->AddChild(node_);
}

void Controller::BuildShape() {
    ion::gfxutils::EllipsoidSpec spec;
    spec.vertex_type = ion::gfxutils::ShapeSpec::kPosition;
    spec.band_count   = 20;
    spec.sector_count = 22;
    spec.size.Set(.1f, .05f, .2f);

    node_.Reset(new ion::gfx::Node);
    const ion::gfx::ShaderInputRegistryPtr& global_reg =
        ion::gfx::ShaderInputRegistry::GetGlobalRegistry();
    node_->AddShape(ion::gfxutils::BuildEllipsoidShape(spec));
    node_->AddUniform(global_reg->Create<ion::gfx::Uniform>(
                         "uBaseColor", Vector4f(.8f, .8f, .8f, 1.f)));
    matrix_index_ =
        node_->AddUniform(global_reg->Create<ion::gfx::Uniform>(
                             "uModelviewMatrix", Matrix4f::Identity()));
}
