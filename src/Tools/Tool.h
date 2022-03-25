#pragma once

#include "Items/Grippable.h"
#include "Math/Types.h"
#include "Memory.h"
#include "Parser/Registry.h"
#include "Selection.h"
#include "SG/NodePath.h"
#include "Util/General.h"
#include "Util/Notifier.h"

DECL_SHARED_PTR(Board);
DECL_SHARED_PTR(ColorManager);
DECL_SHARED_PTR(CommandManager);
DECL_SHARED_PTR(FeedbackManager);
DECL_SHARED_PTR(Model);
DECL_SHARED_PTR(PanelManager);
DECL_SHARED_PTR(PrecisionManager);
DECL_SHARED_PTR(SettingsManager);
DECL_SHARED_PTR(TargetManager);
DECL_SHARED_PTR(Tool);

/// Tool is a derived SG::Node class that serves as an abstract base class for
/// all interactive tools that can be attached to Models in the scene.
///
/// \ingroup Tools
class Tool : public Grippable {
  public:
    /// The Context is provided by the ToolManager. It allows Tool classes to
    /// access the available manager instances and other info during their
    /// operation.
    struct Context {
        ColorManagerPtr     color_manager;
        CommandManagerPtr   command_manager;
        PrecisionManagerPtr precision_manager;
        FeedbackManagerPtr  feedback_manager;
        SettingsManagerPtr  settings_manager;
        TargetManagerPtr    target_manager;
        PanelManagerPtr     panel_manager;

        /// Board to use for Tools derived from PanelTool.
        BoardPtr            board;

        /// Flag indicating how to transform Models.
        bool                is_axis_aligned = false;

        /// Flag indicating whether the app is in alternate input mode.
        bool                is_alternate_mode = false;

        /// Path to the parent node of the Tool; this is used for coordinate
        /// conversions.
        SG::NodePath        path_to_parent_node;
    };
    typedef std::shared_ptr<Context> ContextPtr;

    /// Sets a Context that can be used by derived Tool classes during their
    /// operation.
    void SetContext(const ContextPtr &context);

    // ------------------------------------------------------------------------
    // Public interface.
    // ------------------------------------------------------------------------

    /// Returns a Notifier that is invoked when a drag on some part of the Tool
    /// has started. An observer is passed the Tool that is dragging.
    Util::Notifier<Tool &> & GetDragStarted() { return drag_started_; }

    /// Returns a Notifier that is invoked when a drag on some part of the Tool
    /// has ended. An observer is passed the Tool that is dragging.
    Util::Notifier<Tool &> & GetDragEnded() { return drag_ended_; }

    /// Returns true if the Tool is specialized for a specific type of
    /// Model. The base class defines this to return false.
    virtual bool IsSpecialized() const { return false; }

    /// Returns true if the Tool can be used for the given Selection.
    bool CanBeUsedFor(const Selection &sel) const;

    /// Attaches the tool to the selection. This assumes that CanBeUsedFor()
    /// returned true and the Tool is not already attached. The index will be 0
    /// except for a PassiveTool (which can be attached to a secondary
    /// selection).
    void AttachToSelection(const Selection &sel, size_t index);

    /// Detaches from the current selection. This should not be called if the
    /// Tool is not attached.
    void DetachFromSelection();

    /// Reattaches the Tool to the current selection. The base class defines
    /// this to detach and attach again. Derived classes may choose not to
    /// reattach if in the middle of something.
    virtual void ReattachToSelection();

    /// Returns the Selection the Tool is attached to, which will be empty if
    /// it is not attached.
    const Selection GetSelection() const { return selection_; }

    /// Returns the Model the Tool is attached to, which will be null if it is
    /// not attached.
    ModelPtr GetModelAttachedTo() const;

    // ------------------------------------------------------------------------
    // Grippable interface.
    // ------------------------------------------------------------------------
    virtual bool IsGrippableEnabled() const override {
        return GetModelAttachedTo().get();
    }
    /// The base Tool class defines this to return GripGuideType::kNone.
    virtual GripGuideType GetGripGuideType() const override {
        return GripGuideType::kNone;
    }
    /// The base Tool class defines this to set the Widget in the info to null.
    virtual void UpdateGripInfo(GripInfo &info) override {
        info.widget.reset();
    }

  protected:
    /// Allows derived tool classes to access the Context.
    Context & GetContext() const;

    // ------------------------------------------------------------------------
    // Derived classes must implement these methods.
    // ------------------------------------------------------------------------

    /// Returns true if the Tool can be attached to the given Selection, which
    /// is known to have at least one Model.
    virtual bool CanAttach(const Selection &sel) const = 0;

    /// Attaches to the selection returned by GetSelection(). This will be
    /// called ony for a valid selection for this Tool.
    virtual void Attach() = 0;

    /// Detaches from the selection returned by GetSelection(). This will be
    /// called only if the Tool is attached to the selection.
    virtual void Detach() = 0;

    // ------------------------------------------------------------------------
    // Helper functions for derived classes.
    // ------------------------------------------------------------------------

    /// This can help implement the CanAttach() function by requiring that
    /// every selected Model is derived from the templated type.
    template <typename T>
    bool AreSelectedModelsOfType(const Selection &sel) const {
        for (auto &sel_path: sel.GetPaths())
            if (! Util::CastToDerived<T>(sel_path.GetModel()))
                return false;
        return true;
    }

    /// Creates a Command of the templated and named type.
    template <typename T> std::shared_ptr<T> CreateCommand() {
        return Parser::Registry::CreateObject<T>();
    }

    /// Returns a CoordConv instance for converting between object, local, and
    /// stage coordinates (the root coordinates for the CoordConv) for the
    /// attached Model.
    CoordConv GetStageCoordConv() const;

    /// Converts a point to world coordinates from the local coordinates of the
    /// given Node, which must be found somewhere under the Tool.
    Point3f ToWorld(const SG::NodePtr &local_node, const Point3f &p) const;

    /// Translates the Tool to place its center at the center of the attached
    /// Model's bounds in stage coordinates. If allow_axis_aligned is true and
    /// the is_axis_aligned flag in the Context is set, the Tool's rotation is
    /// set to identity to align with the stage coordinate axes. Otherwise, the
    /// Tool is rotated to match the rotation of the Model in stage
    /// coordinates. Returns the size of the Model's bounds (oriented properly)
    /// in stage coordinates.
    Vector3f MatchModelAndGetSize(bool allow_axis_aligned);

    /// Returns a translation vector that puts the Tool at the given distance
    /// above the top center of the attached Model.
    Vector3f GetPositionAboveModel(float distance) const;

    /// Returns the color to use for snapped feedback.
    static Color GetSnappedFeedbackColor();

    /// Returns the color to use for feedback in the given dimension. If
    /// is_snapped is true, it uses the active target material color.
    /// Otherwise, it uses the regular color for that dimension.
    static Color GetFeedbackColor(int dim, bool is_snapped);

  private:
    ContextPtr context_;

    /// Current Selection. Empty except for the Tool attached to the primary
    /// selection.
    Selection                selection_;

    /// Index into the Selection of the path for the Model this Tool is
    /// attached to. This is -1 when the Tool is not attached. If not -1, it
    /// will always be 0 (the primary selection) except for a PassiveTool,
    /// which is the only Tool that can be attached to a secondary selection.
    int                      model_sel_index_ = -1;

    Util::Notifier<Tool &>   drag_started_;  ///< Notifies when a drag starts.
    Util::Notifier<Tool &>   drag_ended_;    ///< Notifies when a drag ends.
};
