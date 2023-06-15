#include "Panes/ImagePane.h"

#include "Parser/Registry.h"
#include "SG/FileImage.h"
#include "SG/Texture.h"
#include "SG/UniformBlock.h"
#include "Util/Assert.h"

void ImagePane::AddFields() {
    AddField(path_.Init("path"));

    LeafPane::AddFields();
}

bool ImagePane::IsValid(std::string &details) {
    if (! LeafPane::IsValid(details))
        return false;

    if (path_.GetValue().empty()) {
        details = "No image file path specified";
        return false;
    }

    return true;
}

void ImagePane::CreationDone() {
    LeafPane::CreationDone();

    if (! IsTemplate()) {
        // Access the SG::FileImage and set its path.
        ASSERT(! GetUniformBlocks().empty());
        auto &block = GetUniformBlocks()[0];
        ASSERT(! block->GetTextures().empty());
        auto &tex = block->GetTextures()[0];
        auto image = std::dynamic_pointer_cast<SG::FileImage>(tex->GetImage());
        ASSERT(image);
        image->SetFilePath(path_.GetValue());
    }
}

std::string ImagePane::ToString(bool is_brief) const {
    // Add the image path.
    return LeafPane::ToString(is_brief) + " '" + path_.GetValue() + "'";
}
