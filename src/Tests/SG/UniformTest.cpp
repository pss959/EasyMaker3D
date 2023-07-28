#include "SceneTestBase.h"
#include "SG/Material.h"
#include "SG/Scene.h"
#include "SG/Texture.h"
#include "SG/Uniform.h"
#include "SG/UniformBlock.h"

class UniformTest : public SceneTestBase {};

TEST_F(UniformTest, DefaultUniformBlock) {
    auto block = CreateObject<SG::UniformBlock>();

    EXPECT_EQ("", block->GetPassName());
    EXPECT_NULL(block->GetMaterial());
    EXPECT_TRUE(block->GetTextures().empty());
    EXPECT_EQ("", block->GetSubImageName());
    EXPECT_TRUE(block->GetUniforms().empty());

    auto &reg = ion::gfx::ShaderInputRegistry::GetGlobalRegistry();
    auto ion_context = GetIonContext();
    auto ion_block = block->SetUpIon(ion_context, reg);
    EXPECT_NOT_NULL(ion_block.Get());

    block->SetPassName("Lighting");
    EXPECT_EQ("Lighting", block->GetPassName());
}

TEST_F(UniformTest, UniformBlock) {
    const std::string contents = R"(
  blocks: [
    UniformBlock {
      pass_name: "Lighting",
      material: Material "TestMaterial" {},
      textures: [
        Texture "Tex0" {
          uniform_name: "uTex",
          image: FileImage {
            path: "Shelf.jpg",
            sub_images: [SubImage "Subby" {}],
          }
        },
        CLONE "Tex0" "Tex1" {}
      ],
      sub_image_name: "Subby",
    }
  ],
)";

    auto scene = ReadScene(BuildSceneString(contents), false);
    EXPECT_NOT_NULL(scene->GetRootNode());
    EXPECT_EQ(1U, scene->GetRootNode()->GetUniformBlocks().size());
    auto block = scene->GetRootNode()->GetUniformBlocks()[0];
    EXPECT_NOT_NULL(block);
    EXPECT_EQ("Lighting", block->GetPassName());
    EXPECT_NOT_NULL(block->GetMaterial());
    EXPECT_EQ("TestMaterial", block->GetMaterial()->GetName());
    EXPECT_EQ(2U, block->GetTextures().size());
    EXPECT_EQ("Tex0", block->GetTextures()[0]->GetName());
    EXPECT_EQ("Tex1", block->GetTextures()[1]->GetName());
    EXPECT_EQ("Subby", block->GetSubImageName());
    EXPECT_TRUE(block->GetUniforms().empty());

    block->SetSubImageName("Blah");
    EXPECT_EQ("Blah", block->GetSubImageName());
}
