#pragma once

#include <string>

#include "Base/Memory.h"
#include "Widgets/IconWidget.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(IconSwitcherWidget);

/// IconSwitcherWidget is a derived IconWidget class that allows the icon to
/// change between several different shapes, based on an index field. The index
/// selects the child to display as the icon.
///
/// \ingroup Widgets
class IconSwitcherWidget : public IconWidget {
  public:
    /// Sets the index of the child to display.
    void SetIndex(int index);

    /// Returns the index of the current child being displayed. If none is
    /// displayed, this returns -1.
    int GetIndex() const { return index_; }

    /// Convenience that set the index to that of the child with the given
    /// name. Asserts if it is not found.
    void SetIndexByName(const std::string &name);

    /// Redefines this to also make all child icons a consistent size.
    virtual void FitIntoCube(float size, const Point3f &center) override;

  protected:
    IconSwitcherWidget() {}

    virtual void AddFields() override;
    virtual void CreationDone() override;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::TField<int> index_{"index", -1};
    ///@}

    /// Updates when the index changes.
    void UpdateIndex_(int new_index, bool force_update);

    friend class Parser::Registry;
};
