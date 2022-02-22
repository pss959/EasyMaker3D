#include "Tools/FindTools.h"

#include "SG/Search.h"

std::vector<ToolPtr> FindTools(const SG::Node &root_node) {
    std::vector<ToolPtr> tools;

    auto add_tool = [&](const std::string &type_name){
        tools.push_back(SG::FindTypedNodeUnderNode<Tool>(root_node, type_name));
    };

    add_tool("BevelTool");
    add_tool("CylinderTool");
    add_tool("PassiveTool");
    add_tool("TranslationTool");

    return tools;
}
