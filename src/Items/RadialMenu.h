#pragma once

#include <vector>

#include "Base/Memory.h"
#include "Enums/Action.h"
#include "Math/Curves.h"
#include "Math/Types.h"
#include "SG/Node.h"
#include "Util/Notifier.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(RadialMenu);
DECL_SHARED_PTR(RadialMenuInfo);
DECL_SHARED_PTR(PushButtonWidget);

/// A RadialMenu instance provides functions for using or editing a radial menu
/// whether in VR or not. The menu consists of two circles (outer and inner)
/// with buttons laid out radially between them.
///
/// \ingroup Items
class RadialMenu : public SG::Node {
  public:
    /// Returns a Notifier that is invoked when one of the RadialMenu buttons
    /// is clicked. It is passed the index of the button and the Action
    /// associated with that button.
    Util::Notifier<size_t, Action> & GetButtonClicked() {
        return button_clicked_;
    }

    /// Updates the RadialMenu from the given RadialMenuInfo. If update_enabled
    /// is true, this disables any button if its action is Action::kNone.
    void UpdateFromInfo(const RadialMenuInfo &info, bool update_enabled);

    /// Changes the action associated with the indexed button. If
    /// update_enabled is true, this disables the button if the action is
    /// Action::kNone.
    void ChangeButtonAction(size_t index, Action action, bool update_enabled);

    /// Highlights the button (if any) at the given angle.
    void HighlightButton(const Anglef &angle);

    /// Clears the highlight from the highlighted button.
    void ClearHighlightedButton();

    /// Simulates a button press on the currently highlighted button.
    void SimulateButtonPress();

  protected:
    RadialMenu() {}

    virtual void CreationDone() override;

  private:
    /// This struct stores everything needed to process buttons.
    struct Button_ {
        size_t              index;
        CircleArc           arc;
        PushButtonWidgetPtr widget;
    };

    /// Notifies when a button is clicked.
    Util::Notifier<size_t, Action> button_clicked_;

    /// Button template that is cloned to create each menu button.
    PushButtonWidgetPtr  button_;

    /// Node that all buttons are added to.
    SG::NodePtr          button_parent_;

    /// Info about all buttons.
    std::vector<Button_> buttons_;

    /// Button Widget last highlighted by HighlightButton().
    PushButtonWidgetPtr highlighted_button_;

    void InitCircle_(const std::string &name, float radius);
    void InitButton_(size_t count, size_t index, Button_ &button);
    static std::vector<Point2f> GetButtonPoints_(size_t count, size_t index,
                                                 Point2f &center);
    static CircleArc ComputeArc_(size_t count, size_t index, float margin);

    friend class Parser::Registry;
};
