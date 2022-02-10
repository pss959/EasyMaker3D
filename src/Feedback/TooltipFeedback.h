#pragma once

#include "Feedback/Feedback.h"

/// TooltipFeedback is a derived Feedback class used for showing a text
/// tooltip.
// \ingroup Feedback
class TooltipFeedback : public Feedback {
  public:
    /// Sets the number of seconds to delay showing a tooltip after it is
    /// activated.
    static void SetDelay(float seconds);

    /// Redefines this to return true, as tooltips are placed relative to the
    /// viewer in world coordinates.
    virtual bool IsInWorldCoordinates() const override { return true; }

    /// Sets the text to display
    void SetText(const std::string &text);

    /// Returns the size of the text in object coordinates. This works only
    /// after SetText() is called.
    Vector3f GetTextSize() const;

    /// Redefines this to set the background color for the text. The text color
    /// is always black.
    virtual void SetColor(const Color &color) override;

    /// Redefines this to show the tooltip after the current tooltip delay.
    virtual void Activate() override;

    /// Redefines this to cancel showing the tooltip if it is waiting to be
    /// displayed.
    virtual void Deactivate() override;

  protected:
    TooltipFeedback();

    virtual void CreationDone() override;

  private:
    class Impl_;
    std::unique_ptr<Impl_> impl_;

    friend class Parser::Registry;
};
typedef std::shared_ptr<TooltipFeedback> TooltipFeedbackPtr;
