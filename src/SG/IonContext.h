#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include <ion/gfx/shaderinputregistry.h>
#include <ion/gfx/shaderprogram.h>
#include <ion/gfxutils/shadermanager.h>
#include <ion/text/fontmanager.h>

#include "Assert.h"
#include "SG/Tracker.h"
#include "SG/Typedefs.h"
#include "Util/General.h"

namespace SG {

/// The IonContext class stores information that is used to help set up Ion
/// data in SG graphs. It has information about the render passes and shaders
/// set up for a Scene and a Tracker used to find Ion resources. There is a
/// single instance maintained by the Scene which is supplied to every Node
/// when it is set up.
class IonContext {
  public:
    /// This struct saves information relating to a shader program. Instances
    /// of this are stored in a map keyed by shader program name.
    struct ProgramInfo {
        /// Index into the pass names of the RenderPass this shader is for.
        size_t pass_index;
        /// The Ion ShaderProgram.
        ion::gfx::ShaderProgramPtr program;
    };

    /// Sets the Tracker.
    void SetTracker(const TrackerPtr &tracker) {
        ASSERT(tracker);
        tracker_ = tracker;
    }

    /// Returns the Tracker.
    Tracker & GetTracker() const {
        ASSERT(tracker_);
        return *tracker_;
    }

    /// Sets the ShaderManager.
    void SetShaderManager(
        const ion::gfxutils::ShaderManagerPtr &shader_manager) {
        ASSERT(shader_manager);
        shader_manager_ = shader_manager;
    }

    /// Returns the ShaderManager.
    const ion::gfxutils::ShaderManagerPtr & GetShaderManager() {
        ASSERT(shader_manager_);
        return shader_manager_;
    }

    /// Sets the FontManager.
    void SetFontManager(
        const ion::text::FontManagerPtr &font_manager) {
        ASSERT(font_manager);
        font_manager_ = font_manager;
    }

    /// Returns the FontManager.
    const ion::text::FontManagerPtr & GetFontManager() {
        ASSERT(font_manager_);
        return font_manager_;
    }

    /// Adds the name of a render pass.
    void AddPassName(const std::string &pass_name) {
        pass_names_.push_back(pass_name);
    }

    /// Returns the number of render passes.
    size_t GetPassCount() const { return pass_names_.size(); }

    /// Returns the index for the pass with the given name. Returns -1 if the
    /// pass name is not valid.
    int GetPassIndex(const std::string &name) const {
        for (size_t i = 0; i < pass_names_.size(); ++i)
            if (pass_names_[i] == name)
                return i;
        return -1;
    }

    /// Adds an Ion ShaderProgram associated with the given name and pass name.
    void AddShaderProgram(const std::string &name, const std::string &pass_name,
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

    /// Returns a reference to a ProgramInfo for the shader with the given
    /// name. It will assert if there is no such shader.
    const ProgramInfo & GetShaderProgramInfo(const std::string &name) const {
        auto it = program_map_.find(name);
        ASSERT(it != program_map_.end());
        return it->second;
    }

    /// Convenience that returns the Ion ShaderInputRegistry to use for the
    /// render pass with the given name. If the name is empty, this will be the
    /// global registry. Otherwise, it uses the ShaderProgram corresponding to
    /// the render pass from the given vector. This asserts if the pass name
    /// does not match an existing pass.
    ion::gfx::ShaderInputRegistryPtr GetRegistryForPass(
        const std::string &pass_name,
        const std::vector<ion::gfx::ShaderProgramPtr> &programs) const {
        if (pass_name.empty())
            return ion::gfx::ShaderInputRegistry::GetGlobalRegistry();
        const int index = GetPassIndex(pass_name);
        ASSERT(index >= 0 && static_cast<size_t>(index) < programs.size());
        return programs[index]->GetRegistry();
    }

    /// Resets the IonContext. This clears out the pass and program information
    /// but leaves the managers and Tracker alone.
    void Reset() {
        pass_names_.clear();
        program_map_.clear();
    }

  private:
    ion::gfxutils::ShaderManagerPtr shader_manager_;
    ion::text::FontManagerPtr       font_manager_;
    TrackerPtr                      tracker_;

    /// Names of render passes.
    std::vector<std::string> pass_names_;

    /// Maps every known shader name to a corresponding ProgramInfo struct.
    std::unordered_map<std::string, ProgramInfo> program_map_;
};

typedef std::shared_ptr<IonContext> IonContextPtr;

}  // namespace SG
