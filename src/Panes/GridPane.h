#pragma once

#include <vector>

#include "Base/Memory.h"
#include "Panes/ContainerPane.h"

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
    virtual void CreationDone() override;

    virtual Vector2f ComputeBaseSize() const override;
    virtual void     LayOutSubPanes() override;

  private:
    /// An instance of this struct is stored for rows and columns.
    struct DimData_ {
        size_t            count;         ///< Number of cells in this dimension.
        float             spacing;       ///< Spacing between cells.
        std::vector<bool> expands;       ///< Whether each cell expands.
        size_t            expand_count;  ///< Number of expanding cells.
    };

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

    DimData_ dim_data_[2];  ///< Column, row data.

    bool CheckDim_(int dim, Str &details);
    bool CheckPanes_(Str &details);
    void SetUpDim_(int dim);

    /// Returns a vector of sizes to use for all rows or columns to fit the
    /// given size. Sets min_size to the minimum size in the dimension.
    std::vector<float> ComputeSizes_(int dim, float size,
                                     float &min_size) const;

    /// Returns a vector of the base size of all cells in the given dimension
    /// and sets total to the total base size (including padding and spacing)
    /// in that dimension.
    std::vector<float> ComputeBaseSizes_(int dim, float &total) const;

    size_t GetCellIndex_(size_t row, size_t col) const {
        return row * dim_data_[0].count + col;
    }

    friend class Parser::Registry;
};
