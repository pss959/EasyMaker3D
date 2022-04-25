#pragma once

#include "Feedback/Feedback.h"
#include "Memory.h"

DECL_SHARED_PTR(AngularFeedback);

/// AngularFeedback is a derived Feedback class used for operations that
/// involve a radial angle. The feedback consists of two lines and an arc
/// subtending the angle.
///
/// \ingroup Feedback
class AngularFeedback : public Feedback {
  public:
    virtual void SetColor(const Color &color) override;

    /// Modifies the feedback to subtend an angle. The angle origin is put at
    /// center (in stage coordinates), offset up in Y by the given amount. The
    /// text is offset by an optional amount (text_up_offset). The feedback
    /// shows the rotation around the given axis subtending the angle indicated
    /// by start_angle and end_angle.
    void SubtendAngle(const Point3f &center, float up_offset,
                      float text_up_offset, const Vector3f &axis,
                      const Anglef &start_angle, const Anglef &end_angle);

  protected:
    AngularFeedback();

    virtual void CreationDone() override;

  private:
    class Impl_;
    std::unique_ptr<Impl_> impl_;

    friend class Parser::Registry;
};
