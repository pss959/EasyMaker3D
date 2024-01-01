#include "App/Args.h"

#include "Util/Assert.h"
#include "Util/General.h"
#include "Util/String.h"
#include "Util/Tuning.h"

Args::Args(int argc, const char **argv, const Str &usage) :
    args_(docopt::docopt(usage, { argv + 1, argv + argc },
                         true,         // Show help if requested
                         "Version " + TK::kVersionString)) {
}

bool Args::HasArg(const Str &name) const {
    return args_.find(name) != args_.end();
}

Str Args::GetString(const Str &name) const {
    const auto &arg = GetArg_(name);
    return arg && arg.isString() ? arg.asString() : "";
}

Str Args::GetStringChoice(const Str &name, const StrVec &choices) const {
    ASSERT(! choices.empty());
    auto str = GetString(name);
    if (str.empty())
        str = choices[0];
    else if (! Util::Contains(choices, str))
        throw docopt::DocoptArgumentError("Invalid choice for '" + name +
                                          "' argument: '" + str + "'");
    return str;
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
    ASSERTM(HasArg(name), name);
    return args_.at(name);
}
