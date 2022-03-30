#include "Tools/FindTools.h"

#include "Items/Shelf.h"
#include "SG/Search.h"
#include "Widgets/IconWidget.h"

namespace {

// ----------------------------------------------------------------------------
// Helper class that is used to find Tools add them.
// ----------------------------------------------------------------------------

class ToolFinder_ {
  public:
    explicit ToolFinder_(const SG::Scene &scene) :
        scene_(scene),
        tool_root_(*SG::FindNodeInScene(scene, "Tools")) {
    }

    /// The destructor disables searching in the Tools node so the real tools
    // are found when searching for widgets.
    ~ToolFinder_() {
        tool_root_.SetFlagEnabled(SG::Node::Flag::kSearch, false);
    }

    /// Adds the tool with the given type name.
    void AddTool(const std::string &type_name) {
        auto tool = SG::FindTypedNodeUnderNode<Tool>(tool_root_, type_name);
        tools_.push_back(tool);
    }

    /// Adds all tools from the named shelf.
    void AddToolsFromShelf(const std::string &shelf_name);

    /// Returns the resulting vector of Tools.
    const std::vector<ToolPtr> GetTools() const { return tools_; }

  private:
    const SG::Scene &scene_;
    SG::Node        &tool_root_;

    std::vector<ToolPtr> tools_;  ///< All Tools that were found.
};

void ToolFinder_::AddToolsFromShelf(const std::string &shelf_name) {
    // Add Tools in the order they are found on the shelf. This is important
    // for general Tools so that switching is consistent.
    const Shelf &shelf = *SG::FindTypedNodeInScene<Shelf>(scene_, shelf_name);
    for (const IconWidgetPtr &icon: shelf.GetIcons())
        AddTool(Util::EnumToWord(icon->GetAction()));
}

}  // anonymous namespace

// ----------------------------------------------------------------------------
// Public functions.
// ----------------------------------------------------------------------------

std::vector<ToolPtr> FindTools(const SG::Scene &scene) {
    ToolFinder_ finder(scene);

    // Special case for PassiveTool.
    finder.AddTool("PassiveTool");

    // General tools.
    finder.AddToolsFromShelf("GeneralToolShelf");

    // Specialized tools are handled individually.
    finder.AddTool("BevelTool");
    finder.AddTool("CylinderTool");
    finder.AddTool("ImportTool");
    finder.AddTool("TextTool");

    return finder.GetTools();
}
