#pragma once

#include <memory>
#include <string>

#include "Panel.h"
#include "Util/Notifier.h"

/// ToolPanel is a derived Panel class that is an abstract base class for any
/// Panel that is used for a SpecializedTool. It always has a Done button for
/// closing the panel (allowing the app to switch back to a general tool).
///
/// ToolPanel provides an easy way for Tools to respond to user interaction
/// with any of the interactive Panes. The Notifier returned by
/// GetInteraction() is passed a key string identifying the source of the
/// interaction and an InteractionType that indicates the type of interaction.
/// Note that the key strings are defined by derived classes.
///
/// \ingroup Panels
class ToolPanel : public Panel {
  public:
    /// Enum passed to the GetInteraction() observers when interaction occurs.
    /// If the interaction is draggable (such as a slider), the InteractionType
    /// passed to the function will be kStartDrag, kDrag, or kEndDrag. For
    /// other types, the InteractionType will be kImmediate.
    enum class InteractionType {
        kDragStart,  //! Start of a  drag interaction.
        kDrag,       //! Continuation of a  drag interaction.
        kDragEnd,    //! End of a  drag interaction.
        kImmediate,  //! Non-drag-based interaction.
    };

    /// Returns a Notifier that is invoked when interaction occurs in the
    /// ToolPanel.
    Util::Notifier<const std::string &, InteractionType> & GetInteraction() {
        return interaction_;
    }

  protected:
    /// Derived classes should call this when interaction occurs within the
    /// ToolPanel.
    void ReportChange(const std::string &key,
                      InteractionType type = ChangeType::kImmediate) {
        interaction_.Notify(key, type);
    }

  private:
    Util::Notifier<const std::string &, InteractionType> interaction_;
};

typedef std::shared_ptr<ToolPanel> ToolPanelPtr;
