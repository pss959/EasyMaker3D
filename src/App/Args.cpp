#include "App/Args.h"

#include "Util/Assert.h"
#include "Util/String.h"
#include "Util/Tuning.h"

Args::Args(int argc, const char **argv, const Str &usage) :
    args_(docopt::docopt(usage, { argv + 1, argv + argc },
                         true,         // Show help if requested
                         "Version " + TK::kVersionString)) {
}

Str Args::GetString(const Str &name) const {
    const auto &arg = GetArg_(name);
    return arg && arg.isString() ? arg.asString() : "";
}

bool Args::GetBool(const Str &name) const {
    const auto &arg = GetArg_(name);
    return arg && arg.isBool() ? arg.asBool() : false;
}

int Args::GetAsInt(const Str &name, int default_value) const {
    int n;
    return Util::StringToInteger(GetString(name), n) ? n : default_value;
}

const docopt::value & Args::GetArg_(const Str &name) const {
    ASSERTM(args_.find(name) != args_.end(), name);
    return args_.at(name);
}
