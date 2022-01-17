#include "Panes/ImagePane.h"

#include "Parser/Registry.h"
#include "SG/FileImage.h"
#include "SG/Texture.h"
#include "SG/UniformBlock.h"
#include "Util/Assert.h"
#include "Util/General.h"

void ImagePane::AddFields() {
    AddField(path_);
    Pane::AddFields();
}

bool ImagePane::IsValid(std::string &details) {
    if (! Pane::IsValid(details))
        return false;

    if (path_.GetValue().empty()) {
        details = "No image file path specified";
        return false;
    }

    return true;
}

void ImagePane::CreationDone(bool is_template) {
    Pane::CreationDone(is_template);

    if (! is_template) {
        // Access the SG::FileImage and set its path.
        ASSERT(! GetUniformBlocks().empty());
        auto &block = GetUniformBlocks()[0];
        ASSERT(! block->GetTextures().empty());
        auto &tex = block->GetTextures()[0];
        auto image = Util::CastToDerived<SG::FileImage>(tex->GetImage());
        ASSERT(image);
        image->SetFilePath(path_.GetValue());
    }
}
