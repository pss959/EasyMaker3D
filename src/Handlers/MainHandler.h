#pragma once

#include <functional>

#include "ClickInfo.h"
#include "Event.h"
#include "Handlers/Handler.h"
#include "Memory.h"
#include "Util/Notifier.h"

DECL_SHARED_PTR(Grippable);
DECL_SHARED_PTR(MainHandler);
DECL_SHARED_PTR(PrecisionManager);
DECL_SHARED_PTR(SceneContext);

/// MainHandler is a derived Handler that does most of the interactive event
/// handling for the application.
///
/// \ingroup Handlers
class MainHandler : public Handler {
  public:
    /// Typedef for function passed to SetPathFilter().
    typedef std::function<bool(const SG::NodePath &path)> PathFilter;

    MainHandler();
    virtual ~MainHandler();

    /// Sets the PrecisionManager used for interaction.
    void SetPrecisionManager(const PrecisionManagerPtr &precision_manager);

    /// Sets the SceneContext to interact with.
    void SetSceneContext(const SceneContextPtr &context);

    /// Adds a Grippable instance that responds to grip-related events to a
    /// list. The order in which instances are added is important: the first
    /// Grippable in the list that responds true for IsGrippableEnabled() is
    /// the one that will be asked about grip interaction.
    void AddGrippable(const GrippablePtr &grippable);

    /// Returns a Notifier that is invoked when a click is detected. An
    /// Observer is passed a ClickInfo instance containing all relevant data.
    Util::Notifier<const ClickInfo &> & GetClicked();

    /// Returns a Notifier that is invoked when a valuator of some sort (such
    /// as a mouse wheel) changes. It is passed the device and the relative
    /// change in position.
    Util::Notifier<Event::Device, float> & GetValuatorChanged();

    /// Returns true if the MainHandler is waiting for something to happen,
    /// meaning that it is not in the middle of handling a button press, drag
    /// operation, or waiting for a click timer.
    bool IsWaiting() const;

    /// Sets a path filter function. If this is not null (which it is by
    /// default), the function is called whenever a pointer ray is intersected
    /// with the scene to find an active Widget. The function is passed the
    /// path to the intersected object and should return true if a Widget on
    /// the path should be hovered or activated or false if it should not. This
    /// is used to "grab" events for items such as panels that are transient
    /// but that take focus. Passing a null function clears this behavior.
    void SetPathFilter(const PathFilter &filter);

    /// This should be called explicitly each frame to check for click
    /// timeouts.
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
