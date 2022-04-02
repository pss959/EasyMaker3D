#include "SG/IonContext.h"

#include "SG/Tracker.h"
#include "Util/Assert.h"
#include "Util/General.h"

namespace SG {

void IonContext::SetTracker(const TrackerPtr &tracker) {
    ASSERT(tracker);
    tracker_ = tracker;
}

Tracker & IonContext::GetTracker() const {
    ASSERT(tracker_);
    return *tracker_;
}

void IonContext::SetShaderManager(
    const ion::gfxutils::ShaderManagerPtr &shader_manager) {
    ASSERT(shader_manager);
    shader_manager_ = shader_manager;
}

const ion::gfxutils::ShaderManagerPtr & IonContext::GetShaderManager() {
    ASSERT(shader_manager_);
    return shader_manager_;
}

void IonContext::SetFontManager(const ion::text::FontManagerPtr &font_manager) {
    ASSERT(font_manager);
    font_manager_ = font_manager;
}

const ion::text::FontManagerPtr & IonContext::GetFontManager() {
    ASSERT(font_manager_);
    return font_manager_;
}

int IonContext::GetPassIndex(const std::string &name) const {
    for (size_t i = 0; i < pass_names_.size(); ++i)
        if (pass_names_[i] == name)
            return i;
    return -1;
}

void IonContext::AddShaderProgram(const std::string &name,
                                  const std::string &pass_name,
                                  const ion::gfx::ShaderProgramPtr &program) {
    ASSERT(! name.empty());
    ASSERT(program);
    ASSERT(! Util::MapContains(program_map_, name));
    int index = GetPassIndex(pass_name);
    ASSERTM(index >= 0, "Cannot get index for pass " + pass_name);
    ProgramInfo info;
    info.pass_index    = index;
    info.program       = program;
    program_map_[name] = info;
}

const IonContext::ProgramInfo & IonContext::GetShaderProgramInfo(
    const std::string &name) const {
    auto it = program_map_.find(name);
    ASSERT(it != program_map_.end());
    return it->second;
}

ion::gfx::ShaderInputRegistryPtr IonContext::GetRegistryForPass(
    const std::string &pass_name,
    const std::vector<ion::gfx::ShaderProgramPtr> &programs) const {
    if (pass_name.empty())
        return ion::gfx::ShaderInputRegistry::GetGlobalRegistry();
    const int index = GetPassIndex(pass_name);
    ASSERTM(index >= 0 && static_cast<size_t>(index) < programs.size(),
            pass_name);
    return programs[index]->GetRegistry();
}

void IonContext::Reset() {
    pass_names_.clear();
    program_map_.clear();
}

}  // namespace SG
