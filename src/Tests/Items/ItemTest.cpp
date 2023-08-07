#include "Items/AppInfo.h"
#include "Items/Border.h"
#include "Items/BuildVolume.h"
#include "Parser/Exception.h"
#include "Tests/SceneTestBase.h"
#include "Tests/Testing.h"
#include "Util/Tuning.h"

// Most Items are simple enough that they can all be tested in one file.

class ItemTest : public SceneTestBase {};

TEST_F(ItemTest, AppInfo) {
    auto info = CreateObject<AppInfo>();
    EXPECT_TRUE(info->GetVersion().empty());
    EXPECT_NULL(info->GetSessionState());

    info = AppInfo::CreateDefault();
    EXPECT_EQ(TK::kVersionString, info->GetVersion());
    EXPECT_NOT_NULL(info->GetSessionState());

    info = ReadTypedItem<AppInfo>(
        "AppInfo { version: \"1.2.3\", session_state: SessionState {} }");
    EXPECT_EQ("1.2.3", info->GetVersion());
    EXPECT_NOT_NULL(info->GetSessionState());

    TEST_THROW(ReadTypedItem<AppInfo>("AppInfo {}"),
               Parser::Exception, "Missing version field data");
    TEST_THROW(ReadTypedItem<AppInfo>("AppInfo { version: \"Something\" }"),
               Parser::Exception, "Missing session_state data");
}

TEST_F(ItemTest, Border) {
    auto border = CreateObject<Border>();
    EXPECT_EQ(Color::Black(), border->GetColor());
    EXPECT_EQ(1,              border->GetWidth());

    border->SetColor(Color(1, 1, 0));
    border->SetWidth(2.5f);
    EXPECT_EQ(Color(1, 1, 0), border->GetColor());
    EXPECT_EQ(2.5f,           border->GetWidth());

    // Size is 0 until SetUpIon() is called.
    auto size = border->GetScaledBounds().GetSize();
    EXPECT_EQ(0, size[0]);
    EXPECT_EQ(0, size[1]);
    border->SetSize(Vector2f(20, 10));
    EXPECT_EQ(0, size[0]);
    EXPECT_EQ(0, size[1]);

    // This calls SetUpIon(). Note that the Border needs to have a
    // MutableTriMeshShape for this to work.
    border = ReadTypedNode<Border>(
        "Border { shapes: [ MutableTriMeshShape {} ] }", true);
    border->SetSize(Vector2f(20, 10));
    size = border->GetScaledBounds().GetSize();
    // There should be a real mesh installed with size 1x1. (The actual size is
    // created with Pane scaling; the passed-in size is to handle width
    // correctly.)
    EXPECT_EQ(1, size[0]);
    EXPECT_EQ(1, size[1]);

    // Width = 0 causes the border to be disabled.
    EXPECT_TRUE(border->IsEnabled());
    border->SetWidth(0);
    EXPECT_FALSE(border->IsEnabled());
}

TEST_F(ItemTest, BuildVolume) {
    auto bv = CreateObject<BuildVolume>();
    EXPECT_EQ(Vector3f(1, 1, 1), bv->GetSize());
    EXPECT_TRUE(bv->IsActive());

    bv->SetSize(Vector3f(20, 30, 40));
    EXPECT_EQ(Vector3f(20, 30, 40), bv->GetSize());
    EXPECT_TRUE(bv->IsActive());

    bv->Activate(false);
    EXPECT_EQ(Vector3f(20, 30, 40), bv->GetSize());
    EXPECT_FALSE(bv->IsActive());

    bv->Activate(true);
    EXPECT_TRUE(bv->IsActive());
    EXPECT_EQ(Vector3f(20, 30, 40), bv->GetScale());
    EXPECT_EQ(Vector3f(0,  15,  0), bv->GetTranslation());
}
