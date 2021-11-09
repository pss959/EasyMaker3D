#pragma once

#include <memory>

#include "ClickInfo.h"
#include "Event.h"
#include "Handlers/Handler.h"
#include "Items/Grippable.h"
#include "Managers/PrecisionManager.h"
#include "SceneContext.h"
#include "Util/Notifier.h"
#include "Widgets/ClickableWidget.h"

/// GripHandler is a derived Handler that deals with grip highlighting and
/// dragging. It maintains an ordered vector of Grippable instances. The first
/// enabled Grippable (if any) is queried to see how to do grip hover
/// highlighting and interaction.
/// XXXX Maybe have base ClickDragHandler class for MainHandler/GripHandler.
/// \ingroup Handlers
class GripHandler : public Handler {
  public:
    GripHandler();
    virtual ~GripHandler();

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

    /// Returns true if the GripHandler is waiting for something to happen,
    /// meaning that it is not in the middle of handling a button press, drag
    /// operation, or waiting for a click timer.
    bool IsWaiting() const;

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

typedef std::shared_ptr<GripHandler> GripHandlerPtr;
