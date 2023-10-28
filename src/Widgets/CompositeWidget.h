#pragma once

#include <vector>

#include "Math/Types.h"
#include "Util/Memory.h"
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
    WidgetPtr GetSubWidget(const Str &name) {
        return FindSubWidget_(name).widget;
    }

    /// Highlights the named sub-widget with the given Color.
    void HighlightSubWidget(const Str &name, const Color &color);

    /// Undoes the highlight applied to the named sub-widget.
    void UnhighlightSubWidget(const Str &name);

  protected:
    CompositeWidget();

    /// Derived classes should call this to find the sub-widget with the given
    /// name and add it to the list of known sub-widgets.  This asserts if the
    /// sub-widget is not found. This also automatically hooks up the
    /// sub-widget's activation Notifier to the Notifier for the
    /// CompositeWidget.
    WidgetPtr AddSubWidget(const Str &name);

    /// Convenience that adds a sub-widget of a given type.
    template <typename T> std::shared_ptr<T> AddTypedSubWidget(
        const Str &name) {
        return std::dynamic_pointer_cast<T>(AddSubWidget(name));
    }

    /// This is invoked when any sub-widget is activated or deactivated. It
    /// notifies all observers of the CompositeWidget by default. Derived
    /// classes can add their own behaviors.
    virtual void SubWidgetActivated(const Str &name, bool is_activation);

  private:
    struct SubWidget_ {
        WidgetPtr widget;
        Color     active_color;    ///< Allows color to be restored.
        Color     inactive_color;  ///< Allows color to be restored.
    };

    std::vector<SubWidget_> sub_widgets_;

    /// Finds and returns the named SubWidget_ in the vector or in any
    /// sub-widget that is also a CompositeWidget. Asserts if not there.
    SubWidget_ & FindSubWidget_(const Str &name);

    /// Used by FindSubWidget_() to do a recursive search.
    SubWidget_ * FindSubWidgetRecursive_(const Str &name);
};
