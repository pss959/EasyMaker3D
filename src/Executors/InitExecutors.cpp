#include "Executors/InitExecutors.h"

#include "Executors/ChangeBendExecutor.h"
#include "Executors/ChangeBevelExecutor.h"
#include "Executors/ChangeCSGExecutor.h"
#include "Executors/ChangeClipExecutor.h"
#include "Executors/ChangeColorExecutor.h"
#include "Executors/ChangeComplexityExecutor.h"
#include "Executors/ChangeCylinderExecutor.h"
#include "Executors/ChangeExtrudedExecutor.h"
#include "Executors/ChangeImportedExecutor.h"
#include "Executors/ChangeMirrorExecutor.h"
#include "Executors/ChangeNameExecutor.h"
#include "Executors/ChangeOrderExecutor.h"
#include "Executors/ChangeRevSurfExecutor.h"
#include "Executors/ChangeTaperExecutor.h"
#include "Executors/ChangeTextExecutor.h"
#include "Executors/ChangeTorusExecutor.h"
#include "Executors/ChangeTwistExecutor.h"
#include "Executors/CombineCSGExecutor.h"
#include "Executors/CombineHullExecutor.h"
#include "Executors/ConvertBendExecutor.h"
#include "Executors/ConvertBevelExecutor.h"
#include "Executors/ConvertClipExecutor.h"
#include "Executors/ConvertMirrorExecutor.h"
#include "Executors/ConvertTaperExecutor.h"
#include "Executors/ConvertTwistExecutor.h"
#include "Executors/CopyExecutor.h"
#include "Executors/CreateExtrudedExecutor.h"
#include "Executors/CreateImportedExecutor.h"
#include "Executors/CreatePrimitiveExecutor.h"
#include "Executors/CreateRevSurfExecutor.h"
#include "Executors/CreateTextExecutor.h"
#include "Executors/DeleteExecutor.h"
#include "Executors/EdgeTargetExecutor.h"
#include "Executors/LinearLayoutExecutor.h"
#include "Executors/PasteExecutor.h"
#include "Executors/PointTargetExecutor.h"
#include "Executors/RadialLayoutExecutor.h"
#include "Executors/RotateExecutor.h"
#include "Executors/ScaleExecutor.h"
#include "Executors/TranslateExecutor.h"

std::vector<ExecutorPtr> InitExecutors() {
    std::vector<ExecutorPtr> executors;

#define ADDEXEC_(TYPE) executors.push_back(ExecutorPtr(new TYPE))

    ADDEXEC_(ChangeBendExecutor);
    ADDEXEC_(ChangeBevelExecutor);
    ADDEXEC_(ChangeCSGExecutor);
    ADDEXEC_(ChangeClipExecutor);
    ADDEXEC_(ChangeColorExecutor);
    ADDEXEC_(ChangeComplexityExecutor);
    ADDEXEC_(ChangeCylinderExecutor);
    ADDEXEC_(ChangeExtrudedExecutor);
    ADDEXEC_(ChangeImportedExecutor);
    ADDEXEC_(ChangeMirrorExecutor);
    ADDEXEC_(ChangeNameExecutor);
    ADDEXEC_(ChangeOrderExecutor);
    ADDEXEC_(ChangeRevSurfExecutor);
    ADDEXEC_(ChangeTaperExecutor);
    ADDEXEC_(ChangeTextExecutor);
    ADDEXEC_(ChangeTorusExecutor);
    ADDEXEC_(ChangeTwistExecutor);
    ADDEXEC_(CombineCSGExecutor);
    ADDEXEC_(CombineHullExecutor);
    ADDEXEC_(ConvertBendExecutor);
    ADDEXEC_(ConvertBevelExecutor);
    ADDEXEC_(ConvertClipExecutor);
    ADDEXEC_(ConvertMirrorExecutor);
    ADDEXEC_(ConvertTaperExecutor);
    ADDEXEC_(ConvertTwistExecutor);
    ADDEXEC_(CopyExecutor);
    ADDEXEC_(CreateExtrudedExecutor);
    ADDEXEC_(CreateImportedExecutor);
    ADDEXEC_(CreatePrimitiveExecutor);
    ADDEXEC_(CreateRevSurfExecutor);
    ADDEXEC_(CreateTextExecutor);
    ADDEXEC_(DeleteExecutor);
    ADDEXEC_(EdgeTargetExecutor);
    ADDEXEC_(LinearLayoutExecutor);
    ADDEXEC_(PasteExecutor);
    ADDEXEC_(PointTargetExecutor);
    ADDEXEC_(RadialLayoutExecutor);
    ADDEXEC_(RotateExecutor);
    ADDEXEC_(ScaleExecutor);
    ADDEXEC_(TranslateExecutor);

#undef ADDEXEC_

    return executors;
}
