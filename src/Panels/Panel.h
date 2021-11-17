#pragma once

#include <functional>
#include <string>
#include <unordered_map>

#include "Event.h"
#include "Managers/SessionManager.h"
#include "Managers/SettingsManager.h"
#include "Panes/Pane.h"
#include "SG/Node.h"
#include "SG/Typedefs.h"

#include <vector>

/// Panel is an abstract base class for all panels used for 2D-ish interaction.
/// It can be attached to a Board to appear in the scene.
class Panel : public SG::Node {
  public:
    /// The Panel::Context stores everything a Panel might need to operate.
    struct Context {
        SessionManagerPtr  session_manager;
        SettingsManagerPtr settings_manager;
    };
    typedef std::shared_ptr<Context> ContextPtr;

    /// Typedef for a function that is invoked when the panel is closed by some
    /// user interaction. A string representing the result is supplied.
    typedef std::function<void(const std::string &)> ClosedFunc;

    virtual void AddFields();
    virtual bool IsValid(std::string &details) override;

    /// Sets a Context that can be used by derived Panel classes during their
    /// operation.
    void SetContext(const ContextPtr &context);

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
    /// Defines a function that is invoked when a button is clicked.
    typedef std::function<void(void)> ButtonFunc;

    Panel() {}

    /// Allows derived tool classes to access the Context.
    Context & GetContext() const;

    /// This is called when the Panel is first created. It allows derived
    /// classes to initialize interface items, such as registering buttons. The
    /// base class defines this to do nothing.
    virtual void InitInterface() {}

    /// This is called before the Panel is shown. It allows derived classes to
    /// update interface items, such as enabling or disabling buttons. The base
    /// class defines this to do nothing.
    virtual void UpdateInterface() {}

    /// Adds a button with the given name and a function to invoke when the
    /// button is clicked.
    void AddButtonFunc(const std::string &name, const ButtonFunc &func);

    /// Closes the panel, reporting the given result string.
    void Close(const std::string &result);

    /// Convenience that returns the current application Settings.
    const Settings & GetSettings() const;

    /// Convenience that sets the text in the TextPane inside the ButtonPane
    /// with the given name. Asserts if it is not found.
    void SetButtonText(const std::string &name, const std::string &text);

    /// Convenience that enables or disables the PushButtonWidget in the
    /// ButtonPane with the given name. Asserts if it is not found.
    void EnableButton(const std::string &name, bool enabled);

    /// Sets the focus to the named Pane. Asserts if there is no such Pane.
    void SetFocus(const std::string &name);

  private:
    typedef std::unordered_map<std::string, ButtonFunc> ButtonFuncMap_;

    /// \name Parsed Fields
    ///@{
    Parser::ObjectField<Pane> pane_{"pane"};
    Parser::TField<bool>      is_movable_{"is_movable",     true};
    Parser::TField<bool>      is_resizable_{"is_resizable", false};
    ///@}

    ContextPtr context_;

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

    /// Maps known buttons to their functions to invoke.
    ButtonFuncMap_ button_func_map_;

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

    /// Finds the Pane with the given name, asserting if it is not found.
    PanePtr FindPane_(const std::string &name);
};

typedef std::shared_ptr<Panel> PanelPtr;
