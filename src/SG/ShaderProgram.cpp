#include "SG/ShaderProgram.h"

#include <ion/gfxutils/shadersourcecomposer.h>

#include "SG/Exception.h"
#include "SG/Tracker.h"
#include "SG/UniformBlock.h"
#include "Util/KLog.h"
#include "Util/Read.h"

using ion::gfx::ShaderInputRegistry;
using ion::gfx::ShaderInputRegistryPtr;
using ion::gfxutils::ShaderManager;
using ion::gfxutils::ShaderSourceComposerPtr;
using ion::gfxutils::StringComposer;

namespace SG {

void ShaderProgram::AddFields() {
    AddField(vertex_source_);
    AddField(geometry_source_);
    AddField(fragment_source_);
    AddField(uniform_defs_);
    AddField(uniform_block_);
}

void ShaderProgram::CreateIonShaderProgram(Tracker &tracker,
                                           ShaderManager &shader_manager) {
    ASSERT(! ion_program_);

    // Create a ShaderInputRegistry.
    ShaderInputRegistryPtr reg = CreateRegistry_();

    // Create a StringComposer for each supplied ShaderSource.
    ShaderManager::ShaderSourceComposerSet sscs;
    sscs.vertex_source_composer   = CreateComposer_(
        "_vp", tracker, GetVertexSource());
    sscs.geometry_source_composer = CreateComposer_(
        "_gp", tracker, GetGeometrySource());
    sscs.fragment_source_composer = CreateComposer_(
        "_fp", tracker, GetFragmentSource());

    // There has to be at least a vertex composer.
    if (! sscs.vertex_source_composer)
        throw Exception("No vertex program for " + GetDesc());

    // Compile the program.
    ion_program_ = shader_manager.CreateShaderProgram(GetName(), reg, sscs);
    if (! ion_program_ || ! ion_program_->GetInfoLog().empty())
        throw Exception("Failed to compile shader program for " + GetDesc() +
                        ":\n" + ion_program_->GetInfoLog());
}

ShaderInputRegistryPtr ShaderProgram::CreateRegistry_() {
    // Create the registry. Always include the global registry.
    ShaderInputRegistryPtr reg(new ShaderInputRegistry);
    KLOG('r', GetDesc() << " created Ion registry "
         << reg.Get() << " for " << GetDesc());
    reg->IncludeGlobalRegistry();

    // Add uniform definitions.
    for (const auto &def: GetUniformDefs()) {
        reg->Add<ion::gfx::Uniform>(
            ShaderInputRegistry::UniformSpec(def->GetName(),
                                             def->GetValueType()));
        KLOG('u', GetDesc() << " added " << def->GetDesc()
             << " to reg " << reg.Get());
    }
    return reg;
}

ion::gfxutils::ShaderSourceComposerPtr ShaderProgram::CreateComposer_(
    const std::string &suffix, Tracker &tracker,
    const ShaderSourcePtr &source) {
    ShaderSourceComposerPtr composer;

    // Do nothing if there is no source.
    if (source) {
        // Check to see if the source was already loaded.
        const Util::FilePath path = Util::FilePath::GetFullResourcePath(
            "shaders", source->GetFilePath());
        std::string str = tracker.FindString(path);

        // Read the file if necessary. Allow includes.
        if (str.empty()) {
            if (! Util::ReadFile(path, str, true))
                throw Exception("Unable to read shader file '" +
                                path.ToString() + "'");
            tracker.AddString(path, str);
        }
        composer.Reset(new StringComposer(GetName() + suffix, str));
    }
    return composer;
}

}  // namespace SG
