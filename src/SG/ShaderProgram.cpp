#include "SG/ShaderProgram.h"

#include <ion/gfxutils/shadersourcecomposer.h>

#include "SG/Exception.h"
#include "SG/FileMap.h"
#include "Util/KLog.h"
#include "Util/Read.h"

using ion::gfx::ShaderInputRegistry;
using ion::gfx::ShaderInputRegistryPtr;
using ion::gfxutils::ShaderManager;
using ion::gfxutils::ShaderSourceComposerPtr;
using ion::gfxutils::StringComposer;

namespace SG {

void ShaderProgram::AddFields() {
    AddField(inherit_from_.Init("inherit_from"));
    AddField(vertex_source_.Init("vertex_source"));
    AddField(geometry_source_.Init("geometry_source"));
    AddField(fragment_source_.Init("fragment_source"));
    AddField(uniform_defs_.Init("uniform_defs"));

    Object::AddFields();
}

void ShaderProgram::SetUpIon(FileMap &file_map, ShaderManager &shader_manager) {
    // This should be called only once since these are never shared.
    ASSERT(! ion_program_);

    // Create a ShaderInputRegistry.
    ShaderInputRegistryPtr reg = CreateRegistry_(shader_manager);

    // Create a StringComposer for each supplied ShaderSource.
    ShaderManager::ShaderSourceComposerSet sscs;
    sscs.vertex_source_composer   = CreateComposer_(
        "_vp", file_map, GetVertexSource());
    sscs.geometry_source_composer = CreateComposer_(
        "_gp", file_map, GetGeometrySource());
    sscs.fragment_source_composer = CreateComposer_(
        "_fp", file_map, GetFragmentSource());

    // There has to be at least a vertex composer.
    if (! sscs.vertex_source_composer)
        throw Exception("No vertex program for " + GetDesc());

    // Compile the program.
    ion_program_ = shader_manager.CreateShaderProgram(GetName(), reg, sscs);
    if (! ion_program_ || ! ion_program_->GetInfoLog().empty())
        throw Exception("Failed to compile shader program for " + GetDesc() +
                        ":\n" + ion_program_->GetInfoLog());
}

ShaderInputRegistryPtr ShaderProgram::CreateRegistry_(
    ShaderManager &shader_manager) {
    // Create the registry.
    ShaderInputRegistryPtr reg(new ShaderInputRegistry);
    KLOG('r', GetDesc() << " created Ion registry "
         << reg.Get() << " for " << GetDesc());

    // If a shader to inherit from is specified, include its
    // registry. Otherwise, include the global registry.
    if (! GetInheritFrom().empty()) {
        auto prog = shader_manager.GetShaderProgram(GetInheritFrom());
        if (! prog)
            throw Exception("Unknown shader for " + GetDesc() +
                            " to inherit from: " + GetInheritFrom());
        reg->Include(prog->GetRegistry());
    }
    else {
        reg->IncludeGlobalRegistry();
    }

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
    const std::string &suffix, FileMap &file_map,
    const ShaderSourcePtr &source) {
    ShaderSourceComposerPtr composer;

    // Do nothing if there is no source.
    if (source) {
        // Check to see if the source was already loaded.
        const FilePath path =
            FilePath::GetFullResourcePath("shaders", source->GetFilePath());
        std::string str = file_map.FindString(path);

        // Read the file if necessary. Allow includes.
        if (str.empty()) {
            if (! Util::ReadFile(path, str, true))
                throw Exception("Unable to read shader file '" +
                                path.ToString() + "'");
            file_map.AddString(path, str);
        }
        composer.Reset(new StringComposer(GetName() + suffix, str));
    }
    return composer;
}

}  // namespace SG
