#pragma once

#include <functional>
#include <string>

#include "Event.h"
#include "Panes/Pane.h"
#include "SG/Node.h"
#include "SG/Typedefs.h"

#include <vector>

/// Panel is an abstract base class for all panels used for 2D-ish interaction.
/// It can be attached to a Board to appear in the scene.
class Panel : public SG::Node {
  public:
    /// Typedef for a function that is invoked when the panel is closed by some
    /// user interaction. A string representing the result is supplied.
    typedef std::function<void(const std::string &)> ClosedFunc;

    virtual void AddFields();
    virtual bool IsValid(std::string &details) override;

    /// Sets a function that is invoked when the panel is closed by some user
    /// interaction. A string representing the result is supplied. This
    /// function is null by default.
    void SetClosedFunc(const ClosedFunc &func) { closed_func_ = func; }

    /// Returns the root Pane for the Panel.
    const PanePtr & GetPane() const { return pane_; }

    /// Returns true if the Board containing the Panel should be movable. The
    /// base class defines this to return true.
    virtual bool IsMovable() const { return is_movable_; }

    /// Returns true if the Board containing the Panel should be resizable. The
    /// base class defines this to return false.
    virtual bool IsResizable() const { return is_resizable_; }

    /// Sets the size of the Panel. This can always be called to set the
    /// initial size and can also be used to resize the Panel if IsResizable()
    /// returns true.
    void SetSize(const Vector2f &size);

    /// Returns the minimum size of the Panel, which is computed by the root
    /// Pane.
    Vector2f GetMinSize() const;

    /// This is called by a Board to potentially handle an event. The base
    /// class defines this to handle escape key, navigation, etc..
    virtual bool HandleEvent(const Event &event);

    /// Sets a flag indicating whether the Panel is shown. This allows the
    /// Panel to set up navigation and anything else it needs.
    void SetIsShown(bool is_shown);

    virtual void PreSetUpIon() override;
    virtual void PostSetUpIon() override;

  protected:
    Panel() {}

    /// This function is invoked when a ButtonPane within the Panel is
    /// clicked. It is passed the name of the ButtonPane. The base class
    /// defines this to do nothing.
    virtual void ProcessButton(const std::string &name) {}

    /// Closes the panel, reporting the given result string.
    void Close(const std::string &result);

  private:
    /// \name Parsed Fields
    ///@{
    Parser::ObjectField<Pane> pane_{"pane"};
    Parser::TField<bool>      is_movable_{"is_movable",     true};
    Parser::TField<bool>      is_resizable_{"is_resizable", false};
    ///@}

    /// Function to invoke when the Panel is closed.
    ClosedFunc closed_func_;

    /// All interactive Pane instances found in the Panel. This is used for
    /// highlighting and navigation.
    std::vector<PanePtr> interactive_panes_;

    /// Index into interactive_panes_ of the current Pane with focus. This is
    /// -1 if there is none.
    int focused_index_ = -1;

    /// This PolyLine is used to highlight the interactive Pane with keyboard
    /// focus.
    SG::PolyLinePtr highlight_line_;

    /// Finds all interactive Panes and adds them to the interactive_panes_
    /// vector.
    void FindInteractivePanes_();

    /// Sets up the click callback in all ButtonPanes.
    void SetUpButtons_();

    /// Highlights the focused Pane for keyboard interaction.
    void HighlightFocusedPane_();

    /// This is invoked when the size of any Pane in the panel changes size.
    void ProcessPaneSizeChange_();

    /// Changes focus in the given direction.
    void ChangeFocus_(int increment);
};

typedef std::shared_ptr<Panel> PanelPtr;
