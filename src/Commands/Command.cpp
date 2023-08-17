#include "Commands/Command.h"

#include "Util/FilePath.h"
#include "Util/General.h"
#include "Util/String.h"

void Command::AddOrphanedCommands(const std::vector<CommandPtr> &commands) {
    Util::AppendVector(commands, orphaned_commands_);
}

Str Command::GetModelDesc(const Str &model_name) {
    return "Model \"" + model_name + "\"";
}

Str Command::GetModelsDesc(const StrVec &model_names) {
    return model_names.size() == 1 ? GetModelDesc(model_names[0]) :
        Util::ToString(model_names.size()) + " Models";
}

Str Command::FixPath(const Str &path_string) {
    const FilePath path(path_string);
    return path.IsAbsolute() ? path_string : path.GetAbsolute().ToString();
}

