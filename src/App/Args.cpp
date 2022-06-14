#include "App/Args.h"

#include "Util/VersionInfo.h"

Args::Args(int argc, const char **argv, const std::string &usage) :
    args_(docopt::docopt(usage, { argv + 1, argv + argc },
                         true,         // Show help if requested
                         "Version " + Util::kVersionString)) {
}

std::string Args::GetString(const std::string &name) const {
    const auto &arg = args_.at(name);
    if (arg && arg.isString())
        return arg.asString();
    else
        return "";
}

bool Args::GetBool(const std::string &name) const {
    const auto &arg = args_.at(name);
    if (arg && arg.isBool())
        return arg.asBool();
    else
        return false;
}
