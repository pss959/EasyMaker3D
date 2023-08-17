#include "Tests/Panes/PaneTestBase.h"

Str PaneTestBase::GetContentsString_(const Str &type_name,
                                     const Str &pane_contents) {
    const Str s = R"(
  children: [
    Node {
      CONSTANTS: [
          BG:     "CLONE \"T_PaneBackground\" \"Background\"",
          BORDER: "CLONE \"T_Border\" \"Border\"",
      ],
      TEMPLATES: [
          <"nodes/templates/PaneBackground.emd">,
          <"nodes/templates/Border.emd">,
          <"nodes/templates/Panes/<TYPE>.emd">
      ],
      children: [CLONE "T_<TYPE>" "<TYPE>" { <CONTENTS> }],
    }
  ]
)";

    return Util::ReplaceString(
        Util::ReplaceString(s, "<TYPE>", type_name),
        "<CONTENTS>", pane_contents);
}
