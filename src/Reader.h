#pragma once

#include "SG/Typedefs.h"
#include "Util/FilePath.h"

#include <ion/gfxutils/shadermanager.h>
#include <ion/text/fontmanager.h>

namespace Parser { class Parser; }
namespace SG     { class Tracker; }

//! The Reader class reads an SG::Scene from a file. Any failure results in a
//! Parser::Exception or SG::Exception being thrown.
class Reader {
  public:
    //! The constructor is passed a Tracker instance that is used to track
    //! resources, an Ion ShaderManager used to create shaders, and an Ion
    //! FontManager used to manage fonts.
    Reader(SG::Tracker &tracker,
           const ion::gfxutils::ShaderManagerPtr &shader_manager,
           const ion::text::FontManagerPtr &font_manager);
    ~Reader();

    //! Reads and returns a Scene from the given path, which must be an
    //! absolute path. If set_up_ion is true (the default), this also sets up
    //! Ion objects in the SG objects; turning this off is probably useful only
    //! for tests.
    SG::ScenePtr ReadScene(const Util::FilePath &path, bool set_up_ion = true);

  private:
    //! Tracker instance used to track resources to avoid extra loading.
    SG::Tracker                    &tracker_;

    //! ShaderManager used to create shaders.
    ion::gfxutils::ShaderManagerPtr shader_manager_;

    //! FontManager used to manage Ion fonts.
    ion::text::FontManagerPtr       font_manager_;
};
