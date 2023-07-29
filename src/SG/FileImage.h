#pragma once

#include <string>

#include <ion/gfx/image.h>

#include "Base/Memory.h"
#include "SG/Image.h"
#include "Util/FilePath.h"

namespace Parser { class Registry; }

namespace SG {

DECL_SHARED_PTR(FileImage);

/// A FileImage object wraps an Ion image read from a file.
///
/// \ingroup SG
class FileImage : public Image {
  public:
    /// Returns the path that the image was read from.
    FilePath GetFilePath() const { return path_.GetValue(); }

    /// Sets the file path. This will assert if an Ion image was already set up.
    void SetFilePath(const FilePath &path);

    /// Implements this to read the image if necessary.
    virtual ion::gfx::ImagePtr CreateIonImage() override;

  protected:
    FileImage() {}

    virtual void AddFields() override;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::TField<std::string> path_;
    ///@}

    friend class Parser::Registry;
};

}  // namespace SG
