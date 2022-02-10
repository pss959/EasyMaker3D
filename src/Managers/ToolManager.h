#pragma once

#include <memory>
#include <string>
#include <vector>

#include "Items/Grippable.h"
#include "SG/Change.h"
#include "SG/Typedefs.h"
#include "Tools/GeneralTool.h"
#include "Tools/PassiveTool.h"
#include "Tools/SpecializedTool.h"

class InstanceManager;
class TargetManager;

/// The ToolManager class manages interactive tools that may be attached to
/// Models in the scene.
///
/// There are 3 types of tools:
///   - General tools: these can be used for any Model (except for the
///     ComplexityTool, which affects most models).
///   - Specialized tools, which can be used only for a specific class of
///     Models.
///   - The PassiveTool, which is attached to all secondary selections and does
///     not provide any interaction.
///
/// Depending on the current selection, the ToolManager supports:
///   - Switching between the current general tool and the appropriate
///     specialized tool.
///   - If the current tool is a general tool, switching to the previous or
///     next general tool.
///
/// Note that a Selection is passed to several of these functions so that each
/// Tool has enough context to decide what to do, since some require knowledge
/// of all selected Models to operate.
///
/// ToolManager is derived from Grippable so that the current Tool (if any) can
/// interact correctly with grip drags; the ToolManager delegates all grip
/// operations to the current Tool.
///

/// \ingroup Managers
class ToolManager : public Grippable {
  public:
    // ------------------------------------------------------------------------
    // Initialization and update.
    // ------------------------------------------------------------------------

    /// The constructor is passed the TargetManager instance so an observer can
    /// be added to hide active tools while a target is being dragged.
    explicit ToolManager(TargetManager &target_manager);

    /// Sets the parent Node for all active Tools.
    void SetParentNode(const SG::NodePtr &parent_node);

    /// Sets a PassiveTool used for creating all PassiveTool instances attached
    /// to secondary selections.
    void SetPassiveTool(const PassiveToolPtr &tool);

    /// Adds a GeneralTool.
    void AddGeneralTool(const GeneralToolPtr &tool);

    /// Adds a SpecializedTool.
    void AddSpecializedTool(const SpecializedToolPtr &tool);

    /// Sets the GeneralTool to use by default.
    void SetDefaultGeneralTool(const GeneralToolPtr &tool);

    /// Resets the ToolManager completely (for reload).
    void Reset();

    /// Resets the ToolManager for a new session.
    void ResetSession();

    /// Clears all tools that have been added. This should be called when
    /// reloading the scene before adding tools again.
    void ClearTools();

    // ------------------------------------------------------------------------
    /// \name Tool management
    ///@{

    /// Returns true if the named general tool can be used for the given
    /// selection.
    bool CanUseGeneralTool(const std::string &name, const Selection &sel);

    /// Returns true if a specialized tool can be used for the given selection.
    bool CanUseSpecializedTool(const Selection &sel);

    /// Returns the specialized tool that can be used for the given selection,
    /// if any.
    SpecializedToolPtr GetSpecializedToolForSelection(const Selection &sel);

    /// Switches to using the named general tool for the selection.
    void UseGeneralTool(const std::string &name, const Selection &sel);

    /// Switches to using a specialized tool if possible and necessary for the
    /// given selection.
    void UseSpecializedTool(const Selection &sel);

    /// Returns true if currently using a specialized tool.
    bool IsUsingSpecializedTool() const { return is_using_specialized_tool_; }

    /// Switches between using a general tool and a specialized tool. There
    /// must be an available specialized tool for the given selection.
    void ToggleSpecializedTool(const Selection &sel);

    /// Returns a vector of names of all specialized tools.
    std::vector<std::string> GetSpecializedToolNames();

    /// Returns the tool currently in use for the primary selection. This is
    /// public primarily for unit tests.
    ToolPtr GetCurrentTool() const;

    /// Attaches the proper Tools to all Models in the given Selection.
    void AttachToSelection(const Selection &sel);

    /// Detaches all tools from the given Selection.
    void DetachTools(const Selection &sel);

    /// Detaches all attached tools.
    void DetachAllTools();

    /// Reattaches all currently attached tools.
    void ReattachTools();

    /// Returns the Tool, if any, attached to a Model.
    ToolPtr GetAttachedTool(const ModelPtr &model) const;

    ///@}

    // ------------------------------------------------------------------------
    /// \name General tool switching
    ///@{

    /// Switches to the previous general tool for the given Selection.
    void UsePreviousGeneralTool(const Selection &sel) {
        UseTool_(GetPreviousOrNextGeneralTool_(sel, false), sel);
    }

    /// Switches to the next general Tool.
    void UseNextGeneralTool(const Selection &sel) {
        UseTool_(GetPreviousOrNextGeneralTool_(sel, true), sel);
    }

    ///@}

    // ------------------------------------------------------------------------
    // Grippable interface. This delegates all calls to the active Tool, if
    // any.
    // ------------------------------------------------------------------------

    virtual bool IsGrippableEnabled() const override {
        auto tool = GetCurrentTool();
        return tool ? tool->IsGrippableEnabled() : false;
    }

    virtual GripGuideType GetGripGuideType() const override {
        // This should not be called unless there is a current Tool.
        ASSERT(GetCurrentTool());
        return GetCurrentTool()->GetGripGuideType();
    }

    virtual void UpdateGripInfo(GripInfo &info) override {
        // This should not be called unless there is a current Tool.
        ASSERT(GetCurrentTool());
        GetCurrentTool()->UpdateGripInfo(info);
    }

  private:
    typedef std::unordered_map<std::string, GeneralToolPtr>     GeneralMap_;
    typedef std::unordered_map<std::string, SpecializedToolPtr> SpecializedMap_;
    typedef std::unordered_map<Model *, ToolPtr>                ToolMap_;

    /// Node to use as the parent of all active Tools.
    SG::NodePtr parent_node_;

    /// Stores each GeneralTool in the order they were added, which determines
    /// how tool switching works.
    std::vector<GeneralToolPtr> general_tools_;

    /// For each type of GeneralTool, this maps the name of the tool to an
    /// instance of it.
    GeneralMap_ general_tool_map_;

    /// For each type of SpecializedTool, this maps the name of the tool to an
    /// instance of it..
    SpecializedMap_ specialized_tool_map_;

    /// Stores the current general tool.
    GeneralToolPtr current_general_tool_;

    /// Stores the default general tool to use when starting a new session.
    GeneralToolPtr default_general_tool_;

    /// Allows access to the Tool actively attached to a Model.
    ToolMap_ tool_map_;

    /// Stores the current specialized tool, or null when using a general tool.
    SpecializedToolPtr current_specialized_tool_;

    /// This is used to toggle between general and specialized tools.
    bool is_using_specialized_tool_ = false;

    /// Set to true while a Tool is actively dragging.
    bool is_tool_dragging_ = false;

    /// Manages PassiveTool instances.
    std::shared_ptr<InstanceManager> passive_tool_manager_;

    /// Attaches the given tool to the primary selection and sets it up.
    void UseTool_(const ToolPtr &tool, const Selection &sel);

    /// Attaches the Tool to the indexed selected Model. The selection is
    /// passed to the Tool in case it operates on multiple Models.
    void AttachToolToModel_(const ToolPtr &tool,
                            const Selection &sel, size_t index);

    /// Detaches the attached Tool (if any) from the given Model.
    void DetachToolFromModel_(Model &model);

    /// Returns the previous or next general tool (relative to the current one)
    /// that can be used for the given Selection. Should never return null.
    GeneralToolPtr GetPreviousOrNextGeneralTool_(const Selection &sel,
                                                 bool is_next);

    /// Callback that is invoked when a Model with a Tool changes. If the Model
    /// bounds changed, this reattaches its Tool.
    void ModelChanged_(const ModelPtr &model, SG::Change change);

    /// Callback that is invoked when a drag on part of a Tool begins. It hides
    /// the PassiveTools on the secondary selections, since they may not be
    /// updated.
    void ToolDragStarted_(Tool &dragged_tool);

    /// Callback that is invoked when a drag on part of a Tool ends. It shows
    /// the PassiveTools on the secondary selections.
    void ToolDragEnded_(Tool &dragged_tool);

    /// Target activation/deactivation callback that hides all active tools
    /// while the target is active so the tool geometry is not detected during
    /// target placement.
    void TargetActivated_(bool is_activation);
};

typedef std::shared_ptr<ToolManager> ToolManagerPtr;
