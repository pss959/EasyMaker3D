#pragma once

#include <memory>
#include <string>

#include "Widgets/IconWidget.h"

namespace Parser { class Registry; }

/// IconSwitcherWidget is a derived IconWidget class that allows the icon to
/// change between several different shapes, based on an index field. The index
/// selects the child to display as the icon.
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

typedef std::shared_ptr<IconSwitcherWidget> IconSwitcherWidgetPtr;
