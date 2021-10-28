#pragma once

#include <memory>
#include <vector>

#include "Panes/MultiPane.h"

namespace Parser { class Registry; }

/// GridPane is a derived MultiPane that arranges contained Panes into a grid
/// of cells with a specified number of rows and columns. Each cell may contain
/// a Pane. Columns are indexed from left to right and rows are indexed from
/// top to bottom. Cells are indexed in row-major order.
class GridPane : public MultiPane {
  public:
    virtual void AddFields() override;
    virtual bool IsValid(std::string &details) override;

    /// Defines this to set the size on all cell panes.
    virtual void SetSize(const Vector2f &size) override;

  protected:
    GridPane() {}

  private:
    /// \name Parsed Fields
    ///@{
    Parser::TField<int>      row_count_{"row_count", 1};
    Parser::TField<int>      column_count_{"column_count", 1};
    Parser::TField<Vector2f> spacing_{"spacing", {0, 0}};
    Parser::TField<float>    padding_{"padding", 0};
    Parser::VField<int>      expanding_rows_{"expanding_rows"};
    Parser::VField<int>      expanding_columns_{"expanding_columns"};
    Parser::VField<int>      empty_cells_{"empty_cells"};
    ///@}

    std::vector<bool>    row_expands_;
    std::vector<bool>    col_expands_;
    std::vector<Pane *>  cell_panes_;  // Note: raw pointers.

    friend class Parser::Registry;
};

typedef std::shared_ptr<GridPane> GridPanePtr;
