#include "Executors/InitExecutors.h"

#include "Executors/ChangeBevelExecutor.h"
#include "Executors/ChangeCSGExecutor.h"
#include "Executors/ChangeCylinderExecutor.h"
#include "Executors/ConvertBevelExecutor.h"
#include "Executors/CreateCSGExecutor.h"
#include "Executors/CreatePrimitiveExecutor.h"
#include "Executors/EdgeTargetExecutor.h"
#include "Executors/PointTargetExecutor.h"
#include "Executors/TranslateExecutor.h"

std::vector<ExecutorPtr> InitExecutors() {
    std::vector<ExecutorPtr> executors;

#define ADDEXEC_(TYPE) executors.push_back(ExecutorPtr(new TYPE))

    ADDEXEC_(ChangeBevelExecutor);
    ADDEXEC_(ChangeCSGExecutor);
    ADDEXEC_(ChangeCylinderExecutor);
    ADDEXEC_(ConvertBevelExecutor);
    ADDEXEC_(CreateCSGExecutor);
    ADDEXEC_(CreatePrimitiveExecutor);
    ADDEXEC_(EdgeTargetExecutor);
    ADDEXEC_(PointTargetExecutor);
    ADDEXEC_(TranslateExecutor);

#undef ADDEXEC_

    return executors;
}
