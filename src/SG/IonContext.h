//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

#include <unordered_map>
#include <vector>

#include <ion/gfx/shaderinputregistry.h>
#include <ion/gfx/shaderprogram.h>
#include <ion/gfxutils/shadermanager.h>
#include <ion/text/fontmanager.h>

#include "Util/Memory.h"

namespace SG {

DECL_SHARED_PTR(Context);
DECL_SHARED_PTR(FileMap);

/// The IonContext class stores information that is used to help set up Ion
/// data in SG graphs. It has information about the render passes and shaders
/// set up for a Scene and a FileMap used to find Ion resources. There is a
/// single instance maintained by the Scene which is supplied to every Node
/// when it is set up.
///
/// \ingroup SG
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

    /// Sets the FileMap.
    void SetFileMap(const FileMapPtr &file_map);

    /// Returns the FileMap.
    FileMap & GetFileMap() const;

    /// Sets the ShaderManager.
    void SetShaderManager(
        const ion::gfxutils::ShaderManagerPtr &shader_manager);

    /// Returns the ShaderManager.
    const ion::gfxutils::ShaderManagerPtr & GetShaderManager();

    /// Sets the FontManager.
    void SetFontManager(const ion::text::FontManagerPtr &font_manager);

    /// Returns the FontManager.
    const ion::text::FontManagerPtr & GetFontManager();

    /// Adds the name of a render pass.
    void AddPassName(const Str &pass_name) {
        pass_names_.push_back(pass_name);
    }

    /// Returns the number of render passes.
    size_t GetPassCount() const { return pass_names_.size(); }

    /// Returns the index for the pass with the given name. Returns -1 if the
    /// pass name is not valid.
    int GetPassIndex(const Str &name) const;

    /// Adds an Ion ShaderProgram associated with the given name and pass name.
    void AddShaderProgram(const Str &name, const Str &pass_name,
                          const ion::gfx::ShaderProgramPtr &program);

    /// Returns a reference to a ProgramInfo for the shader with the given
    /// name. It will assert if there is no such shader.
    const ProgramInfo & GetShaderProgramInfo(const Str &name) const;

    /// Convenience that returns the Ion ShaderInputRegistry to use for the
    /// render pass with the given name. If the name is empty, this will be the
    /// global registry. Otherwise, it uses the ShaderProgram corresponding to
    /// the render pass from the given vector. This asserts if the pass name
    /// does not match an existing pass.
    ion::gfx::ShaderInputRegistryPtr GetRegistryForPass(
        const Str &pass_name,
        const std::vector<ion::gfx::ShaderProgramPtr> &programs) const;

    /// Resets the IonContext. This clears out the pass and program information
    /// but leaves the managers and FileMap alone.
    void Reset();

    /// Increments or decrements the current indentation level for logging
    /// calls to SetUpIon(). Purely for debugging help.
    void ChangeLevel(int amount) { level_ += amount; }

    /// Returns a string with spaces for the current indentation level for
    /// logging.
    Str GetIndent() const;

  private:
    ion::gfxutils::ShaderManagerPtr shader_manager_;
    ion::text::FontManagerPtr       font_manager_;
    FileMapPtr                      file_map_;
    int                             level_ = 0;

    /// Names of render passes.
    StrVec pass_names_;

    /// Maps every known shader name to a corresponding ProgramInfo struct.
    std::unordered_map<Str, ProgramInfo> program_map_;
};

}  // namespace SG
