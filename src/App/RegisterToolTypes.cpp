#include "Parser/Registry.h"
#include "Tools/BevelTool.h"
#include "Tools/CSGTool.h"
#include "Tools/ClipTool.h"
#include "Tools/ColorTool.h"
#include "Tools/ComplexityTool.h"
#include "Tools/CylinderTool.h"
#include "Tools/ExtrudedTool.h"
#include "Tools/ImportTool.h"
#include "Tools/MirrorTool.h"
#include "Tools/NameTool.h"
#include "Tools/PassiveTool.h"
#include "Tools/RevSurfTool.h"
#include "Tools/RotationTool.h"
#include "Tools/ScaleTool.h"
#include "Tools/TextTool.h"
#include "Tools/TorusTool.h"
#include "Tools/TranslationTool.h"

#define REGISTER_TYPE_(T) Parser::Registry::AddType<T>(#T)

void RegisterToolTypes_();

void RegisterToolTypes_() {
    REGISTER_TYPE_(BevelTool);
    REGISTER_TYPE_(CSGTool);
    REGISTER_TYPE_(ClipTool);
    REGISTER_TYPE_(ColorTool);
    REGISTER_TYPE_(ComplexityTool);
    REGISTER_TYPE_(CylinderTool);
    REGISTER_TYPE_(ExtrudedTool);
    REGISTER_TYPE_(ImportTool);
    REGISTER_TYPE_(MirrorTool);
    REGISTER_TYPE_(NameTool);
    REGISTER_TYPE_(PassiveTool);
    REGISTER_TYPE_(RevSurfTool);
    REGISTER_TYPE_(RotationTool);
    REGISTER_TYPE_(ScaleTool);
    REGISTER_TYPE_(TextTool);
    REGISTER_TYPE_(TorusTool);
    REGISTER_TYPE_(TranslationTool);
}

#undef REGISTER_TYPE_

