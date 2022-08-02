#pragma once

#include "Panels/ToolPanel.h"
#include "Tools/Tool.h"
#include "Util/Assert.h"
#include "Util/General.h"

/// PanelTool is a derived Tool class that serves as an abstract base class for
/// tools that use a ToolPanel of some sort for editing a Model.
///
/// \ingroup Tools
class PanelTool : public Tool {
  protected:
    /// Defines this to access the correct Panel (using GetPanelTypeName() for
    /// the type), attach it to the tool Board, and to store it in the
    /// instance.
    virtual void Attach() override;

    /// Defines this to detach the Panel and close the Board.
    virtual void Detach() override;

    /// Convenience that returns the attached Panel as the given type. Asserts
    /// if the PanelTool is not currently attached or if the type is wrong.
    template <typename T> T & GetTypedPanel() const {
        ASSERT(panel_);
        ASSERT(Util::IsA<T>(panel_));
        return *Util::CastToDerived<T>(panel_);
    }

    /// Derived classes must implement to return the name of the type of Panel
    /// to open when attaching to a Model of the appropriate type.
    virtual std::string GetPanelName() const = 0;

    /// This is called when a Panel is opened. The base class defines it to do
    /// nothing. Derived classes can redefine it to set up the Panel from the
    /// attached Model.
    virtual void InitPanel() {};

    /// This is called by ToolPanel::ReportChange() when interaction occurs.
    /// The default implementation does nothing.
    virtual void PanelChanged(const std::string &key,
                              ToolPanel::InteractionType type) {}

  private:
    ToolPanelPtr panel_;  ///< Panel interacting with. (Null when not attached.)

    bool is_dragging_ = false;  ///< Set to true during a drag interaction.
};
