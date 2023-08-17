#include "Items/Controller.h"

#include <ion/math/transformutils.h>

#include "Items/RadialMenu.h"
#include "Math/Linear.h"
#include "Parser/Registry.h"
#include "SG/ColorMap.h"
#include "SG/CoordConv.h"
#include "SG/MutableTriMeshShape.h"
#include "SG/NodePath.h"
#include "SG/ProceduralImage.h"
#include "SG/Search.h"
#include "SG/Texture.h"
#include "SG/Tube.h"
#include "Util/Assert.h"
#include "Util/Enum.h"
#include "Util/Tuning.h"

void Controller::SetHand(Hand hand) {
    hand_ = hand;
    if (hand_ == Hand::kLeft)
        RotateLeftGuides_();
}

void Controller::UseCustomModel(const CustomModel &custom_model) {
    ASSERT(custom_model.texture_image);
    ASSERT(! custom_model.mesh.points.empty());
    ASSERT(custom_model.mesh.points.size() ==
           custom_model.mesh.normals.size());
    ASSERT(custom_model.mesh.points.size() ==
           custom_model.mesh.tex_coords.size());

    // Save the CustomModel data.
    custom_model_ = custom_model;

    // Disable the default model.
    auto def = SG::FindNodeUnderNode(*this, "DefaultModel");
    def->SetEnabled(false);

    // Build a MutableTriMeshShape from the ModelMesh.
    auto shape = Parser::Registry::CreateObject<SG::MutableTriMeshShape>();
    shape->ChangeModelMesh(custom_model.mesh);

    // Add the shape to the custom model and enable it.
    auto cust = SG::FindNodeUnderNode(*this, "CustomModel");
    cust->AddShape(shape);
    cust->SetEnabled(true);

    // Access the ProceduralImage from the Texture from the UniformBlock and
    // set its function to install the given image. This is the easiest way to
    // install a custom Image.
    ASSERT(! cust->GetUniformBlocks().empty());
    const auto &block = cust->GetUniformBlocks()[0];
    ASSERT(block);
    ASSERT(! block->GetTextures().empty());
    const auto &tex = block->GetTextures()[0];
    ASSERT(tex);
    auto proc_image =
        std::dynamic_pointer_cast<SG::ProceduralImage>(tex->GetImage());
    ASSERT(proc_image);
    proc_image->SetFunction([&](){ return custom_model.texture_image; });
    proc_image->RegenerateImage();

    // Adjust positioning of feedback based on the model.
    ASSERT(guide_parent_);
    ASSERT(touch_node_);
    pointer_start_point_ = custom_model.pointer_pos;
    guide_parent_->SetTranslation(custom_model.grip_pos);
    touch_node_->SetTranslation(custom_model.touch_pos);

    // Recompute values that depend on the geometry.
    UpdateForGeometry_();
}

void Controller::SetGripGuideType(GripGuideType type) {
    if (cur_guide_->GetGripGuideType() != type) {
        // Set and show the current guide, hiding the rest.
        for (auto &guide: guides_) {
            if (guide->GetGripGuideType() == type) {
                cur_guide_ = guide;
                guide->SetEnabled(true);
            }
            else {
                guide->SetEnabled(false);
            }
        }
    }
}

GripGuideType Controller::GetGripGuideType() const {
    return cur_guide_ ? cur_guide_->GetGripGuideType() : GripGuideType::kNone;
}

Vector3f Controller::GetGuideDirection() const {
    // Point away from the palm.
    return hand_ == Hand::kLeft ? Vector3f::AxisX() : -Vector3f::AxisX();
}

void Controller::SetTouchMode(bool in_touch_mode) {
    if (is_in_touch_mode_ != in_touch_mode) {
        is_in_touch_mode_ = in_touch_mode;
        ShowAffordance_(Trigger::kTouch, in_touch_mode);
    }
}

void Controller::ShowAll(bool show) {
    ShowAffordance_(Trigger::kPointer, show);
    ShowAffordance_(Trigger::kGrip,    show);
    ShowAffordance_(Trigger::kTouch,   show && is_in_touch_mode_);
}

void Controller::SetTriggerMode(Trigger trigger, bool is_triggered) {
    if (is_triggered) {
        switch (trigger) {
          case Trigger::kPointer:
            pointer_node_->SetBaseColor(
                SG::ColorMap::SGetColor("LaserActiveColor"));
            ShowAffordance_(Trigger::kGrip,    false);
            ShowAffordance_(Trigger::kTouch,   false);
            break;
          case Trigger::kGrip:
            ShowAffordance_(Trigger::kPointer, false);
            ShowAffordance_(Trigger::kTouch,   false);
            is_grip_dragging_ = true;
            break;
          case Trigger::kTouch:
            ASSERT(is_in_touch_mode_);
            ShowAffordance_(Trigger::kPointer, false);
            ShowAffordance_(Trigger::kGrip,    false);
            break;
        }
    }
    else {
        pointer_node_->SetBaseColor(
            SG::ColorMap::SGetColor("LaserInactiveColor"));
        ShowAll(true);
        is_grip_dragging_ = false;
    }
}

void Controller::ShowPointerHover(bool show, const Point3f &pt) {
    if (pointer_hover_node_) {
        pointer_hover_node_->SetEnabled(show);
        Point3f end_pt;
        if (show) {
            // Scale based on distance from controller to maintain a reasonable
            // size. The scale is 1 at a distance of 1 unit.
            const float distance = ion::math::Distance(pointer_start_point_, pt);
            const float scale = 1 + (distance - 1) * TK::kPinchHoverScale;
            pointer_hover_node_->SetUniformScale(scale);
            pointer_hover_node_->SetTranslation(pt);

            // Make the pointer end at the sphere.
            end_pt = pt;
        }
        else {
            // Use a long laser pointer.
            end_pt.Set(0, 0, -10000);
        }
        // Change the points of the tube and adjust the taper based on the
        // distance.
        pointer_hover_tube_->SetEndpoints(pointer_start_point_, end_pt);
        pointer_hover_tube_->SetTaper(
            ComputeTaper_(pointer_start_point_, end_pt));
    }
}

void Controller::ShowGripHover(bool show, const Point3f &pt,
                               const Color &color) {
    if (grip_hover_node_) {
        grip_hover_node_->SetEnabled(show);
        if (show) {
            ASSERT(cur_guide_->GetGripGuideType() != GripGuideType::kNone);
            Point3f guide_pt = cur_guide_->GetHoverPoint();
            // Flip X for left hand.
            if (hand_ == Hand::kLeft)
                guide_pt[0] = -guide_pt[0];
            guide_pt += guide_parent_->GetTranslation();
            grip_hover_node_->SetBaseColor(color);
            grip_hover_tube_->SetEndpoints(guide_pt, pt);
            grip_hover_tube_->SetTaper(ComputeTaper_(guide_pt, pt));
        }
    }
}

void Controller::ShowTouch(bool is_start) {
    const Str color_name = is_start ? "TouchActiveColor" : "TouchInactiveColor";
    touch_tip_node_->SetBaseColor(SG::ColorMap::SGetColor(color_name));
    Vibrate(.1f);
}

void Controller::Vibrate(float seconds) {
    if (vibrate_func_)
        vibrate_func_(seconds);
}

void Controller::AttachObject(const SG::NodePtr &object,
                              const Vector3f &offset) {
    Vector3f trans = offset;
    if (hand_ == Hand::kRight)
        trans[0] = -trans[0];

    // Rotate the object so that it aligns with the controller.
    object->SetRotation(Rotationf::FromAxisAndAngle(Vector3f::AxisX(),
                                                    Anglef::FromDegrees(-90)));

    // Offset from the center of the controller.
    object->SetTranslation(model_node_->GetScaledBounds().GetCenter() + trans);

    AddChild(object);
}

void Controller::DetachObject(const SG::NodePtr &object) {
    RemoveChild(object);
}

void Controller::AttachRadialMenu(const RadialMenuPtr &menu) {
    auto &parent = *SG::FindNodeUnderNode(*this, "RadialMenuParent");
    // Flip the translation in X for the left hand.
    if (hand_ == Hand::kLeft) {
        Vector3f trans = parent.GetTranslation();
        trans[0] = -trans[0];
        parent.SetTranslation(trans);
    }
    parent.AddChild(menu);
}

void Controller::PostSetUpIon() {
    // Access the important nodes.
    model_node_         = SG::FindNodeUnderNode(*this, "Model");
    touch_node_         = SG::FindNodeUnderNode(*this, "Touch");
    pointer_node_       = SG::FindNodeUnderNode(*this, "LaserPointer");
    grip_node_          = SG::FindNodeUnderNode(*this, "Grip");
    pointer_hover_node_ = SG::FindNodeUnderNode(*this, "PointerHoverHighlight");
    grip_hover_node_    = SG::FindNodeUnderNode(*this, "GripHoverHighlight");
    touch_tip_node_     = SG::FindNodeUnderNode(*this, "TouchTip");

    touch_tip_node_->SetBaseColor(
        SG::ColorMap::SGetColor("TouchInactiveColor"));

    // Access the laser pointer Tube shape.
    pointer_hover_tube_ =
        SG::FindTypedShapeInNode<SG::Tube>(*pointer_node_, "Tube");

    // Access the Tube shape for the grip hover so it can have its endpoints
    // adjusted for feedback.
    grip_hover_tube_ =
        SG::FindTypedShapeInNode<SG::Tube>(*grip_hover_node_, "Tube");

    // Access the GripGuides parent node and rotate for the left controller.
    guide_parent_ = SG::FindNodeUnderNode(*grip_node_, "GripGuides");

    // Add each of its children as a guide.
    for (auto &child: guide_parent_->GetChildren()) {
        GripGuidePtr guide = std::dynamic_pointer_cast<GripGuide>(child);
        ASSERT(guide);
        guides_.push_back(guide);
    }
    ASSERT(guides_.size() == Util::EnumCount<GripGuideType>());

    // Start with no guide (the kNone version).
    cur_guide_ = guides_[0];

    UpdateForGeometry_();

    // Set to the inactive state.
    SetTriggerMode(Trigger::kPointer, false);
}

bool Controller::GetCustomModelData(ModelMesh &mesh,
                                    ion::gfx::ImagePtr &image) const {
    if (! custom_model_.mesh.points.empty()) {
        mesh  = custom_model_.mesh;
        image = custom_model_.texture_image;
        return true;
    }
    return false;
}

void Controller::ShowAffordance_(Trigger trigger, bool is_shown) {
    SG::NodePtr node;
    switch (trigger) {
      case Trigger::kPointer: node = pointer_node_; break;
      case Trigger::kGrip:    node = grip_node_;    break;
      case Trigger::kTouch:   node = touch_node_;   break;
    }
    ASSERT(node);
    node->SetEnabled(is_shown);
}

void Controller::RotateLeftGuides_() {
    ASSERT(guide_parent_);
    const Rotationf flip =
        Rotationf::FromAxisAndAngle(Vector3f::AxisZ(),
                                    Anglef::FromDegrees(180));
    guide_parent_->SetRotation(flip * guide_parent_->GetRotation());
}

void Controller::UpdateForGeometry_() {
    // Translate the guides to touch a good point on the controller model.
    PositionGuides_();

    // Set up the touch geometry and math.
    SetUpForTouch_();
}

void Controller::PositionGuides_() {
    const bool is_left = hand_ == Hand::kLeft;

    // Find the mesh point that is the maximum to the left (for the right hand)
    // or to the right (for the left hand).
    Point3f target_point;
    const auto cust = SG::FindNodeUnderNode(*this, "CustomModel");
    if (cust->IsEnabled()) {
        ASSERT(! cust->GetShapes().empty());
        const auto shape = std::dynamic_pointer_cast<SG::MutableTriMeshShape>(
            cust->GetShapes()[0]);
        ASSERT(shape);
        const auto &mesh = shape->GetMesh();
        target_point = mesh.points[0];
        for (const auto &pt: mesh.points) {
            const bool is_on_correct_side = is_left ?
                (pt[0] > target_point[0]) : (pt[0] < target_point[0]);
            if (is_on_correct_side)
                target_point = pt;
        }
    }
    else {
        // No custom model. Use a fixed point on the Ellipsoid.
        target_point = Point3f(is_left ? .025 : -.025, 0, 0);
    }

    // Position the guide parent. Always use the right side of the guide since
    // it is rotated for the left controller.
    const auto guide_bounds = guide_parent_->GetScaledBounds();
    Point3f guide_pt = guide_bounds.GetFaceCenter(Bounds::Face::kRight);
    if (is_left)
        guide_pt[0] = -guide_pt[0];
    const Vector3f trans = target_point - guide_pt;
    guide_parent_->SetTranslation(trans);
}

void Controller::SetUpForTouch_() {
    // The touch geometry must be enabled for coordinate conversion to work.
    const bool was_enabled = touch_node_->IsEnabled();
    touch_node_->SetEnabled(true);

    const auto touch_path =
        SG::FindNodePathUnderNode(touch_node_, *touch_tip_node_);
    const SG::CoordConv touch_cc(touch_path);
    const Matrix4f      touch_m = touch_cc.GetObjectToRootMatrix();

    touch_offset_ = Vector3f(touch_m * Point3f::Zero());

    // Also compute the touch radius.
    const Bounds touch_bounds =
        TransformBounds(touch_tip_node_->GetBounds(), touch_m);
    touch_radius_ = .5f * touch_bounds.GetSize()[0];

    touch_node_->SetEnabled(was_enabled);
}

float Controller::ComputeTaper_(const Point3f &p0, const Point3f &p1) {
    // Distance is about .2 for touch on panels and about 60 to something near
    // the center of the stage and about 120 near the back wall. Want taper to
    // be about 100 for the back wall and about 1 for panels.
    const float d = ion::math::Distance(p0, p1);
    return Lerp(d / 120, 1, 100);
}
