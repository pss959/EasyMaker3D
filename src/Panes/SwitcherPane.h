#pragma once

#include "Memory.h"
#include "Panes/ContainerPane.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(SwitcherPane);

/// SwitcherPane is a derived ContainerPane that displays none or one of its
/// contained Panes at any time. The index field indicates which one is
/// displayed; a negative or too-large value turns off all display. The
/// SwitcherPane is sized to the maximum size of all of its contained Panes.
class SwitcherPane : public ContainerPane {
  public:
    /// Sets the index of the contained Pane to display.
    void SetIndex(int index);

    /// Returns the index of the current Pane being displayed. If none is
    /// displayed, this returns -1.
    int GetIndex() const { return index_; }

    virtual std::string ToString() const override;

  protected:
    SwitcherPane() {}

    virtual void AddFields() override;
    virtual void CreationDone() override;

    virtual Vector2f ComputeBaseSize() override;
    virtual void     LayOutPanes(const Vector2f &size) override;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::TField<int> index_{"index", -1};
    ///@}

    /// Updates when the index changes.
    void UpdateIndex_(int new_index, bool force_update);

    friend class Parser::Registry;
};
