#include "SG/StateTable.h"
#include "Tests/SceneTestBase.h"
#include "Tests/Testing.h"

/// \ingroup Tests
class StateTableTest : public SceneTestBase {};

TEST_F(StateTableTest, DefaultStateTable) {
    auto st = CreateObject<SG::StateTable>();

    EXPECT_NOT_NULL(st);
    EXPECT_EQ(Vector4f::Zero(), st->GetClearColor());
    EXPECT_EQ(0,                st->GetClearDepth());
    EXPECT_EQ(0,                st->GetClearStencil());
    EXPECT_EQ(1,                st->GetLineWidth());
    EXPECT_FALSE(st->IsBlendEnabled());
    EXPECT_FALSE(st->IsDepthTestEnabled());
    EXPECT_FALSE(st->IsCullFaceEnabled());
    EXPECT_EQ(SG::StateTable::CullFaceMode::kCullBack, st->GetCullFaceMode());
    EXPECT_FALSE(st->IsCreateStencil());
    EXPECT_FALSE(st->IsUseStencil());
    EXPECT_FALSE(st->IsResetStencil());

    SG::StateTable::BlendEquation rgb, alpha;
    st->GetBlendEquations(rgb, alpha);
    EXPECT_EQ(SG::StateTable::BlendEquation::kAdd, rgb);
    EXPECT_EQ(SG::StateTable::BlendEquation::kAdd, alpha);

    SG::StateTable::BlendFunctionFactor rgbs, rgbd, as, ad;
    st->GetBlendFunctions(rgbs, rgbd, as, ad);
    EXPECT_EQ(SG::StateTable::BlendFunctionFactor::kOne,  rgbs);
    EXPECT_EQ(SG::StateTable::BlendFunctionFactor::kZero, rgbd);
    EXPECT_EQ(SG::StateTable::BlendFunctionFactor::kOne,  as);
    EXPECT_EQ(SG::StateTable::BlendFunctionFactor::kZero, ad);

    // SetUpIon() should work.
    auto ion_st = st->SetUpIon();
    EXPECT_NOT_NULL(ion_st.Get());
}

TEST_F(StateTableTest, LineWidth) {
    auto st = CreateObject<SG::StateTable>();
    EXPECT_EQ(1, st->GetLineWidth());

    // Call SetUpIon() so the Ion StateTable is updated as well.
    auto ion_st = st->SetUpIon();
    EXPECT_NOT_NULL(ion_st.Get());

    st->SetLineWidth(.6f);
    EXPECT_EQ(.6f, st->GetLineWidth());
}
