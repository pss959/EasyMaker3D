#pragma once

#include <functional>
#include <string>

#include "Memory.h"
#include "Event.h"
#include "Panels/PanelHelper.h"
#include "Panes/ContainerPane.h"
#include "SG/Node.h"
#include "Util/General.h"

#include <vector>

DECL_SHARED_PTR(ButtonPane);
DECL_SHARED_PTR(NameManager);
DECL_SHARED_PTR(Panel);
DECL_SHARED_PTR(SelectionManager);
DECL_SHARED_PTR(SelectionManager);
DECL_SHARED_PTR(SessionManager);
DECL_SHARED_PTR(Settings);
DECL_SHARED_PTR(SettingsManager);
namespace SG { DECL_SHARED_PTR(PolyLine); }

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
        NameManagerPtr      name_manager;
        SelectionManagerPtr selection_manager;
        SessionManagerPtr   session_manager;
        SettingsManagerPtr  settings_manager;
        PanelHelperPtr      panel_helper;
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

    virtual void PostSetUpIon() override;

  protected:
    /// Defines a function that is invoked when a button is clicked.
    typedef std::function<void(void)> ButtonFunc;

    /// Type of function that is invoked by DisplayMessage().
    typedef std::function<void(void)> MessageFunc;

    /// Type of function that is invoked by AskQuestion().
    typedef std::function<void(const std::string &)> QuestionFunc;

    Panel() {}

    virtual void AddFields();
    virtual bool IsValid(std::string &details) override;
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

    /// Redefines this to update the focus if necessary.
    virtual void UpdateForRenderPass(const std::string &pass_name) override;

    /// Convenience that returns the current application Settings.
    const Settings & GetSettings() const;

    /// \name ButtonPane conveniences.
    ///@{

    /// Convenience that finds the ButtonPane with the given name and sets it
    /// to invoke the given function when clicked.
    void AddButtonFunc(const std::string &name, const ButtonFunc &func);

    /// Convenience that sets the text in the TextPane inside the ButtonPane
    /// with the given name. Asserts if it is not found.
    void SetButtonText(const std::string &name, const std::string &text);

    /// Convenience that enables or disables the ButtonPane with the given
    /// name. Asserts if it is not found.
    void EnableButton(const std::string &name, bool enabled);

    ///@}

    /// \name Focus management.
    ///@{

    /// Sets the focus to the given Pane. Asserts if there is no such Pane.
    void SetFocus(const PanePtr &pane);

    /// Sets the focus to the named Pane. Asserts if there is no such Pane.
    void SetFocus(const std::string &name);

    /// Returns the currently focused Pane, or null if there is none.
    PanePtr GetFocusedPane() const;

    ///@}

    /// Convenience that opens a DialogPanel to display the given message along
    /// with an "OK" button that invokes the given function (if not null).
    void DisplayMessage(const std::string &message,
                        const MessageFunc &func);

    /// Convenience that opens a DialogPanel to ask the given question and get
    /// a "Yes" or "No" result, which is passed to the given function.
    void AskQuestion(const std::string &question, const QuestionFunc &func);

    /// Convenience that calls Close on the PanelHelper. Dervied classes can
    /// modify this behavior.
    virtual void Close(const std::string &result) {
        context_->panel_helper->Close(result);
    }

  private:
    typedef std::unordered_map<ButtonPanePtr, ButtonFunc> ButtonFuncMap_;

    /// \name Parsed Fields
    ///@{
    Parser::ObjectField<ContainerPane> pane_{"pane"};
    Parser::TField<bool>               is_movable_{"is_movable",     true};
    Parser::TField<bool>               is_resizable_{"is_resizable", false};
    ///@}

    ContextPtr context_;

    /// Set to true if the Panel size may have changed.
    bool size_may_have_changed_ = false;

    /// Set to true if the focus highlight may need to be updated.
    bool update_focus_highlight_ = false;

    /// All interactive Pane instances found in the Panel. This is used for
    /// highlighting and navigation.
    std::vector<PanePtr> interactive_panes_;

    /// Index into interactive_panes_ of the current Pane with focus. This is
    /// -1 if there is none.
    int focused_index_ = -1;

    /// This PolyLine is used to highlight the interactive Pane with keyboard
    /// focus.
    SG::PolyLinePtr highlight_line_;

    /// Saves the MessageFunc passed to DisplayMessage() so it can be invoked
    /// later.
    MessageFunc  message_func_;

    /// Saves the QuestionFunc passed to AskQuestion() so it can be invoked
    /// later.
    QuestionFunc question_func_;

    /// Finds all interactive Panes under the given one (inclusive) and adds
    /// them to the interactive_panes_ vector.
    void FindInteractivePanes_(const PanePtr &pane);

    /// Initializes interaction for an interactive Pane.
    void InitPaneInteraction_(const PanePtr &pane);

    /// Highlights the focused Pane for keyboard interaction.
    void HighlightFocusedPane_();

    /// This is invoked when the contents of the root Pane have changed.
    void ProcessPaneContentsChange_();

    /// Changes focus in the given direction.
    void ChangeFocusBy_(int increment);

    /// Changes focus to the indexed interactive Pane.
    void ChangeFocusTo_(size_t index);

    /// Activates the given interactive Pane from a button click or key press.
    void ActivatePane_(const PanePtr &pane, bool is_click);
};
