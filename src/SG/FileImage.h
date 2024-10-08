//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

#include <ion/gfx/image.h>

#include "SG/Image.h"
#include "Util/FilePath.h"
#include "Util/Memory.h"

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
    Parser::TField<Str> path_;
    ///@}

    friend class Parser::Registry;
};

}  // namespace SG
