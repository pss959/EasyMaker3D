#pragma once

#include <string>

#include <ion/gfx/image.h>

#include "SG/Image.h"
#include "Util/FilePath.h"

namespace Parser { class Registry; }

namespace SG {

/// A FileImage object wraps an Ion image read from a file.
class FileImage : public Image {
  public:
    virtual void AddFields() override;

    /// Returns the path that the image was read from.
    Util::FilePath GetFilePath() const { return path_.GetValue(); }

    /// Implements this to read the image if necessary.
    virtual ion::gfx::ImagePtr CreateIonImage(Tracker &tracker) override;

  protected:
    FileImage() {}

  private:
    /// \name Parsed Fields
    ///@{
    Parser::TField<std::string> path_{"path"};
    ///@}

    friend class Parser::Registry;
};

}  // namespace SG
