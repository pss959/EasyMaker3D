//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "SG/ShaderProgram.h"

#include <ion/gfxutils/shadersourcecomposer.h>

#include "SG/Exception.h"
#include "SG/FileMap.h"
#include "SG/IonContext.h"
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

void ShaderProgram::SetUpIon(const IonContextPtr &ion_context) {
    // This should be called only once since these are never shared.
    ASSERT(! ion_program_);

    // Create a ShaderInputRegistry.
    auto &shader_manager = *ion_context->GetShaderManager();
    ShaderInputRegistryPtr reg = CreateRegistry_(shader_manager);

    KLOG('Z', ion_context->GetIndent() << "SetUpIon for " << GetDesc()
         << " with registry " << reg.Get());

    // Create a StringComposer for each supplied ShaderSource.
    ShaderManager::ShaderSourceComposerSet sscs;
    auto &file_map = ion_context->GetFileMap();
    sscs.vertex_source_composer   = CreateComposer_(
        "_vp", file_map, GetVertexSource());
    sscs.geometry_source_composer = CreateComposer_(
        "_gp", file_map, GetGeometrySource());
    sscs.fragment_source_composer = CreateComposer_(
        "_fp", file_map, GetFragmentSource());

    // There has to be at least a vertex composer.
    if (! sscs.vertex_source_composer)
        throw Exception("No vertex program for " + GetDesc());

    // Create the Ion ShaderProgram. Note that this does NOT compile the
    // source; that is done during rendering. There is no way for this to
    // fail.
    ion_program_ = shader_manager.CreateShaderProgram(GetName(), reg, sscs);
    ASSERT(ion_program_.Get());
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
        KLOG('r', GetDesc() << " including Ion registry "
             << prog->GetRegistry().Get() << " from " << GetInheritFrom());
    }
    else {
        reg->IncludeGlobalRegistry();
        KLOG('r', GetDesc() << " including global Ion registry");
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
    const Str &suffix, FileMap &file_map,
    const ShaderSourcePtr &source) {
    ShaderSourceComposerPtr composer;

    // Do nothing if there is no source.
    if (source) {
        // Check to see if the source was already loaded.
        const FilePath path =
            FilePath::GetFullResourcePath("shaders", source->GetFilePath());
        Str str = file_map.FindString(path);

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
