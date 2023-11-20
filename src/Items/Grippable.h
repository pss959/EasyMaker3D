#pragma once

#include <vector>

#include <ion/base/invalid.h>
#include <ion/math/vectorutils.h>

#include "Base/Event.h"
#include "Enums/GripGuideType.h"
#include "Enums/Hand.h"
#include "Math/Types.h"
#include "SG/ColorMap.h"
#include "SG/Node.h"
#include "SG/NodePath.h"
#include "Util/Memory.h"

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

        /// Type of grip guide to display on the controller.
        GripGuideType      guide_type = GripGuideType::kBasic;

        /// ClickableWidget that is the hover target, or null if there is none.
        ClickableWidgetPtr widget;

        /// Target point of the grip hover feedback in world coordinates. This
        /// is ignored if widget is null.
        Point3f            target_point{0, 0, 0};

        /// Color to use for the grip hover feedback. This is ignored if widget
        /// is null.
        Color              color = SG::ColorMap::SGetColor("GripNeutralColor");

        ///@}
    };

    /// Sets a path from the scene root to the Grippable that can be used for
    /// converting to and from world coordinates. Derived classes should call
    /// this version if they override this.
    virtual void SetPath(const SG::NodePath &path) { path_ = path; }

    /// Returns a raw pointer to a Node to use to represent the Grippable in
    /// the scene. This Node is used to determine the coordinate conversion for
    /// grip hovers. If the Grippable is not active, this should return a null
    /// pointer.
    virtual const SG::Node * GetGrippableNode() const = 0;

    /// This is given a GripInfo instance with just the Event filled in. The
    /// Grippable should set the other fields in the instance with the correct
    /// grip hover target.
    virtual void UpdateGripInfo(GripInfo &info) = 0;

    /// This is called to tell the Grippable that a grip has been activated or
    /// deactivated. The Hand for the relevant controller is passed in. The
    /// base class defines this to do nothing.
    virtual void ActivateGrip(Hand hand, bool is_active) {}

  protected:
    Grippable() {}

    /// Returns the path from the root of the scene to the Grippable that can
    /// be used to convert to and from world coordinates.
    const SG::NodePath & GetPath() const;

    /// Derived classes can call this to compare a direction vector \p dir to a
    /// set of candidate vectors. The index of the closest candidate is
    /// returned; it will be the one with the smallest angle between it (or its
    /// opposite) and \p dir, setting \p is_opposite to true if the opposite
    /// was used. If \p max_angle is positive, this will return a choice only
    /// if the computed angle is not larger. If there are no good choices, -1
    /// is returned. This assumes all directions are unit vectors.
    static int GetBestDirIndex(const std::vector<Vector3f> &candidates,
                               const Vector3f &dir, const Anglef &max_angle,
                               bool &is_opposite);

    // Special case of GetBestDirIndex() that uses the 3 principal axes.
    static int GetBestAxis(const Vector3f &dir, const Anglef &max_angle,
                           bool &is_opposite);

    /// Returns the color to use for a neutral grip hover connection.
    static Color GetNeutralGripColor() {
        return SG::ColorMap::SGetColor("GripNeutralColor");
    }

    /// Returns the color to use for an active grip hover connection.
    static Color GetActiveGripColor() {
        return SG::ColorMap::SGetColor("GripActiveColor");
    }

  private:
    /// Path from the root of the scene to the Grippable.
    SG::NodePath path_;

    friend class Parser::Registry;
};
