#include "Tests/Panels/PanelTestBase.h"

#include "Panes/ContainerPane.h"
#include "SG/Search.h"
#include "Util/String.h"

Str PanelTestBase::GetContentsString_() {
    return R"(
  children: [
    <"nodes/templates/RadialMenu.emd">, # Required for RadialMenuPanel
    <"nodes/Panels.emd">,
  ]
)";
}

void PanelTestBase::SetTestContext_(Panel &panel) {
    Panel::ContextPtr pc(new Panel::Context);
    panel.SetTestContext(pc);
}

SG::NodePtr PanelTestBase::FindPane_(const Panel &panel, const Str &name) {
    return SG::FindNodeUnderNode(*panel.GetPane(), name);
}
