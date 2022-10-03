#include "Commands/Command.h"

#include "Util/FilePath.h"
#include "Util/General.h"
#include "Util/String.h"

void Command::AddOrphanedCommands(const std::vector<CommandPtr> &commands) {
    Util::AppendVector(commands, orphaned_commands_);
}

std::string Command::GetModelDesc(const std::string &model_name) {
    return "Model \"" + model_name + "\"";
}

std::string Command::GetModelsDesc(
    const std::vector<std::string> &model_names) {
    return model_names.size() == 1 ? GetModelDesc(model_names[0]) :
        Util::ToString(model_names.size()) + " Models";
}

std::string Command::FixPath(const std::string &path_string) {
    const FilePath path(path_string);
    return path.IsAbsolute() ? path_string : path.GetAbsolute().ToString();
}

