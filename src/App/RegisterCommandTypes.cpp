#include "Commands/ChangeBevelCommand.h"
#include "Commands/ChangeCSGOperationCommand.h"
#include "Commands/ChangeClipCommand.h"
#include "Commands/ChangeColorCommand.h"
#include "Commands/ChangeComplexityCommand.h"
#include "Commands/ChangeCylinderCommand.h"
#include "Commands/ChangeEdgeTargetCommand.h"
#include "Commands/ChangeExtrudedCommand.h"
#include "Commands/ChangeImportedModelCommand.h"
#include "Commands/ChangeMirrorCommand.h"
#include "Commands/ChangeNameCommand.h"
#include "Commands/ChangeOrderCommand.h"
#include "Commands/ChangePointTargetCommand.h"
#include "Commands/ChangeRevSurfCommand.h"
#include "Commands/ChangeTextCommand.h"
#include "Commands/ChangeTorusCommand.h"
#include "Commands/ChangeTwistCommand.h"
#include "Commands/CommandList.h"
#include "Commands/ConvertBevelCommand.h"
#include "Commands/ConvertClipCommand.h"
#include "Commands/ConvertMirrorCommand.h"
#include "Commands/ConvertTwistCommand.h"
#include "Commands/CopyCommand.h"
#include "Commands/CreateCSGModelCommand.h"
#include "Commands/CreateExtrudedModelCommand.h"
#include "Commands/CreateHullModelCommand.h"
#include "Commands/CreateImportedModelCommand.h"
#include "Commands/CreatePrimitiveModelCommand.h"
#include "Commands/CreateRevSurfModelCommand.h"
#include "Commands/CreateTextModelCommand.h"
#include "Commands/DeleteCommand.h"
#include "Commands/LinearLayoutCommand.h"
#include "Commands/PasteCommand.h"
#include "Commands/RadialLayoutCommand.h"
#include "Commands/RotateCommand.h"
#include "Commands/ScaleCommand.h"
#include "Commands/TranslateCommand.h"
#include "Parser/Registry.h"

#define REGISTER_TYPE_(T) Parser::Registry::AddType<T>(#T)

void RegisterCommandTypes_();

void RegisterCommandTypes_() {
    // Types required for command processing.
    REGISTER_TYPE_(CommandList);

    REGISTER_TYPE_(ChangeBevelCommand);
    REGISTER_TYPE_(ChangeCSGOperationCommand);
    REGISTER_TYPE_(ChangeClipCommand);
    REGISTER_TYPE_(ChangeColorCommand);
    REGISTER_TYPE_(ChangeComplexityCommand);
    REGISTER_TYPE_(ChangeCylinderCommand);
    REGISTER_TYPE_(ChangeEdgeTargetCommand);
    REGISTER_TYPE_(ChangeExtrudedCommand);
    REGISTER_TYPE_(ChangeImportedModelCommand);
    REGISTER_TYPE_(ChangeMirrorCommand);
    REGISTER_TYPE_(ChangeNameCommand);
    REGISTER_TYPE_(ChangeOrderCommand);
    REGISTER_TYPE_(ChangePointTargetCommand);
    REGISTER_TYPE_(ChangeRevSurfCommand);
    REGISTER_TYPE_(ChangeTextCommand);
    REGISTER_TYPE_(ChangeTorusCommand);
    REGISTER_TYPE_(ChangeTwistCommand);
    REGISTER_TYPE_(ConvertBevelCommand);
    REGISTER_TYPE_(ConvertClipCommand);
    REGISTER_TYPE_(ConvertMirrorCommand);
    REGISTER_TYPE_(ConvertTwistCommand);
    REGISTER_TYPE_(CopyCommand);
    REGISTER_TYPE_(CreateCSGModelCommand);
    REGISTER_TYPE_(CreateExtrudedModelCommand);
    REGISTER_TYPE_(CreateHullModelCommand);
    REGISTER_TYPE_(CreateImportedModelCommand);
    REGISTER_TYPE_(CreatePrimitiveModelCommand);
    REGISTER_TYPE_(CreateRevSurfModelCommand);
    REGISTER_TYPE_(CreateTextModelCommand);
    REGISTER_TYPE_(DeleteCommand);
    REGISTER_TYPE_(LinearLayoutCommand);
    REGISTER_TYPE_(PasteCommand);
    REGISTER_TYPE_(RadialLayoutCommand);
    REGISTER_TYPE_(RotateCommand);
    REGISTER_TYPE_(ScaleCommand);
    REGISTER_TYPE_(TranslateCommand);
}

#undef REGISTER_TYPE_
