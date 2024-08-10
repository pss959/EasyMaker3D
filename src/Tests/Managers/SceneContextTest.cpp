#include "Managers/SceneContext.h"
#include "Tests/SceneTestBase.h"
#include "Tests/Testing.h"
#include "Util/Tuning.h"

/// \ingroup Tests
class SceneContextTest : public SceneTestBase {};

TEST_F(SceneContextTest, FillFromScene) {
    // Have to read the actual workshop scene for this to work successfully.
    const auto contents =
        ReadResourceFile("scenes/workshop" + TK::kDataFileExtension);
    auto scene = ReadScene(contents);

    // Pass false for is_full_scene. Only the Gantry and Cameras should be set
    // up.
    {
        SceneContext sc;
        sc.FillFromScene(scene, false);

        EXPECT_EQ(scene, sc.scene);
        EXPECT_NULL(sc.frustum);     // Not set up by SceneContext.
        EXPECT_NOT_NULL(sc.gantry);
        EXPECT_NOT_NULL(sc.vr_camera);
        EXPECT_NOT_NULL(sc.window_camera);
        EXPECT_NULL(sc.shadow_pass);
        EXPECT_NULL(sc.app_board);
        EXPECT_NULL(sc.key_board);
        EXPECT_NULL(sc.tool_board);
        EXPECT_NULL(sc.wall_board);
        EXPECT_NULL(sc.tree_panel);
        EXPECT_NULL(sc.keyboard_panel);
        EXPECT_NULL(sc.precision_control);
        EXPECT_NULL(sc.height_pole);
        EXPECT_NULL(sc.height_slider);
        EXPECT_NULL(sc.left_controller);
        EXPECT_NULL(sc.right_controller);
        EXPECT_NULL(sc.inspector);
        EXPECT_NULL(sc.left_radial_menu);
        EXPECT_NULL(sc.right_radial_menu);
        EXPECT_NULL(sc.room);
        EXPECT_NULL(sc.work_hider);
        EXPECT_NULL(sc.root_model);
        EXPECT_NULL(sc.stage);
        EXPECT_NULL(sc.build_volume);
        EXPECT_TRUE(sc.path_to_stage.empty());
    }

    // Pass true for is_full_scene.
    {
        SceneContext sc;
        sc.FillFromScene(scene, true);

        EXPECT_EQ(scene, sc.scene);
        EXPECT_NULL(sc.frustum);     // Not set up by SceneContext.
        EXPECT_NOT_NULL(sc.gantry);
        EXPECT_NOT_NULL(sc.vr_camera);
        EXPECT_NOT_NULL(sc.window_camera);
        EXPECT_NOT_NULL(sc.shadow_pass);
        EXPECT_NOT_NULL(sc.app_board);
        EXPECT_NOT_NULL(sc.key_board);
        EXPECT_NOT_NULL(sc.tool_board);
        EXPECT_NOT_NULL(sc.wall_board);
        EXPECT_NOT_NULL(sc.tree_panel);
        EXPECT_NOT_NULL(sc.keyboard_panel);
        EXPECT_NOT_NULL(sc.precision_control);
        EXPECT_NOT_NULL(sc.height_pole);
        EXPECT_NOT_NULL(sc.height_slider);
        EXPECT_NOT_NULL(sc.left_controller);
        EXPECT_NOT_NULL(sc.right_controller);
        EXPECT_NOT_NULL(sc.inspector);
        EXPECT_NOT_NULL(sc.left_radial_menu);
        EXPECT_NOT_NULL(sc.right_radial_menu);
        EXPECT_NOT_NULL(sc.room);
        EXPECT_NOT_NULL(sc.work_hider);
        EXPECT_NOT_NULL(sc.root_model);
        EXPECT_NOT_NULL(sc.stage);
        EXPECT_NOT_NULL(sc.build_volume);
        EXPECT_FALSE(sc.path_to_stage.empty());
    }
}
