#include "SG/ProceduralImage.h"

#include "SG/Exception.h"

namespace SG {

std::unordered_map<std::string,
                   ProceduralImage::ImageFunc> ProceduralImage::func_map_;

void ProceduralImage::AddFields() {
    AddField(function_);
}

ion::gfx::ImagePtr ProceduralImage::CreateIonImage(Tracker &tracker) {
    const std::string &func_name = GetFunctionName();
    if (func_name.empty())
        throw Exception("No function name supplied for ProceduralImage");

    // Look up the function name.
    auto it = func_map_.find(func_name);
    if (it == func_map_.end())
        throw Exception("ProceduralImage function '" + func_name +
                        "' not found");
    return it->second();
}

}  // namespace SG
