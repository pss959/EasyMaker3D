#pragma once

#include "Base/Memory.h"
#include "Math/Profile.h"
#include "Math/Types.h"
#include "Panes/IPaneInteractor.h"
#include "Panes/LeafPane.h"
#include "Util/Notifier.h"

namespace Parser { class Registry; }

class Slider2DWidget;
DECL_SHARED_PTR(ClickableWidget);
DECL_SHARED_PTR(ProfilePane);

/// ProfilePane is a derived LeafPane that supports editing of 2D Profiles. All
/// interaction is through Widgets that already receive events.
///
/// \ingroup Panes
class ProfilePane : public LeafPane, public IPaneInteractor {
  public:
    /// Returns a Notifier that is invoked when an interactive Widget is
    /// activated or deactivated. It is passed and a flag indicating activation
    /// or deactivation.
    Util::Notifier<bool> & GetActivation();

    /// Returns a Notifier that is invoked when the Profile changes.
    Util::Notifier<> & GetProfileChanged();

    /// Sets the Profile to edit. This initializes the ProfilePane for editing.
    void SetProfile(const Profile &profile);

    /// Returns the Profile, which may have been edited. The default Profile is
    /// returned by CreateDefaultProfile().
    const Profile & GetProfile() const;

    /// Creates and returns the default Profile used for the ProfilePane: fixed
    /// from (0, 1) to (1, 0).
    static Profile CreateDefaultProfile();

    /// Sets the precision in X and Y to round points to. Passing zero for
    /// either turns off rounding in that dimension. The default is 0 for both.
    /// Note that points are in the range from (0,0) to (1,1), so the precision
    /// should be relative to that size.
    void SetPointPrecision(const Vector2f &xy_precision);

    /// Redefines this to maintain the aspect ratio of items in the Pane.
    virtual void UpdateForLayoutSize(const Vector2f &size) override;

    /// Returns the ClickableWidget to grip hover for the given point,
    /// which is normalized to (0,1) in both dimensions.
    ClickableWidgetPtr GetGripWidget(const Point2f &p);

    /// Redefines this to test the movable point sliders and also midpoints for
    /// touch interaction.
    virtual WidgetPtr GetTouchedWidget(const TouchInfo &info,
                                       float &closest_distance) override;

    // IPaneInteractor interface.
    virtual IPaneInteractor * GetInteractor() override;
    virtual BorderPtr GetFocusBorder() const override;

    virtual void PostSetUpIon() override;

  protected:
    ProfilePane();
    virtual ~ProfilePane() override;

    virtual void CreationDone() override;

    /// This is called when setting the range (in 0-1 profile coordinates) of
    /// the Slider2DWidget used to drag a movable point. The current Profile
    /// and the index of the point within the profile are supplied. The base
    /// class defines this to always return (0,1).
    virtual Range2f GetMovablePointRange(Slider2DWidget &slider,
                                         const Profile &profile,
                                         size_t index) const;

    /// This is called to determine if a new point can be inserted into the
    /// given Profile between the given index and the next point in the
    /// Profile. The base class defines this to always return true).
    virtual bool CanInsertPoint(const Profile &profile, size_t index) const;

  private:
    class Impl_;
    std::unique_ptr<Impl_> impl_;

    friend class Parser::Registry;
};
