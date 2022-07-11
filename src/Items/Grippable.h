#pragma once

#include <string>
#include <vector>

#include <ion/base/invalid.h>
#include <ion/math/vectorutils.h>

#include "Base/Event.h"
#include "Base/Memory.h"
#include "Enums/GripGuideType.h"
#include "Enums/Hand.h"
#include "Math/Types.h"
#include "SG/ColorMap.h"
#include "SG/Node.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(ClickableWidget);
DECL_SHARED_PTR(Controller);
DECL_SHARED_PTR(Grippable);

/// Grippable is an abstract base class for nodes that implement VR controller
/// grip interaction in some way. There are several pure virtual functions that
/// derived classes must implement.
///
/// \ingroup Items
class Grippable : public SG::Node {
  public:
    /// The GripInfo struct packages up information to help implement
    /// grip-hovering and grip-dragging in VR. All but the Event are set by a
    /// Grippable object.
    struct GripInfo {
        /// \name Values set by the MainHandler.
        /// A Grippable object can use these to implement its UpdateGripInfo()
        /// function.
        ///@{

        /// Event containing controller position, direction, and motion values.
        Event              event;

        /// Controller involved in the grip operation. This will not be null.
        ControllerPtr      controller;

        /// Direction of the guide attached to the controller involved in the
        /// grip operation.
        Vector3f           guide_direction;

        ///@}

        /// \name Values set by the Grippable.
        /// A Grippable object can modify these in its UpdateGripInfo()
        /// function.
        ///@{

        /// ClickableWidget that is the hover target, or null if there is none.
        ClickableWidgetPtr widget;

        /// Target point of the grip hover feedback in world coordinates. This
        /// is ignored if widget is null.
        Point3f            target_point{0, 0, 0};

        /// Color to use for the grip hover feedback. This is ignored if widget
        /// is null.
        Color              color = SG::ColorMap::SGetColor("GripDefaultColor");

        ///@}
    };

    /// Returns a raw pointer to a Node to use to represent the Grippable in
    /// the scene. This Node is used to determine the coordinate conversion for
    /// grip hovers. If the Grippable is not active, this should return a null
    /// pointer.
    virtual const SG::Node * GetGrippableNode() const = 0;

    /// Returns the GripGuideType that should be used to hover-highlight the
    /// Grippable's interaction. The base class defines this to return the
    /// basic guide.
    virtual GripGuideType GetGripGuideType() const {
        return GripGuideType::kBasic;
    }

    /// This is given a GripInfo instance with just the Event filled in. The
    /// Grippable should set the other fields in the instance with the correct
    /// grip hover target.
    virtual void UpdateGripInfo(GripInfo &info) = 0;

    /// This is called to tell the Grippable that a grip has been activated or
    /// deactivated. The Hand for the relevant controller is passed in. The
    /// base class defines this to do nothing.
    virtual void ActivateGrip(Hand hand, bool is_active) {}

  protected:
    /// Struct used by GetClosestDir().
    struct DirChoice {
        std::string name;
        Vector3f    direction;
        DirChoice() {}
        DirChoice(const std::string &name_in, const Vector3f &dir_in) :
            name(name_in), direction(ion::math::Normalized(dir_in)) {}
    };

    Grippable() {}

    /// Derived classes can call this to get the best choice from a collection
    /// of DirChoice instances. The index of the best choice in the vector is
    /// returned; it will be the one with the smallest angle between its
    /// direction and the given direction. If max_angle is positive, this will
    /// return a choice only if the angle is not larger. If there are no good
    /// choices, ion::base::kInvalidIndex is returned. This assumes all
    /// directions are unit vectors.
    static size_t GetBestDirChoice(const std::vector<DirChoice> &choices,
                                   const Vector3f &direction,
                                   const Anglef &max_angle);

    /// This is similar to GetBestDirChoice() except that it also allows the
    /// vectors to point in opposite directions. This sets to is_opposite to
    /// true in that case.
    static size_t GetBestDirChoiceSymmetric(
        const std::vector<DirChoice> &choices,
        const Vector3f &direction, const Anglef &max_angle, bool &is_opposite);

  private:
    friend class Parser::Registry;
};
