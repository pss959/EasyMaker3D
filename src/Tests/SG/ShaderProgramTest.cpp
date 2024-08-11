//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "SG/Exception.h"
#include "SG/ShaderProgram.h"
#include "Tests/SceneTestBase.h"
#include "Tests/Testing2.h"
#include "Util/String.h"

/// \ingroup Tests
class ShaderProgramTest : public SceneTestBase {};

TEST_F(ShaderProgramTest, DefaultShaderProgram) {
    auto prog = CreateObject<SG::ShaderProgram>();

    EXPECT_NOT_NULL(prog);
    EXPECT_EQ("", prog->GetInheritFrom());
    EXPECT_NULL(prog->GetVertexSource());
    EXPECT_NULL(prog->GetGeometrySource());
    EXPECT_NULL(prog->GetFragmentSource());

    auto context = GetIonContext();

    // SetUpIon() should fail.
    TEST_THROW(prog->SetUpIon(context), SG::Exception, "No vertex program");

    // Create an instance with a valid vertex shader source.
    const Str input0 = R"(
ShaderProgram "SP" {
  vertex_source: ShaderSource { path: "Shadow_vp.glsl" },
}
)";
    prog = ParseTypedObject<SG::ShaderProgram>(input0);
    prog->SetUpIon(context);
    EXPECT_NOT_NULL(prog->GetIonShaderProgram().Get());

    // Create an instance with a bogus vertex shader source.
    const Str input1 = R"(
ShaderProgram "SP" {
  vertex_source: ShaderSource { path: "Bogus.glsl" }
}
)";
    prog = ParseTypedObject<SG::ShaderProgram>(input1);
    EXPECT_NOT_NULL(prog);
    TEST_THROW(prog->SetUpIon(context), SG::Exception, "Unable to read shader");

    // Create an instance with bad inheritance.
    const Str input2 = R"(
ShaderProgram "SP" {
  vertex_source: ShaderSource { path: "Shadow_vp.glsl" },
  inherit_from: "NoSuchProgram",
}
)";
    prog = ParseTypedObject<SG::ShaderProgram>(input2);
    EXPECT_NOT_NULL(prog);
    TEST_THROW(prog->SetUpIon(context), SG::Exception, "Unknown shader");
}
