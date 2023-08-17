#include "App/Args.h"

#include "Util/Assert.h"
#include "Util/Tuning.h"

Args::Args(int argc, const char **argv, const Str &usage) :
    args_(docopt::docopt(usage, { argv + 1, argv + argc },
                         true,         // Show help if requested
                         "Version " + TK::kVersionString)) {
}

Str Args::GetString(const Str &name) const {
    const auto &arg = GetArg_(name);
    if (arg && arg.isString())
        return arg.asString();
    else
        return "";
}

bool Args::GetBool(const Str &name) const {
    const auto &arg = GetArg_(name);
    if (arg && arg.isBool())
        return arg.asBool();
    else
        return false;
}

const docopt::value & Args::GetArg_(const Str &name) const {
    ASSERTM(args_.find(name) != args_.end(), name);
    return args_.at(name);
}
