#pragma once

#include <memory>

#include "ClickInfo.h"
#include "Event.h"
#include "Handlers/Handler.h"
#include "SceneContext.h"
#include "Util/Notifier.h"
#include "Widgets/ClickableWidget.h"

/// MainHandler is a derived Handler that does most of the interactive event
/// handling for the application.
/// \ingroup Handlers
class MainHandler : public Handler {
  public:
    MainHandler();
    virtual ~MainHandler();

    /// Sets the SceneContext to interact with.
    void SetSceneContext(std::shared_ptr<SceneContext> context);

    /// Returns a Notifier that is invoked when a click is detected. An
    /// Observer is passed a ClickInfo instance containing all relevant data.
    Util::Notifier<const ClickInfo &> & GetClicked();

    /// Returns a Notifier that is invoked when a valuator of some sort (such
    /// as a mouse wheel) changes. It is passed the device and the relative
    /// change in position.
    Util::Notifier<Event::Device, float> & GetValuatorChanged();

    /// Returns true if the MainHandler is waiting for something to happen,
    // meaning that it is not in the middle of handling a button press, drag
    // operation, or waiting for a click timer.
    bool IsWaiting() const;

    /// This should be called explicitly each frame to check for click
    // timeouts.
    void ProcessUpdate(bool is_alternate_mode);

    // ------------------------------------------------------------------------
    // Handler interface.
    // ------------------------------------------------------------------------
    virtual bool HandleEvent(const Event &event) override;
    virtual void Reset() override;

  private:
    class Impl_;   // This does most of the work.

    std::unique_ptr<Impl_> impl_;
};
