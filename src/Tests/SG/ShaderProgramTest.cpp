#include "SceneTestBase.h"
#include "SG/Exception.h"
#include "SG/FileMap.h"
#include "SG/ShaderProgram.h"
#include "Util/String.h"

class ShaderProgramTest : public SceneTestBase {};

TEST_F(ShaderProgramTest, DefaultShaderProgram) {
    auto prog = CreateObject<SG::ShaderProgram>();

    EXPECT_NOT_NULL(prog);
    EXPECT_EQ("", prog->GetInheritFrom());
    EXPECT_NULL(prog->GetVertexSource());
    EXPECT_NULL(prog->GetGeometrySource());
    EXPECT_NULL(prog->GetFragmentSource());

    // SetUpIon() should fail.
    SG::FileMap fm;
    ion::gfxutils::ShaderManagerPtr sm(new ion::gfxutils::ShaderManager);
    TEST_THROW(prog->SetUpIon(fm, *sm), SG::Exception, "No vertex program");

    // Create an instance with a valid vertex shader source.
    const std::string input0 = R"(
ShaderProgram "SP" {
  vertex_source: ShaderSource { path: "Shadow_vp.glsl" },
}
)";
    prog = ReadTypedItem<SG::ShaderProgram>(input0);
    prog->SetUpIon(fm, *sm);
    EXPECT_NOT_NULL(prog->GetIonShaderProgram().Get());

    // Create an instance with a bogus vertex shader source.
    const std::string input1 = R"(
ShaderProgram "SP" {
  vertex_source: ShaderSource { path: "Bogus.glsl" }
}
)";
    prog = ReadTypedItem<SG::ShaderProgram>(input1);
    EXPECT_NOT_NULL(prog);
    TEST_THROW(prog->SetUpIon(fm, *sm), SG::Exception, "Unable to read shader");

    // Create an instance with bad inheritance.
    const std::string input2 = R"(
ShaderProgram "SP" {
  vertex_source: ShaderSource { path: "Shadow_vp.glsl" },
  inherit_from: "NoSuchProgram",
}
)";
    prog = ReadTypedItem<SG::ShaderProgram>(input2);
    EXPECT_NOT_NULL(prog);
    TEST_THROW(prog->SetUpIon(fm, *sm), SG::Exception, "Unknown shader");
}
