#pragma once

#include "Feedback/Feedback.h"

/// LinearFeedback is a derived Feedback class used for operations that take
/// place along a 3D line.  The feedback consists of uprights joined by a
/// crossbar indicating the length. Text displaying the current length appears
/// above the crossbar.
//
// \ingroup Feedback
class LinearFeedback : public Feedback {
  public:
    virtual void SetColor(const Color &color) override;

    /// Sets the feedback to span between two given points in Stage
    /// coordinates. This asserts that the distance between the points is not
    /// zero.
    void SpanPoints(const Point3f &p0, const Point3f &p1);

    /// Sets the feedback to span a length in Stage coordinates. The feedback
    /// reaches from pt to the point at the signed length along the given
    /// direction vector from pt.
    void SpanLength(const Point3f &pt, const Vector3f &dir, float length);

  protected:
    LinearFeedback();

    virtual void CreationDone() override;

  private:
    class Impl_;
    std::unique_ptr<Impl_> impl_;

    friend class Parser::Registry;
};
typedef std::shared_ptr<LinearFeedback> LinearFeedbackPtr;
