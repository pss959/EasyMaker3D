#include "Items/Controller.h"
#include "Tests/SceneTestBase.h"
#include "Tests/Testing.h"

class ControllerTest : public SceneTestBase {};

TEST_F(ControllerTest, Default) {
    auto cont = CreateObject<Controller>();
    EXPECT_EQ(Hand::kRight,         cont->GetHand());
    EXPECT_EQ(GripGuideType::kNone, cont->GetGripGuideType());
    EXPECT_EQ(Vector3f(-1, 0, 0),   cont->GetGuideDirection());
    EXPECT_FALSE(cont->IsInTouchMode());
    EXPECT_EQ(Vector3f(0, 0, 0),    cont->GetTouchOffset());
    EXPECT_EQ(0,                    cont->GetTouchRadius());
    ModelMesh mesh;
    ion::gfx::ImagePtr image;
    EXPECT_FALSE(cont->GetCustomModelData(mesh, image));
}
