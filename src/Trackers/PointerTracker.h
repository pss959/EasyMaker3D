#pragma once

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
    /// Alias for function passed to SetPathFilter().
    using PathFilter = std::function<bool(const SG::NodePath &path)>;

    explicit PointerTracker(Actuator actuator) : Tracker(actuator) {}

    /// Sets the path filter. See the documentation for
    /// MainHandler::SetPathFilter() for details.
    void SetPathFilter(const PathFilter &filter) { path_filter_ = filter; }

    virtual void UpdateHovering(const Event &event) override;
    virtual void StopHovering() override;
    virtual void FillActivationDragInfo(DragInfo &info) override;
    virtual bool MovedEnoughForDrag(const Event &event) override;
    virtual void FillEventDragInfo(const Event &event, DragInfo &info) override;
    virtual void FillClickInfo(ClickInfo &info) override;
    virtual void Reset() override;

  protected:
    /// Derived classes must define this to set Ray and return true if the
    /// given Event contains information for the appropriate actuator.
    virtual bool GetRay(const Event &event, Ray &ray) = 0;

    /// Returns the minimum change in the ray angle for MovedEnoughForDrag() to
    /// return true.
    virtual Anglef GetMinRayAngleChange() const = 0;

    /// Once an intersection is found with the current Ray, this is called to
    /// let derived classes update state based on the SG::Hit.
    virtual void ProcessCurrentHit(const SG::Hit &hit) = 0;

    /// If there is a Widget pointed to by the given Event, this sets \p widget
    /// and the current Widget to it and returns it. Otherwise, it leaves \p
    /// widget alone and returns null. If \p is_activation is true, this also
    /// saves the activation Ray and Hit.
    bool GetCurrentWidget(const Event &event, bool is_activation,
                          WidgetPtr &widget);

  private:
    PathFilter    path_filter_;
    Ray           current_ray_;     ///< Ray for the current Event.
    SG::Hit       current_hit_;     ///< Hit for the current Event.
    Ray           activation_ray_;  ///< Ray for the Event at activation.
    SG::Hit       activation_hit_;  ///< Hit for the Event at activation.
    WidgetPtr     current_widget_;  ///< Current tracked Widget (or null).

    /// If the given Event relates to this Tracker, this updates the current
    /// Data_, stores the intersected Widget, if any, in widget, and returns
    /// true.
    bool UpdateCurrentData_(const Event &event, WidgetPtr &widget);
};
