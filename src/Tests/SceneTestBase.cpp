//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Tests/SceneTestBase.h"

#include "SG/ColorMap.h"
#include "SG/FileMap.h"
#include "SG/Reader.h"
#include "Tests/TempFile.h"
#include "Tests/Testing.h"
#include "Util/Assert.h"
#include "Util/String.h"
#include "Util/Tuning.h"

const float SceneTestBase::MS = TK::kInitialModelScale;

SceneTestBase::~SceneTestBase() {
    scene_.reset();
    SG::ColorMap::Reset();
    ResetContext();
}

SG::ScenePtr SceneTestBase::ReadScene(const Str &input, bool set_up_ion) {
    if (! ion_context_)
        InitIonContext_();

    TempFile file(input);
    SG::Reader reader;
    scene_ = reader.ReadScene(file.GetPath(), ion_context_->GetFileMap());
    if (scene_ && set_up_ion)
        scene_->SetUpIon(ion_context_);
    return scene_;
}

SG::ScenePtr SceneTestBase::BuildAndReadScene(const Str &contents,
                                              bool set_up_ion) {
    const Str header = R"(
Scene {
  render_passes: [
    LightingPass "Lighting" {
      shader_programs: [<"programs/BaseColor.emd">]
    }
  ],
  root_node: Node "Root" {
)";
    return ReadScene(header + contents + "}}", set_up_ion);
}

SG::ScenePtr SceneTestBase::ReadRealScene(const Str &contents,
                                          bool set_up_ion) {
    const Str input = Util::ReplaceString(
        ReadDataFile("RealScene.emd"), "#<CONTENTS>", contents);
    return ReadScene(input, set_up_ion);
}

SG::IonContextPtr SceneTestBase::GetIonContext() {
    if (! ion_context_)
        InitIonContext_();
    return ion_context_;
}

void SceneTestBase::SetUpIonForNode(SG::Node &node) {
    node.SetUpIon(GetIonContext(), std::vector<ion::gfx::ShaderProgramPtr>());
}

void SceneTestBase::InitIonContext_() {
    ASSERT(! ion_context_);
    ion_context_.reset(new SG::IonContext);
    ion_context_->SetFileMap(SG::FileMapPtr(new SG::FileMap()));
    ion_context_->SetShaderManager(
        ion::gfxutils::ShaderManagerPtr(new ion::gfxutils::ShaderManager));
    ion_context_->SetFontManager(
        ion::text::FontManagerPtr(new ion::text::FontManager));
}
