#pragma once

#include <string>

#include "Base/Memory.h"
#include "Panel.h"
#include "Util/Notifier.h"

DECL_SHARED_PTR(ToolPanel);

namespace Parser { class Registry; }

/// ToolPanel is a derived Panel class that is an abstract base class for any
/// Panel that is used by a Panel-based Tool (derived from PanelTool).
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
    ToolPanel() {}

    /// Derived classes should call this when interaction occurs within the
    /// ToolPanel.
    void ReportChange(const std::string &key, InteractionType type) {
        interaction_.Notify(key, type);
    }

  private:
    Util::Notifier<const std::string &, InteractionType> interaction_;

    friend class Parser::Registry;
};
