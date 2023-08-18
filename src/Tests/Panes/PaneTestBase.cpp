#include "Tests/Panes/PaneTestBase.h"

#include "Util/String.h"

Str PaneTestBase::GetContentsString_(const Str &type_name,
                                     const Str &pane_contents) {
    const Str base_string = R"(
  children: [
    <"nodes/templates/Panes/AllPanes.emd">,
    CLONE "T_@TYPE@" "@TYPE@" { @CONTENTS@ },
  ]
)";

    Str str = Util::ReplaceString(base_string, "@TYPE@",     type_name);
    str     = Util::ReplaceString(str,         "@CONTENTS@", pane_contents);

    return str;
}
