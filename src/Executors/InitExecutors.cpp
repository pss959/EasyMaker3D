#include "Executors/InitExecutors.h"

#include "Executors/ChangeBevelExecutor.h"
#include "Executors/ChangeCSGExecutor.h"
#include "Executors/ChangeColorExecutor.h"
#include "Executors/ChangeComplexityExecutor.h"
#include "Executors/ChangeCylinderExecutor.h"
#include "Executors/ChangeNameExecutor.h"
#include "Executors/ChangeOrderExecutor.h"
#include "Executors/ConvertBevelExecutor.h"
#include "Executors/CreateCSGExecutor.h"
#include "Executors/CreateHullExecutor.h"
#include "Executors/CreateImportedExecutor.h"
#include "Executors/CreatePrimitiveExecutor.h"
#include "Executors/EdgeTargetExecutor.h"
#include "Executors/PointTargetExecutor.h"
#include "Executors/RotateExecutor.h"
#include "Executors/ScaleExecutor.h"
#include "Executors/TranslateExecutor.h"

std::vector<ExecutorPtr> InitExecutors() {
    std::vector<ExecutorPtr> executors;

#define ADDEXEC_(TYPE) executors.push_back(ExecutorPtr(new TYPE))

    ADDEXEC_(ChangeBevelExecutor);
    ADDEXEC_(ChangeCSGExecutor);
    ADDEXEC_(ChangeColorExecutor);
    ADDEXEC_(ChangeComplexityExecutor);
    ADDEXEC_(ChangeCylinderExecutor);
    ADDEXEC_(ChangeNameExecutor);
    ADDEXEC_(ChangeOrderExecutor);
    ADDEXEC_(ConvertBevelExecutor);
    ADDEXEC_(CreateCSGExecutor);
    ADDEXEC_(CreateHullExecutor);
    ADDEXEC_(CreateImportedExecutor);
    ADDEXEC_(CreatePrimitiveExecutor);
    ADDEXEC_(EdgeTargetExecutor);
    ADDEXEC_(PointTargetExecutor);
    ADDEXEC_(RotateExecutor);
    ADDEXEC_(ScaleExecutor);
    ADDEXEC_(TranslateExecutor);

#undef ADDEXEC_

    return executors;
}
