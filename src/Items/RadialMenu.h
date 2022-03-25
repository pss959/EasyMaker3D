#pragma once

#include <vector>

#include "Enums/Action.h"
#include "Math/Types.h"
#include "Memory.h"
#include "SG/Node.h"
#include "Util/Notifier.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(RadialMenu);
DECL_SHARED_PTR(RadialMenuInfo);
DECL_SHARED_PTR(PushButtonWidget);

/// A RadialMenu instance provides functions for using or editing a radial menu
/// whether in VR or not. The menu consists of two circles (outer and inner)
/// with buttons laid out radially between them.
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

  protected:
    RadialMenu() {}

    virtual void CreationDone() override;

  private:
    /// Notifies when a button is clicked.
    Util::Notifier<size_t, Action> button_clicked_;

    /// Button template that is cloned to create each menu button.
    PushButtonWidgetPtr button_;

    /// Node that all buttons are added to.
    SG::NodePtr         buttons_;

    void InitCircle_(const std::string &name, float radius);
    PushButtonWidgetPtr InitButton_(size_t count, size_t index, Action action);
    static std::vector<Point2f> GetButtonPoints_(size_t count, size_t index,
                                                 Point2f &center);

    friend class Parser::Registry;
};
