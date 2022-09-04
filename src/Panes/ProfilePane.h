#pragma once

#include "Base/Memory.h"
#include "Math/Profile.h"
#include "Panes/IPaneInteractor.h"
#include "Panes/LeafPane.h"
#include "Util/Notifier.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(ClickableWidget);
DECL_SHARED_PTR(ProfilePane);

/// ProfilePane is a derived LeafPane that supports editing of 2D Profiles. It
/// is not marked as being interactive because it cannot do anything with
/// keyboard focus and all interaction is through Widgets that already receive
/// events.
///
/// \ingroup Panes
class ProfilePane : public LeafPane, public IPaneInteractor {
  public:
    /// Returns a Notifier that is invoked when an interactive Widget is
    /// activated or deactivated. It is passed and a flag indicating activation
    /// or deactivation.
    Util::Notifier<bool> & GetActivation();

    /// Returns a Notifier that is invoked when the Profile changes. It is
    /// passed the new Profile.
    Util::Notifier<const Profile &> & GetProfileChanged();

    /// Returns the minimum number of additional points the edited Profile is
    /// allowed to have. The default is 0.
    size_t GetMinPointCount() const { return min_point_count_; }

    /// Sets the Profile to edit. This initializes the ProfilePane for editing.
    void SetProfile(const Profile &profile);

    /// Returns the Profile, which may have been edited.
    const Profile & GetProfile() const;

    /// Redefines this to maintain the aspect ratio of items in the Pane.
    virtual void SetLayoutSize(const Vector2f &size) override;

    /// Returns the ClickableWidget to grip hover for the given point,
    /// which is normalized to (0,1) in both dimensions.
    ClickableWidgetPtr GetGripWidget(const Point2f &p);

    virtual IPaneInteractor * GetInteractor() override { return this; }
    virtual bool CanFocus() const override { return true; }

  protected:
    ProfilePane();

    virtual void AddFields() override;
    virtual bool IsValid(std::string &details) override;
    virtual void CreationDone() override;

  private:
    class Impl_;
    std::unique_ptr<Impl_> impl_;

    /// \name Parsed Fields
    ///@{
    Parser::TField<int> min_point_count_;
    ///@}

    friend class Parser::Registry;
};
