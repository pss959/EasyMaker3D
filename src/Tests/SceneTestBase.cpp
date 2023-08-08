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

SG::ScenePtr SceneTestBase::ReadScene(const std::string &input,
                                      bool set_up_ion) {
    if (! ion_context_)
        InitIonContext_();

    TempFile file(input);
    SG::Reader reader;
    scene_ = reader.ReadScene(file.GetPath(), ion_context_->GetFileMap());
    if (scene_ && set_up_ion)
        scene_->SetUpIon(ion_context_);
    return scene_;
}

SG::ScenePtr SceneTestBase::BuildAndReadScene(const std::string &contents,
                                              bool set_up_ion) {
    const std::string header = R"(
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

SG::ScenePtr SceneTestBase::ReadSceneWithIncludes(
    const std::vector<std::string> &inc_file_names) {

    std::string contents = "children: [\n";
    for (const auto &inc: inc_file_names) {
        contents += "  <\"" + inc + "\">,\n";
    }
    contents += "],\n";

    const std::string input = Util::ReplaceString(
        ReadDataFile("RealScene.emd"), "#<CONTENTS>", contents);

    return ReadScene(input, true);
}

SG::IonContextPtr SceneTestBase::GetIonContext() {
    if (! ion_context_)
        InitIonContext_();
    return ion_context_;
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

void SceneTestBase::AddNodeToRealScene_(const SG::NodePtr &node) {
    EXPECT_NOT_NULL(node);

    ReadScene(ReadDataFile("RealScene.emd"), true);
    EXPECT_NOT_NULL(scene_);
    EXPECT_NOT_NULL(scene_->GetRootNode());

    /// This will set up Ion in the Node.
    scene_->GetRootNode()->AddChild(node);
    EXPECT_NOT_NULL(node->GetIonNode().Get());
}
