#pragma once

#include <memory>
#include <string>
#include <vector>

#include <ion/base/invalid.h>
#include <ion/math/vectorutils.h>

#include "Enums/GripGuideType.h"
#include "Event.h"
#include "Math/Types.h"
#include "SG/Node.h"
#include "Widgets/ClickableWidget.h"

namespace Parser { class Registry; }

/// Grippable is an abstract base class for nodes that implement VR controller
/// grip interaction in some way. There are several pure virtual functions that
/// derived classes must implement.
class Grippable : public SG::Node {
  public:
    /// The GripInfo struct packages up information to help implement
    /// grip-hovering and grip-dragging in VR. All but the Event are set by a
    /// Grippable object.
    struct GripInfo {
        /// Event containing controller position, direction, and motion values.
        Event              event;

        /// Target point of the grip hover feedback in world coordinates.
        Point3f            target_point{0, 0, 0};

        /// Color to use for the grip hover feedback. Defaults to white if not
        /// set.
        Color              color = Color::White();

        /// ClickableWidget that is the hover target, or null if there is none.
        ClickableWidgetPtr widget;
    };

    /// Returns true if the Grippable is currently enabled.
    virtual bool IsGrippableEnabled() const = 0;

    /// This is given a GripInfo instance with just the Event filled in. The
    /// Grippable should set the other fields in the instance with the correct
    /// grip hover target.
    virtual void UpdateGripInfo(GripInfo &info) = 0;

    /// Returns the GripGuideType that should be used to hover-highlight the
    /// Grippable's interaction. The base class defines this to return the
    /// basic guide.
    virtual GripGuideType GetGripGuideType() const {
        return GripGuideType::kBasic;
    }

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

  private:
    friend class Parser::Registry;
};

typedef std::shared_ptr<Grippable> GrippablePtr;
