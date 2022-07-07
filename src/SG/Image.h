#pragma once

#include <ion/gfx/image.h>

#include "Base/Memory.h"
#include "SG/Object.h"
#include "SG/SubImage.h"

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
    SubImagePtr FindSubImage(const std::string &name) const;

    /// Returns the Ion image. This is null until SetUpIon() is called.
    const ion::gfx::ImagePtr & GetIonImage() { return ion_image_; }

  protected:
    Image() {}

    virtual void AddFields() override;
    virtual void CreationDone() override;

    /// Derived classes must implement this to create an Ion Image.
    virtual ion::gfx::ImagePtr CreateIonImage(FileMap &file_map) = 0;

    /// Derived classes can call this to replace the current image with a new
    /// one when something changes.
    void ReplaceImage(const ion::gfx::ImagePtr new_image);

  private:
    /// Type of map storing SubImage data.
    typedef std::unordered_map<std::string, SubImagePtr> SubImageMap_;

    /// \name Parsed Fields
    ///@{
    Parser::ObjectListField<SubImage> sub_images_{"sub_images"};
    ///@}

    /// Maps name of a SubImage to a SubImage instance.
    SubImageMap_ sub_image_map_;

    ion::gfx::ImagePtr ion_image_;  /// Associated Ion Image.
};

}  // namespace SG
