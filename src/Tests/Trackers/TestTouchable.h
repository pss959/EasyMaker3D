//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

#include "Util/Memory.h"
#include "Widgets/ITouchable.h"

DECL_SHARED_PTR(Widget);

/// Derived ITouchable class used for easier testing. It allows the Widget
/// returned by GetTouchedWidget() to be set as a public member variable.
///
/// \ingroup Tests
class TestTouchable : public ITouchable {
  public:
    WidgetPtr widget;  ///< Widget returned by GetTouchedWidget().

    virtual WidgetPtr GetTouchedWidget(const Point3f &, float) const override {
        return widget;
    }
};

DECL_SHARED_PTR(TestTouchable);
