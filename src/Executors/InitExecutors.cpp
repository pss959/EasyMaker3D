#include "Executors/InitExecutors.h"

#include "Executors/ChangeBevelExecutor.h"
#include "Executors/ChangeCSGExecutor.h"
#include "Executors/ChangeClipExecutor.h"
#include "Executors/ChangeColorExecutor.h"
#include "Executors/ChangeComplexityExecutor.h"
#include "Executors/ChangeCylinderExecutor.h"
#include "Executors/ChangeImportedExecutor.h"
#include "Executors/ChangeMirrorExecutor.h"
#include "Executors/ChangeNameExecutor.h"
#include "Executors/ChangeOrderExecutor.h"
#include "Executors/ChangeRevSurfExecutor.h"
#include "Executors/ChangeTextExecutor.h"
#include "Executors/ChangeTorusExecutor.h"
#include "Executors/ConvertBevelExecutor.h"
#include "Executors/ConvertClipExecutor.h"
#include "Executors/ConvertMirrorExecutor.h"
#include "Executors/CopyExecutor.h"
#include "Executors/CreateCSGExecutor.h"
#include "Executors/CreateHullExecutor.h"
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

    ADDEXEC_(ChangeBevelExecutor);
    ADDEXEC_(ChangeCSGExecutor);
    ADDEXEC_(ChangeClipExecutor);
    ADDEXEC_(ChangeColorExecutor);
    ADDEXEC_(ChangeComplexityExecutor);
    ADDEXEC_(ChangeCylinderExecutor);
    ADDEXEC_(ChangeImportedExecutor);
    ADDEXEC_(ChangeMirrorExecutor);
    ADDEXEC_(ChangeNameExecutor);
    ADDEXEC_(ChangeOrderExecutor);
    ADDEXEC_(ChangeRevSurfExecutor);
    ADDEXEC_(ChangeTextExecutor);
    ADDEXEC_(ChangeTorusExecutor);
    ADDEXEC_(ConvertBevelExecutor);
    ADDEXEC_(ConvertClipExecutor);
    ADDEXEC_(ConvertMirrorExecutor);
    ADDEXEC_(CopyExecutor);
    ADDEXEC_(CreateCSGExecutor);
    ADDEXEC_(CreateHullExecutor);
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
