#pragma once

#include <vector>

#include "Base/Memory.h"
#include "Math/Types.h"
#include "Widgets/Widget.h"

DECL_SHARED_PTR(CompositeWidget);

/// CompositeWidget is an abstract base class for widgets that are composed of
/// other types of widgets.
///
/// \ingroup Widgets
class CompositeWidget : public Widget {
  public:
    /// Returns the sub-widget with the given name. Asserts if it is not
    /// found.
    WidgetPtr GetSubWidget(const std::string &name) {
        return FindSubWidget_(name).widget;
    }

    /// Highlights the named sub-widget with the given Color.
    void HighlightSubWidget(const std::string &name, const Color &color);

    /// Undoes the highlight applied to the named sub-widget.
    void UnhighlightSubWidget(const std::string &name);

  protected:
    CompositeWidget();

    /// Derived classes should call this to find the sub-widget with the given
    /// name and add it to the list of known sub-widgets.  This asserts if the
    /// sub-widget is not found. This also automatically hooks up the
    /// sub-widget's activation Notifier to the Notifier for the
    /// CompositeWidget.
    WidgetPtr AddSubWidget(const std::string &name);

    ///@}

  private:
    struct SubWidget_ {
        WidgetPtr widget;
        Color     active_color;    ///< Allows color to be restored.
        Color     inactive_color;  ///< Allows color to be restored.
    };

    std::vector<SubWidget_> sub_widgets_;

    /// Finds and returns the named SubWidget_ in the vector or in any
    /// sub-widget that is also a CompositeWidget. Asserts if not there.
    SubWidget_ & FindSubWidget_(const std::string &name);

    /// Used by FindSubWidget_() to do a recursive search.
    SubWidget_ * FindSubWidgetRecursive_(const std::string &name);
};
