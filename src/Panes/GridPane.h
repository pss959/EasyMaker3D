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
    /// An instance of this struct is stored for rows and columns.
    struct DimData_ {
        size_t            count;         ///< Number of cells in this dimension.
        std::vector<bool> expands;       ///< Whether each cell expands.
        size_t            expand_count;  ///< Number of expanding cells.
    };

    /// \name Parsed Fields
    ///@{
    Parser::TField<int>      row_count_{"row_count", 1};
    Parser::TField<int>      column_count_{"column_count", 1};
    Parser::TField<float>    row_spacing_{"row_spacing", 0};
    Parser::TField<float>    column_spacing_{"column_spacing", 0};
    Parser::TField<float>    padding_{"padding", 0};
    Parser::VField<int>      expanding_rows_{"expanding_rows"};
    Parser::VField<int>      expanding_columns_{"expanding_columns"};
    Parser::VField<int>      empty_cells_{"empty_cells"};
    ///@}

    DimData_            dim_data_[2];  ///< Column, row data.
    std::vector<Pane *> cell_panes_;   ///< Raw pointers to contained panes.

    bool SetUpDim_(DimData_ &data, size_t count,
                   const Parser::VField<int> &field, std::string &details);
    bool StorePanes_(std::string &details);
    void LayOutPanes_(const Vector2f &size);

    /// Returns a vector of sizes to use for all rows or columns to fit the
    /// given size.
    std::vector<float> ComputeSizes_(int dim, float size) const;

    /// Returns a vector of the minimum size of all cells in the given
    /// dimension.
    std::vector<float> ComputeMinSizes_(int dim) const;

    size_t GetCellIndex_(size_t row, size_t col) const {
        return row * dim_data_[0].count + col;
    }

    friend class Parser::Registry;
};

typedef std::shared_ptr<GridPane> GridPanePtr;
