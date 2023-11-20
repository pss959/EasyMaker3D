#pragma once

#include <vector>

#include "Enums/Action.h"

/// The HelpMap class manages constant strings associated with Action
/// values. The strings are used as help tooltips and in the cheat sheet.
///
/// \ingroup Base
class HelpMap {
  public:
    HelpMap();

    /// Returns the string for the given Action.
    const Str & GetHelpString(Action action) const;

  private:
    /// Help string for each Action.
    StrVec help_strings_;
};
