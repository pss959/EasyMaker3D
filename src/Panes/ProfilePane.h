#pragma once

#include <memory>

#include "Math/Profile.h"
#include "Panes/Pane.h"
#include "Util/Notifier.h"

namespace Parser { class Registry; }

/// ProfilePane is a derived Pane that supports editing of 2D Profiles. It is
/// not marked as being interactive because it cannot do anything with keyboard
/// focus and all interaction is through Widgets that already receive events.
class ProfilePane : public Pane {
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
    Parser::TField<int> min_point_count_{"min_point_count", 0};
    ///@}

    friend class Parser::Registry;
};

typedef std::shared_ptr<ProfilePane> ProfilePanePtr;
