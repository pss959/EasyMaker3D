#include "Panes/GridPane.h"
#include "Panes/SpacerPane.h"
#include "Tests/Panes/PaneTestBase.h"
#include "Tests/Testing.h"
#include "Util/String.h"

/// \ingroup Tests
class GridPaneTest : public PaneTestBase {
  protected:
    /// GridPane has no template, so this just creates an instance.
    GridPanePtr GetGridPane(const Str &contents = "");
};

GridPanePtr GridPaneTest::GetGridPane(const Str &contents) {
    const Str s = R"(
  children: [
    <"nodes/templates/Panes/AllPanes.emd">,
    GridPane "GridPane" { @CONTENTS@ }
  ]
)";
    return ReadRealNode<GridPane>(
        Util::ReplaceString(s, "@CONTENTS@", contents), "GridPane");
};

TEST_F(GridPaneTest, IsValid) {
    SetParseTypeName("GridPane");
    TestInvalid("row_count: 0", "Non-positive row or column count");
    TestInvalid("row_count: 1, column_count: -1",
                "Non-positive row or column count");
    TestInvalid("row_count: 2, column_count: 2, expanding_columns: [3]",
                "Index in expanding_columns out of range");
    TestInvalid("row_count: 2, column_count: 2, expanding_rows: [-1]",
                "Index in expanding_rows out of range");
    TestInvalid("", "not equal to cell count");
    TestInvalid("row_count: 2, column_count: 2,"
                " panes: [SpacerPane {}, SpacerPane {}, SpacerPane {}]",
                "not equal to cell count");
    TestValid("row_count: 2, column_count: 2,"
              " panes: [BoxPane {}, SpacerPane {}, BoxPane {}, SpacerPane {}]");
}

TEST_F(GridPaneTest, LayoutNonExpanding) {
    const Str contents = R"(
  TEMPLATES: [ SpacerPane "T_SP" { min_size: 10 10 } ],
  row_count:         2,
  column_count:      3,
  row_spacing:       10,
  column_spacing:    8,
  padding:           4,
  panes: [
    CLONE "T_SP" "Sp00" {},
    CLONE "T_SP" "Sp01" {},
    CLONE "T_SP" "Sp02" {},
    CLONE "T_SP" "Sp10" {},
    CLONE "T_SP" "Sp11" {},
    CLONE "T_SP" "Sp12" {},
  ]
)";
    auto grid = GetGridPane(contents);
    auto sp00 = SG::FindTypedNodeUnderNode<SpacerPane>(*grid, "Sp00");
    auto sp01 = SG::FindTypedNodeUnderNode<SpacerPane>(*grid, "Sp01");
    auto sp02 = SG::FindTypedNodeUnderNode<SpacerPane>(*grid, "Sp02");
    auto sp10 = SG::FindTypedNodeUnderNode<SpacerPane>(*grid, "Sp10");
    auto sp11 = SG::FindTypedNodeUnderNode<SpacerPane>(*grid, "Sp11");
    auto sp12 = SG::FindTypedNodeUnderNode<SpacerPane>(*grid, "Sp12");

    // Each SpacerPane min size is (10,10). Add cell sizes + spacing + padding.
    const Vector2f base_size((3 * 10) + (2 *  8) + (2 * 4),
                             (2 * 10) + (1 * 10) + (2 * 4));

    EXPECT_EQ(base_size, grid->GetBaseSize());
    EXPECT_EQ(Vector2f::Zero(), grid->GetLayoutSize());  // Not layed out yet.

    grid->SetLayoutSize(base_size);
    EXPECT_EQ(base_size, grid->GetBaseSize());
    EXPECT_EQ(base_size, grid->GetLayoutSize());
    EXPECT_EQ(Vector2f(10, 10), sp00->GetLayoutSize());
    EXPECT_EQ(Vector2f(10, 10), sp01->GetLayoutSize());
    EXPECT_EQ(Vector2f(10, 10), sp02->GetLayoutSize());
    EXPECT_EQ(Vector2f(10, 10), sp10->GetLayoutSize());
    EXPECT_EQ(Vector2f(10, 10), sp11->GetLayoutSize());
    EXPECT_EQ(Vector2f(10, 10), sp12->GetLayoutSize());

    // Increase the layout size of the grid. Nothing should change size.
    const Vector2f new_size = base_size + Vector2f(40, 50);
    grid->SetLayoutSize(new_size);
    EXPECT_EQ(base_size, grid->GetBaseSize());
    EXPECT_EQ(new_size,  grid->GetLayoutSize());
    EXPECT_EQ(Vector2f(10, 10), sp00->GetLayoutSize());
    EXPECT_EQ(Vector2f(10, 10), sp01->GetLayoutSize());
    EXPECT_EQ(Vector2f(10, 10), sp02->GetLayoutSize());
    EXPECT_EQ(Vector2f(10, 10), sp10->GetLayoutSize());
    EXPECT_EQ(Vector2f(10, 10), sp11->GetLayoutSize());
    EXPECT_EQ(Vector2f(10, 10), sp12->GetLayoutSize());
}

TEST_F(GridPaneTest, Layout) {
    const Str contents = R"(
  TEMPLATES: [
    SpacerPane "T_SP" {
      min_size: 10 10,
      resize_width: True,
      resize_height: True,
    }
  ],
  row_count:         2,
  column_count:      3,
  row_spacing:       10,
  column_spacing:    8,
  padding:           4,
  expanding_rows:    [1],
  expanding_columns: [1],
  panes: [
    CLONE "T_SP" "Sp00" {},
    CLONE "T_SP" "Sp01" {},
    CLONE "T_SP" "Sp02" {},
    CLONE "T_SP" "Sp10" {},
    CLONE "T_SP" "Sp11" {},
    CLONE "T_SP" "Sp12" {},
  ]
)";
    auto grid = GetGridPane(contents);
    auto sp00 = SG::FindTypedNodeUnderNode<SpacerPane>(*grid, "Sp00");
    auto sp01 = SG::FindTypedNodeUnderNode<SpacerPane>(*grid, "Sp01");
    auto sp02 = SG::FindTypedNodeUnderNode<SpacerPane>(*grid, "Sp02");
    auto sp10 = SG::FindTypedNodeUnderNode<SpacerPane>(*grid, "Sp10");
    auto sp11 = SG::FindTypedNodeUnderNode<SpacerPane>(*grid, "Sp11");
    auto sp12 = SG::FindTypedNodeUnderNode<SpacerPane>(*grid, "Sp12");

    // Each SpacerPane min size is (10,10). Add cell sizes + spacing + padding.
    const Vector2f base_size((3 * 10) + (2 *  8) + (2 * 4),
                             (2 * 10) + (1 * 10) + (2 * 4));

    EXPECT_EQ(base_size, grid->GetBaseSize());
    EXPECT_EQ(Vector2f::Zero(), grid->GetLayoutSize());  // Not layed out yet.

    grid->SetLayoutSize(base_size);
    EXPECT_EQ(base_size, grid->GetBaseSize());
    EXPECT_EQ(base_size, grid->GetLayoutSize());
    EXPECT_EQ(Vector2f(10, 10), sp00->GetLayoutSize());
    EXPECT_EQ(Vector2f(10, 10), sp01->GetLayoutSize());
    EXPECT_EQ(Vector2f(10, 10), sp02->GetLayoutSize());
    EXPECT_EQ(Vector2f(10, 10), sp10->GetLayoutSize());
    EXPECT_EQ(Vector2f(10, 10), sp11->GetLayoutSize());
    EXPECT_EQ(Vector2f(10, 10), sp12->GetLayoutSize());

    // Increase the layout size of the grid. Only the second row and second
    // column should change size.
    const Vector2f new_size = base_size + Vector2f(40, 50);
    grid->SetLayoutSize(new_size);
    EXPECT_EQ(base_size, grid->GetBaseSize());
    EXPECT_EQ(new_size,  grid->GetLayoutSize());
    EXPECT_EQ(Vector2f(10, 10), sp00->GetLayoutSize());
    EXPECT_EQ(Vector2f(50, 10), sp01->GetLayoutSize());
    EXPECT_EQ(Vector2f(10, 10), sp02->GetLayoutSize());
    EXPECT_EQ(Vector2f(10, 60), sp10->GetLayoutSize());
    EXPECT_EQ(Vector2f(50, 60), sp11->GetLayoutSize());
    EXPECT_EQ(Vector2f(10, 60), sp12->GetLayoutSize());
}
