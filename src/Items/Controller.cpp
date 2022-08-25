#include "Items/Controller.h"

#include "App/CoordConv.h"
#include "Base/Tuning.h"
#include "Math/Linear.h"
#include "SG/ColorMap.h"
#include "SG/Line.h"
#include "SG/NodePath.h"
#include "SG/ProceduralImage.h"
#include "SG/Search.h"
#include "Util/Assert.h"
#include "Util/Enum.h"
#include "Util/General.h"

void Controller::SetHand(Hand hand) {
    hand_ = hand;
    if (hand == Hand::kLeft && guide_parent_)
        RotateGuides_();
}

void Controller::UseCustomModel(const CustomModel &custom_model) {
    ASSERT(custom_model.shape);
    ASSERT(custom_model.texture_image);

    // Get the size of the default model and disable it.
    auto def = SG::FindNodeUnderNode(*this, "DefaultModel");
    const Bounds def_bounds = def->GetBounds();
    def->SetEnabled(false);

    // Add the shape to the custom model and enable it.
    auto cust = SG::FindNodeUnderNode(*this, "CustomModel");
    cust->AddShape(custom_model.shape);
    cust->SetEnabled(true);

    // Scale and translate the custom model to match the default model.
    const Bounds cust_bounds = cust->GetBounds();
    cust->SetUniformScale(def_bounds.GetSize()[2] / cust_bounds.GetSize()[2]);
    cust->SetTranslation(def_bounds.GetCenter() - cust_bounds.GetCenter());

    // Access the ProceduralImage from the Texture from the UniformBlock and
    // set its function to install the given image. This is the easiest way to
    // install a custom Image.
    ASSERT(! cust->GetUniformBlocks().empty());
    const auto &block = cust->GetUniformBlocks()[0];
    ASSERT(block);
    ASSERT(! block->GetTextures().empty());
    const auto &tex = block->GetTextures()[0];
    ASSERT(tex);
    auto proc_image = Util::CastToDerived<SG::ProceduralImage>(tex->GetImage());
    ASSERT(proc_image);
    proc_image->SetFunction([&](){ return custom_model.texture_image; });
    proc_image->RegenerateImage();
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

Vector3f Controller::GetGuideDirection() const {
    // The Rotation guide points forward when not grip dragging.
    if (! is_grip_dragging_ &&
        cur_guide_->GetGripGuideType() == GripGuideType::kRotation)
        return -Vector3f::AxisZ();

    // All other cases: point away from the palm.
    else
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
        if (show) {
            // Scale based on distance from controller to maintain a reasonable
            // size. The scale is 1 at a distance of 1 unit.
            const float distance = ion::math::Distance(Point3f::Zero(), pt);
            const float scale = 1 + (distance - 1) * TK::kPinchHoverScale;
            pointer_hover_node_->SetUniformScale(scale);
            pointer_hover_node_->SetTranslation(pt);

            // Make the pointer end at the sphere.
            pointer_hover_line_->SetEndpoints(Point3f::Zero(), pt);
        }
        else {
            // Use a long laser pointer line.
            pointer_hover_line_->SetEndpoints(Point3f::Zero(),
                                              Point3f(0, 0, -10000));
        }
    }
}

void Controller::ShowGripHover(bool show, const Point3f &pt,
                               const Color &color) {
    if (grip_hover_node_) {
        grip_hover_node_->SetEnabled(show);
        if (show) {
            // Flip X for left hand.
            Point3f guide_pt = cur_guide_->GetHoverPoint();
            if (hand_ == Hand::kLeft)
                guide_pt[0] = -guide_pt[0];
            grip_hover_node_->SetBaseColor(color);
            grip_hover_line_->SetEndpoints(guide_pt, pt);
        }
    }
}

void Controller::ShowTouch(bool is_start) {
    const std::string color_name =
        is_start ? "TouchActiveColor" : "TouchInactiveColor";
    touch_tip_node_->SetBaseColor(SG::ColorMap::SGetColor(color_name));
    Vibrate(.1f);
}

void Controller::Vibrate(float seconds) {
    if (vibrate_func_)
        vibrate_func_(seconds);
}

void Controller::AttachObject(const SG::NodePtr &object, float size_fraction,
                              const Vector3f &offset) {
    Vector3f trans = offset;
    if (hand_ == Hand::kRight)
        trans[0] = -trans[0];
    object->SetTranslation(trans);

    const auto &model       = *SG::FindNodeUnderNode(*this, "Model");
    const auto object_scale = object->GetScale();
    const auto object_size  = object->GetScaledBounds().GetSize();
    const auto model_size   = model.GetScaledBounds().GetSize();
    const float obj_max = object_size[GetMaxElementIndex(object_size)];
    object->SetScale(object_scale * (size_fraction * model_size[2] / obj_max));

    AddChild(object);
}

void Controller::DetachObject(const SG::NodePtr &object) {
    RemoveChild(object);
}

void Controller::PostSetUpIon() {
    // Access the important nodes.
    touch_node_         = SG::FindNodeUnderNode(*this, "Touch");
    pointer_node_       = SG::FindNodeUnderNode(*this, "LaserPointer");
    grip_node_          = SG::FindNodeUnderNode(*this, "Grip");
    pointer_hover_node_ = SG::FindNodeUnderNode(*this, "PointerHoverHighlight");
    grip_hover_node_    = SG::FindNodeUnderNode(*this, "GripHoverHighlight");

    // Set up the touch math. This has to be done while the touch geometry is
    // enabled.
    const auto touch_path = SG::FindNodePathUnderNode(touch_node_, "TouchTip");
    touch_tip_node_       = touch_path.back();
    const CoordConv touch_cc(touch_path);
    touch_offset_ = Vector3f(touch_cc.ObjectToRoot(Point3f::Zero()));
    touch_tip_node_->SetBaseColor(
        SG::ColorMap::SGetColor("TouchInactiveColor"));

    // Also compute the touch radius.
    const Bounds touch_bounds = TransformBounds(
        touch_tip_node_->GetBounds(), touch_cc.GetObjectToRootMatrix());
    touch_radius_ = .5f * touch_bounds.GetSize()[0];

    // Access the laser pointer Line shape.
    pointer_hover_line_ =
        SG::FindTypedShapeInNode<SG::Line>(*pointer_node_, "Line");

    // Access the Line shape for the grip hover so it can have its endpoints
    // adjusted for feedback.
    grip_hover_line_ =
        SG::FindTypedShapeInNode<SG::Line>(*grip_hover_node_, "Line");

    // Access the GripGuides parent node and rotate for the left controller.
    guide_parent_ = SG::FindNodeUnderNode(*grip_node_, "GripGuides");
    if (GetHand() == Hand::kLeft)
        RotateGuides_();

    // Add each of its children as a guide.
    for (auto &child: guide_parent_->GetChildren()) {
        GripGuidePtr guide = Util::CastToDerived<GripGuide>(child);
        ASSERT(guide);
        guides_.push_back(guide);
    }
    ASSERT(guides_.size() == Util::EnumCount<GripGuideType>());

    // Start with no guide (the kNone version).
    cur_guide_ = guides_[0];

    // Set to the inactive state.
    SetTriggerMode(Trigger::kPointer, false);
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

void Controller::RotateGuides_() {
    ASSERT(guide_parent_);
    guide_parent_->SetRotation(
        Rotationf::FromAxisAndAngle(Vector3f::AxisZ(),
                                    Anglef::FromDegrees(180)));
}
