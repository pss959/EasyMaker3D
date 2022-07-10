#pragma once

#include "Base/Event.h"
#include "Math/Types.h"
#include "SG/Hit.h"
#include "SG/NodePath.h"
#include "Trackers/Tracker.h"

/// PointerTracker is a derived Tracker class that serves as a base class for
/// pointer-based trackers (mouse and pinch).
///
/// \ingroup Trackers
class PointerTracker : public Tracker {
  public:
    /// Typedef for function passed to SetPathFilter().
    typedef std::function<bool(const SG::NodePath &path)> PathFilter;

    explicit PointerTracker(Actuator actuator) : Tracker(actuator) {}

    /// Sets the path filter. See the documentation for
    /// MainHandler::SetPathFilter() for details.
    void SetPathFilter(const PathFilter &filter) { path_filter_ = filter; }

    virtual void UpdateHovering(const Event &event) override;
    virtual void StopHovering() override;
    virtual bool MovedEnoughForDrag(const Event &event) override;
    virtual void FillActivationDragInfo(DragInfo &info) override;
    virtual void FillEventDragInfo(const Event &event, DragInfo &info) override;
    virtual void FillClickInfo(ClickInfo &info) override;
    virtual void Reset() override;

  protected:
    /// Derived classes must define this to return the Event::Device
    /// corresponding to the specific Tracker.
    virtual Event::Device GetDevice() const = 0;

    /// Derived classes must define this to set Ray and return true if the
    /// given Event contains information for the appropriate actuator.
    virtual bool GetRay(const Event &event, Ray &ray) = 0;

    /// Once an intersection is found with the current Ray, this is called to
    /// let derived classes update state based on the SG::Hit. This class
    /// defines it to do nothing.
    virtual void ProcessCurrentHit(const SG::Hit &hit) {}

    /// Sets the current Widget to the Widget pointed to by the given Event, if
    /// any. If there is a Widget, this activates it and returns the
    /// Widget. Otherwise, it returns null.
    WidgetPtr ActivateWidget(const Event &event);

    /// Deactivates the current Widget (if any) and returns the Widget
    // pointed to by the given pointer Ray, if any.
    WidgetPtr DeactivateWidget(const Event &event);

  private:
    PathFilter    path_filter_;
    Ray           current_ray_;     ///< Ray for the current Event.
    SG::Hit       current_hit_;     ///< Hit for the current Event.
    Ray           activation_ray_;  ///< Ray for the Event at activation.
    SG::Hit       activation_hit_;  ///< Hit for the Event at activation.
    WidgetPtr     current_widget_;  ///< Current tracked Widget (or null).

    /// Updates the current Ray and Hit based on the given Event and returns
    /// the intersected Widget, if any.
    WidgetPtr UpdateCurrentData_(const Event &event);
};
