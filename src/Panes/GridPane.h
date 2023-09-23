#pragma once

#include <vector>

#include "Base/Memory.h"
#include "Panes/ContainerPane.h"
#include "Panes/PaneLayout.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(GridPane);

/// GridPane is a derived ContainerPane that arranges contained Panes into a grid
/// of cells with a specified number of rows and columns. Each cell may contain
/// a Pane. Columns are indexed from left to right and rows are indexed from
/// top to bottom. Cells are indexed in row-major order.
///
/// \ingroup Panes
class GridPane : public ContainerPane {
  public:
    virtual Str ToString(bool is_brief) const override;

  protected:
    GridPane() {}

    virtual void AddFields() override;
    virtual bool IsValid(Str &details) override;

    virtual Vector2f ComputeBaseSize() const override;
    virtual void     LayOutSubPanes() override;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::TField<int>      row_count_;
    Parser::TField<int>      column_count_;
    Parser::TField<float>    row_spacing_;
    Parser::TField<float>    column_spacing_;
    Parser::TField<float>    padding_;
    Parser::VField<int>      expanding_rows_;
    Parser::VField<int>      expanding_columns_;
    ///@}

    /// Returns a PaneLayout::GridData for a dimension.
    PaneLayout::GridData GetData_(int dim) const;

    bool CheckDim_(int dim, Str &details);
    bool CheckPanes_(Str &details);

    friend class Parser::Registry;
};
