#include "Tests/Panels/PanelInteractor.h"

#include "Panes/ButtonPane.h"
#include "Panes/CheckboxPane.h"
#include "Panes/ContainerPane.h"
#include "Panes/DropdownPane.h"
#include "Panes/LabeledSliderPane.h"
#include "Panes/RadioButtonPane.h"
#include "Panes/SliderPane.h"
#include "Panes/TextInputPane.h"
#include "Place/ClickInfo.h"
#include "Tests/Widgets/DragTester.h"
#include "Widgets/PushButtonWidget.h"
#include "Widgets/Slider1DWidget.h"

PanelInteractor::PanelInteractor(const PanelPtr &panel) {
    ASSERT(panel);
    panel_ = panel;
}

PanelInteractor::PanelInteractor(const std::function<PanelPtr()> &panel_func) {
    ASSERT(panel_func);
    panel_func_ = panel_func;
}

bool PanelInteractor::IsButtonPaneEnabled(const Str &name) {
    return FindTypedPane<ButtonPane>(name)->IsInteractionEnabled();
}

ButtonPanePtr PanelInteractor::ClickButtonPane(const Str &name) {
    auto but_pane = FindTypedPane<ButtonPane>(name);
    ClickInfo info;  // Contents do not matter.
    but_pane->GetButton().Click(info);
    return but_pane;
}

CheckboxPanePtr PanelInteractor::ToggleCheckboxPane(const Str &name) {
    auto cbox_pane = FindTypedPane<CheckboxPane>(name);
    ClickInfo info;  // Contents do not matter.
    cbox_pane->GetActivationWidget()->Click(info);
    return cbox_pane;
}

DropdownPanePtr PanelInteractor::ChangeDropdownChoice(const Str &name,
                                                    const Str &choice) {
    auto dd_pane = FindTypedPane<DropdownPane>(name);
    dd_pane->SetChoiceFromString(choice, true);  // Notify = true.
    return dd_pane;
}

RadioButtonPanePtr PanelInteractor::ActivateRadioButtonPane(const Str &name) {
    auto rbut_pane = FindTypedPane<RadioButtonPane>(name);
    rbut_pane->SetState(true);
    return rbut_pane;
}

SliderPanePtr PanelInteractor::DragSlider(const Str &name,
                                          const Vector2f &vec) {
    auto lsp = FindTypedPane<LabeledSliderPane>(name)->GetSliderPane();
    auto s1w =
        std::dynamic_pointer_cast<Slider1DWidget>(lsp->GetActivationWidget());

    DragTester dt(s1w);
    dt.SetRayDirection(-Vector3f::AxisZ());
    dt.ApplyMouseDrag(Point3f(0, 0, 0), Point3f(vec[0], vec[1], 0));
    return lsp;
}

TextInputPanePtr PanelInteractor::SetTextInput(const Str &name,
                                               const Str &text) {
    auto input_pane = FindTypedPane<TextInputPane>(name);
    input_pane->GetInteractor()->Activate();
    input_pane->SetInitialText(text);
    input_pane->GetInteractor()->Deactivate();
    return input_pane;
}
