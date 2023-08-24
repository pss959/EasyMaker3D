#pragma once

#include <functional>
#include <string>

#include "Base/Event.h"
#include "Base/Memory.h"
#include "Panes/ContainerPane.h"
#include "SG/Node.h"

#include <vector>

struct TouchInfo;
DECL_SHARED_PTR(ActionAgent);
DECL_SHARED_PTR(BoardAgent);
DECL_SHARED_PTR(Border);
DECL_SHARED_PTR(ButtonPane);
DECL_SHARED_PTR(ClickableWidget);
DECL_SHARED_PTR(NameAgent);
DECL_SHARED_PTR(Panel);
DECL_SHARED_PTR(SelectionAgent);
DECL_SHARED_PTR(SessionAgent);
DECL_SHARED_PTR(Settings);
DECL_SHARED_PTR(SettingsAgent);
DECL_SHARED_PTR(VirtualKeyboard);
DECL_SHARED_PTR(Widget);

/// Panel is an abstract base class for all panels used for 2D-ish interaction.
/// It can be attached to a Board to appear in the scene. A Panel wraps a tree
/// of Pane instances. The root of the tree is a ContainerPane of some type.
///
/// The coordinate system used in Panels and Panes assumes 1 unit is
/// approximately the size of a pixel in a full-screen window. It is up to the
/// Board to establish this coordinate system.
///
/// \ingroup Panels
class Panel : public SG::Node {
  public:
    /// The Panel::Context stores everything a Panel might need to operate.
    struct Context {
        ActionAgentPtr      action_agent;
        BoardAgentPtr       board_agent;
        NameAgentPtr        name_agent;
        SelectionAgentPtr   selection_agent;
        SessionAgentPtr     session_agent;
        SettingsAgentPtr    settings_agent;
        VirtualKeyboardPtr  virtual_keyboard;  ///< Null if VR not enabled.
    };
    typedef std::shared_ptr<Context> ContextPtr;

    /// Sets a Context that can be used by derived Panel classes during their
    /// operation. Derived classes can add their own behavior, but must call
    /// this version.
    virtual void SetContext(const ContextPtr &context);

    /// Version of SetContext() used for testing - not all parts need to be
    /// present in the Context.
    void SetTestContext(const ContextPtr &context);

    /// Returns true if the size of this Panel may have changed since it was
    /// last set.
    bool SizeMayHaveChanged() const { return size_may_have_changed_; }

    /// Returns the root ContainerPane for the Panel.
    const ContainerPanePtr & GetPane() const { return pane_; }

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

    /// Returns the current size of the Panel, which is the size of the root
    /// Pane if it has one, or zero otherwise.
    Vector2f GetSize() const;

    /// Makes sure everything in the Panel is up to date with its current size
    /// and returns the updated size. This needs to be called when contents may
    /// have changed.
    Vector2f UpdateSize();

    /// Returns the minimum size of the Panel, which is the base size of the
    /// root Pane if it has one, or zero otherwise.
    Vector2f GetMinSize() const;

    /// This is called by a Board to potentially handle an event. The base
    /// class defines this to handle escape key, navigation, etc..
    virtual bool HandleEvent(const Event &event);

    /// Sets a flag indicating whether the Panel is shown. This allows the
    /// Panel to set up navigation and anything else it needs.
    void SetIsShown(bool is_shown);

    /// Returns a flag indicating whether the Panel is shown. The default is
    /// false.
    bool IsShown() const { return is_shown_; }

    /// Returns the currently focused Pane, or null if there is none.
    PanePtr GetFocusedPane() const;

    /// Returns the Widget from an interactive Pane that should respond to a
    /// touch represented by the given TouchInfo. If more than one Widget is
    /// touched, the closest one is returned.
    WidgetPtr GetTouchedPaneWidget(const TouchInfo &info);

    /// Returns true if the derived class supports grip hovering. The base
    /// class defines this to always return false.
    virtual bool CanGripHover() const { return false; }

    /// If CanGripHover() returns true, this is called to get the correct
    /// ClickableWidget to hover based on the current controller position (in
    /// Panel coordinates). The base class defines this to return null.
    virtual ClickableWidgetPtr GetGripWidget(const Point2f &panel_point) {
        return ClickableWidgetPtr();
    }

    virtual void PostSetUpIon() override;

  protected:
    /// Defines a function that is invoked when a button is clicked.
    typedef std::function<void(void)> ButtonFunc;

    /// Type of function that is invoked by DisplayMessage().
    typedef std::function<void(void)> MessageFunc;

    /// Type of function that is invoked by AskQuestion().
    typedef std::function<void(const Str &)> QuestionFunc;

    Panel();
    ~Panel() override;

    virtual void AddFields();
    virtual bool IsValid(Str &details) override;
    virtual void CreationDone() override;

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

    /// This is called when the size of the contained Pane may have changed,
    /// allowing derived classes to update anything that is
    /// size-dependent. THis will be called at least once when the original
    /// Pane size is determined. The base class defines this to do nothing.
    virtual void UpdateForPaneSizeChange() {}

    /// Allows derived classes to reset the Panel size to zero so that the next
    /// time it is opened the base size is used.
    void ResetSize();

    /// Indicates whether the Panel should trap valuator events, so that the
    /// scroll wheel does not affect anything else in the app. The base class
    /// defines this to true.
    virtual bool ShouldTrapValuatorEvents() const { return true; }

    /// Convenience that returns the current application Settings.
    const Settings & GetSettings() const;

    /// \name ButtonPane conveniences
    ///@{

    /// Convenience that finds the ButtonPane with the given name and sets it
    /// to invoke the given function when clicked.
    void AddButtonFunc(const Str &name, const ButtonFunc &func);

    /// Convenience that sets the text in the TextPane inside the ButtonPane
    /// with the given name. Asserts if it is not found.
    void SetButtonText(const Str &name, const Str &text);

    /// Convenience that enables or disables the ButtonPane with the given
    /// name. Asserts if it is not found.
    void EnableButton(const Str &name, bool enabled);

    /// \name Focus management.
    ///@{

    /// Sets the focus to the given Pane. Asserts if there is no such Pane.
    void SetFocus(const PanePtr &pane);

    /// Sets the focus to the named Pane. Asserts if there is no such Pane.
    void SetFocus(const Str &name);

    /// This is called after focus changes to the given Pane. The base class
    /// implements this to do nothing.
    virtual void UpdateFocus(const PanePtr &pane) {}

    ///@}

    /// Convenience that opens a DialogPanel to display the given message along
    /// with an "OK" button that invokes the given function (if not null).
    void DisplayMessage(const Str &message, const MessageFunc &func);

    /// Convenience that opens a DialogPanel to ask the given question and get
    /// a "Yes" or "No" result, which is passed to the given function. The
    /// is_no_default flag indicates which button should be focused by default.
    void AskQuestion(const Str &question, const QuestionFunc &func,
                     bool is_no_default);

    /// Convenience that calls Close on the BoardAgent. Derived classes can
    /// modify this behavior.
    virtual void Close(const Str &result);

    /// Uses the BoardAgent to get the named Panel. Asserts if the name is not
    /// known.
    PanelPtr GetPanel(const Str &name) const;

    /// Same as GetPanel(), but requires that the Panel is of the given derived
    /// type. Asserts if not found.
    template <typename T>
    std::shared_ptr<T> GetTypedPanel(const Str &name) const {
        auto panel = std::dynamic_pointer_cast<T>(GetPanel(name));
        ASSERT(panel);
        return panel;
    }

  private:
    class Focuser_;  ///< Handles Pane focus management.

    typedef std::unordered_map<ButtonPanePtr, ButtonFunc> ButtonFuncMap_;

    /// \name Parsed Fields
    ///@{
    Parser::ObjectField<ContainerPane> pane_;
    Parser::TField<bool>               is_movable_;
    Parser::TField<bool>               is_resizable_;
    ///@}

    std::unique_ptr<Focuser_> focuser_;

    ContextPtr context_;

    /// True while the Panel is shown.
    bool is_shown_ = false;

    /// Set to true if the Panel size may have changed.
    bool size_may_have_changed_ = false;

    /// Finds all interactive Panes and sets up the Focuser_.
    void UpdateInteractivePanes_();

    /// Recursively finds all interactive Panes under the given one (inclusive)
    /// and adds them to the given vector.
    void FindInteractivePanes_(const PanePtr &pane,
                               std::vector<PanePtr> &panes);

    /// Handles an event with a key press.
    bool ProcessKeyPress_(const Event &event);

    /// This is invoked when the contents of the root Pane have changed.
    void ProcessPaneContentsChange_();

    /// Returns true if the given Pane can be focused.
    bool CanFocusPane_(Pane &pane) const;
};
