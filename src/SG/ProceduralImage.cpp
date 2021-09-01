#include "SG/ProceduralImage.h"

#include "SG/Exception.h"
#include "SG/SpecBuilder.h"

namespace SG {

std::unordered_map<std::string,
                   ProceduralImage::ImageFunc> ProceduralImage::func_map_;

void ProceduralImage::SetUpIon(IonContext &context) {
    if (! GetIonImage()) {
        if (function_.empty())
            throw Exception("No function name supplied for ProceduralImage");

        // Look up the function name.
        auto it = func_map_.find(function_);
        if (it == func_map_.end())
            throw Exception("ProceduralImage function '" + function_ +
                            "' not found");
        SetIonImage(it->second());
    }
}

Parser::ObjectSpec ProceduralImage::GetObjectSpec() {
    SG::SpecBuilder<ProceduralImage> builder;
    builder.AddString("function", &ProceduralImage::function_);
    return Parser::ObjectSpec{
        "ProceduralImage", false, []{ return new ProceduralImage; },
        builder.GetSpecs() };
}

}  // namespace SG
