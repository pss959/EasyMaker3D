//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

#include <functional>
#include <vector>

#include "Enums/Action.h"
#include "Math/Curves.h"
#include "Math/Types.h"
#include "SG/Node.h"
#include "Util/Memory.h"
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

    /// Updates the RadialMenu from the given RadialMenuInfo.
    void UpdateFromInfo(const RadialMenuInfo &info);

    /// Changes the action associated with the indexed button.
    void ChangeButtonAction(size_t index, Action action);

    /// Updates the enabled state of all visible buttons by calling the given
    /// function. The function is passed the Action attached to the button.
    void EnableButtons(const std::function<bool(Action)> &func);

    /// Highlights the button (if any) at the given angle (in the range 0-360
    /// degrees).
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
        Action              action;
    };

    /// Notifies when a button is clicked.
    Util::Notifier<size_t, Action> button_clicked_;

    /// Button template that is cloned to create each menu button.
    PushButtonWidgetPtr  button_;

    /// Node that all buttons are added to.
    SG::NodePtr          button_parent_;

    /// Info about all buttons.
    std::vector<Button_> buttons_;

    /// Button last highlighted by HighlightButton(). May be null.
    Button_ *             highlighted_button_ = nullptr;

    void InitCircle_(const Str &name, float radius);
    void InitButton_(size_t count, size_t index, Button_ &button);
    static std::vector<Point2f> GetButtonPoints_(size_t count, size_t index,
                                                 Point2f &center);
    static CircleArc ComputeArc_(size_t count, size_t index, float margin);

    friend class Parser::Registry;
};
