//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

#include <ion/gfx/image.h>

#include "SG/Object.h"
#include "SG/SubImage.h"
#include "Util/Memory.h"

namespace SG {

class FileMap;
DECL_SHARED_PTR(Image);
DECL_SHARED_PTR(IonContext);

/// Image is an abstract base class for objects that wrap an Ion image.
///
/// \ingroup SG
class Image : public Object {
  public:
    /// Creates, stores, and returns the Ion Image.
    ion::gfx::ImagePtr SetUpIon(const IonContextPtr &ion_context);

    /// Returns the named SubImage if it exists, or a null pointer otherwise.
    SubImagePtr FindSubImage(const Str &name) const;

    /// Returns the Ion image. This is null until SetUpIon() is called.
    const ion::gfx::ImagePtr & GetIonImage() { return ion_image_; }

  protected:
    Image() {}

    virtual void AddFields() override;
    virtual void CreationDone() override;

    /// Derived classes must implement this to create an Ion Image.
    virtual ion::gfx::ImagePtr CreateIonImage() = 0;

    /// Provides access to the FileMap in the IonContext.
    FileMap & GetFileMap() const;

    /// Derived classes can call this to replace the current image with a new
    /// one when something changes.
    void ReplaceImage(const ion::gfx::ImagePtr new_image);

  private:
    /// Type of map storing SubImage data.
    using SubImageMap_ = std::unordered_map<Str, SubImagePtr>;

    /// \name Parsed Fields
    ///@{
    Parser::ObjectListField<SubImage> sub_images_;
    ///@}

    /// Saves the IonContext used to create the Ion image.
    IonContextPtr ion_context_;

    /// Maps name of a SubImage to a SubImage instance.
    SubImageMap_ sub_image_map_;

    ion::gfx::ImagePtr ion_image_;  ///< Associated Ion Image.
};

}  // namespace SG
