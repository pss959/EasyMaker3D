#include <algorithm>

#include "SG/Scene.h"
#include "SG/Node.h"
#include "SG/Texture.h"
#include "SG/Uniform.h"
#include "SG/UniformBlock.h"
#include "Tests/SceneTestBase.h"
#include "Util/Assert.h"
#include "Util/General.h"

class SetUpIonTest : public SceneTestBase {};

TEST_F(SetUpIonTest, EmptyScene) {
    SG::ScenePtr scene = ReadScene("Scene \"MyScene\" {}\n", true);
    EXPECT_NOT_NULL(scene.get());
}

TEST_F(SetUpIonTest, Uniforms) {
    const std::string input = ReadDataFile("FullScene");
    SG::ScenePtr scene = ReadScene(input, true);
    EXPECT_NOT_NULL(scene.get());

    auto root = scene->GetRootNode();
    EXPECT_NOT_NULL(root);

    EXPECT_EQ(3U, root->GetUniformBlocks().size());
    auto block0 = root->GetUniformBlocks()[0];
    auto block1 = root->GetUniformBlocks()[1];
    auto block2 = root->GetUniformBlocks()[2];
    EXPECT_NOT_NULL(block0);
    EXPECT_NOT_NULL(block1);
    EXPECT_NOT_NULL(block2);
    EXPECT_NOT_NULL(block0->GetIonUniformBlock().Get());
    EXPECT_NOT_NULL(block1->GetIonUniformBlock().Get());
    EXPECT_NOT_NULL(block2->GetIonUniformBlock().Get());

    // There should be 4 Uniforms set up for the Material.
    EXPECT_EQ(4U, block0->GetUniforms().size());
    const std::vector<std::string> material_uniform_names{
        "uBaseColor", "uEmissiveColor", "uSmoothness", "uMetalness" };
    for (const auto &u: block0->GetUniforms())
        EXPECT_TRUE(Util::Contains(material_uniform_names, u->GetName()));

    // Test adding new Uniforms to set below.
    block0->CreateAndAddUniform("UI1", "int_val");
    block0->CreateAndAddUniform("UF1", "float_val");
    block0->CreateAndAddUniform("UF3", "vec3f_val");
    block0->CreateAndAddUniform("UF4", "vec4f_val");
    block0->CreateAndAddUniform("UM4", "mat4_val");

    // Test setting each supported type of Uniform.
    block0->SetIntUniformValue("UI1", 14);
    block0->SetFloatUniformValue("UF1", 2.5f);
    block0->SetVector3fUniformValue("UF3", Vector3f(1, 2, 3));
    block0->SetVector4fUniformValue("UF4", Vector4f(1, 2, 3, 4));
    block0->SetMatrix4fUniformValue("UM4", Matrix4f::Identity());

    // Test bad uniform name.
    TEST_THROW(block0->SetIntUniformValue("NoSuch", 12),
               AssertException, "No such uniform");

    // Test changing the sub-image name in UniformBlock.
    EXPECT_EQ("Sub1", block0->GetSubImageName());
    block0->SetSubImageName("Sub2");
    EXPECT_EQ("Sub2", block0->GetSubImageName());

    // The second UniformBlock has one uniform of each type. Find and set each
    // one.
    const auto &uniforms = block1->GetUniforms();
    const auto find_uniform = [&](const std::string &name){
        const auto match = [&](const auto &u){ return u->GetName() == name; };
        const auto it = std::find_if(uniforms.begin(), uniforms.end(), match);
        ASSERT(it != uniforms.end());
        return *it;
    };
    SG::UniformPtr u;

#define TEST_U_(name, type, func, val0, val1)                           \
    u = find_uniform(name);                                             \
    EXPECT_EQ(type val0 , u->func());                                   \
    u->SetValue<type>(type val1);                                       \
    EXPECT_EQ(type val1, u->func())

    TEST_U_("UI1", int,       GetInt,       (23),         (13));
    TEST_U_("UI2", Vector2i,  GetVector2i,  (1, 2),       (3, 4));
    TEST_U_("UI3", Vector3i,  GetVector3i,  (1, 2, 3),    (3, 4, 5));
    TEST_U_("UI4", Vector4i,  GetVector4i,  (1, 2, 3, 4), (3, 4, 5, 6));
    TEST_U_("UU1", uint,      GetUInt,      (23U),        (13U));
    TEST_U_("UU2", Vector2ui, GetVector2ui, (1, 2),       (3, 4));
    TEST_U_("UU3", Vector3ui, GetVector3ui, (1, 2, 3),    (3, 4, 5));
    TEST_U_("UU4", Vector4ui, GetVector4ui, (1, 2, 3, 4), (3, 4, 5, 6));
    TEST_U_("UF1", float,     GetFloat,     (23),         (13));
    TEST_U_("UF2", Vector2f,  GetVector2f,  (1, 2),       (3, 4));
    TEST_U_("UF3", Vector3f,  GetVector3f,  (1, 2, 3),    (3, 4, 5));
    TEST_U_("UF4", Vector4f,  GetVector4f,  (1, 2, 3, 4), (3, 4, 5, 6));
    TEST_U_("UM2", Matrix2f,  GetMatrix2f,  (1, 2, 3, 4), (3, 4, 5, 6));
    TEST_U_("UM3", Matrix3f,  GetMatrix3f,
            (1, 2, 3, 4, 5, 6, 7,  8,  9),
            (3, 4, 5, 6, 7, 8, 9, 10, 11));
    TEST_U_("UM4", Matrix4f,  GetMatrix4f,
            (1, 2, 3, 4, 5, 6, 7,  8,  9, 10, 11, 12, 13, 14, 15, 16),
            (3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18));

#undef TEST_U_

    // The third UniformBlock has one array uniform of each type. Find and set
    // each one.
}
