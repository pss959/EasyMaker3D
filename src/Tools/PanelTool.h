#pragma once

#include "Panels/ToolPanel.h"
#include "Tools/SpecializedTool.h"
#include "Util/Assert.h"
#include "Util/General.h"

/// PanelTool is a derived SpecializedTool class that serves as an abstract
/// base class for tools that use a ToolPanel of some sort for editing a
/// Model.
///
/// \ingroup Tools
class PanelTool : public SpecializedTool {
  public:
    virtual void UpdateGripInfo(GripInfo &info) override;

    /// Redefines this to do nothing if in the middle of a drag.
    virtual void ReattachToSelection() override {
        if (! is_dragging_)
            SpecializedTool::ReattachToSelection();
    }

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
    /// The default implementation saves a flag during a drag operation so that
    /// reattaching is disabled. Derived classes can call this version and then
    /// add extra handling.
    virtual void PanelChanged(const std::string &key,
                              ToolPanel::InteractionType type);

  private:
    PanelPtr panel_;  ///< Panel interacting with. (Null when not attached.)

    bool is_dragging_ = false;  ///< Set to true during a drag interaction.
};
