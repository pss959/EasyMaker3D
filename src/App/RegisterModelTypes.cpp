//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Models/BentModel.h"
#include "Models/BeveledModel.h"
#include "Models/BoxModel.h"
#include "Models/CSGModel.h"
#include "Models/ClippedModel.h"
#include "Models/CylinderModel.h"
#include "Models/ExtrudedModel.h"
#include "Models/HullModel.h"
#include "Models/ImportedModel.h"
#include "Models/MirroredModel.h"
#include "Models/RevSurfModel.h"
#include "Models/RootModel.h"
#include "Models/SphereModel.h"
#include "Models/TaperedModel.h"
#include "Models/TextModel.h"
#include "Models/TorusModel.h"
#include "Models/TwistedModel.h"
#include "Parser/Registry.h"

#define REGISTER_TYPE_(T) Parser::Registry::AddType<T>(#T)

void RegisterModelTypes_();

void RegisterModelTypes_() {
    REGISTER_TYPE_(BentModel);
    REGISTER_TYPE_(BeveledModel);
    REGISTER_TYPE_(BoxModel);
    REGISTER_TYPE_(CSGModel);
    REGISTER_TYPE_(ClippedModel);
    REGISTER_TYPE_(CylinderModel);
    REGISTER_TYPE_(ExtrudedModel);
    REGISTER_TYPE_(HullModel);
    REGISTER_TYPE_(ImportedModel);
    REGISTER_TYPE_(MirroredModel);
    REGISTER_TYPE_(RootModel);
    REGISTER_TYPE_(RevSurfModel);
    REGISTER_TYPE_(SphereModel);
    REGISTER_TYPE_(TaperedModel);
    REGISTER_TYPE_(TextModel);
    REGISTER_TYPE_(TorusModel);
    REGISTER_TYPE_(TwistedModel);
}

#undef REGISTER_TYPE_

