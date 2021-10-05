#pragma once

#include <memory>

#include "Math/Types.h"
#include "Models/Model.h"
#include "Parser/Registry.h"
#include "SG/Node.h"
#include "Selection.h"
#include "Util/Notifier.h"

class CommandManager;

/// Tool is a derived SG::Node class that serves as an abstract base class for
/// all interactive tools that can be attached to Models in the scene.
///
/// \ingroup Tools
class Tool : public SG::Node { /* : public IGrippable XXXX */
  public:
    /// The Context is provided by the ToolManager. It allows Tool classes to
    /// access the available manager instances and other info during their
    /// operation.
    struct Context {
        std::shared_ptr<CommandManager> command_manager;
        //FeedbackManager &feedback_manager;
        //TargetManager   &target_manager;
        //ModelManager    &model_manager;
        //PanelManager    &panel_manager;

        /// Flag indicating how to transform Models.
        bool            is_axis_aligned = false;

        /// Flag indicating whether the app is in alternate input mode.
        bool            is_alternate_mode = false;

        /// GuiBoard to use for tools that need GUI interaction.
        //GuiBoard        board;
    };

    /// Sets a Context that can be used by derived Tool classes during their
    /// operation.
    void SetContext(std::shared_ptr<Context> &context);

    // ------------------------------------------------------------------------
    // Public interface.
    // ------------------------------------------------------------------------

    /// Returns a Notifier that is invoked when a drag on some part of the Tool
    /// has started. An observer is passed the Tool that is dragging.
    Util::Notifier<Tool&> & GetDragStarted() { return drag_started_; }

    /// Returns a Notifier that is invoked when a drag on some part of the Tool
    /// has ended. An observer is passed the Tool that is dragging.
    Util::Notifier<Tool&> & GetDragEnded() { return drag_ended_; }

    /// Returns true if the Tool can be used for the given Selection.
    bool CanBeUsedFor(const Selection &sel) const;

    /// Attaches the tool to the selection. This assumes that CanBeUsedFor()
    /// returned true and the Tool is not already attached.
    void AttachToSelection(const Selection &sel);

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

    /// Returns the primary Model the Tool is attached to, which will be null
    /// if it is not attached.
    ModelPtr GetPrimaryModel() const;

#if XXXX
    // ------------------------------------------------------------------------
    // IGrippable interface.
    // ------------------------------------------------------------------------

    /// Defines this to return true if the tool is attached to a Model.
    virtual bool IsGrippableEnabled() override { return GetPrimaryModel(); }

    virtual void UpdateGripHoverData(GripData &data) override {
        data.go = null;
    }

    /// The base class defines this to return GripGuide::GuideType::kNone.
    virtual GripGuide::GuideType GetGripGuideType() const override {
        return GripGuide::GuideType::kNone;
    }
#endif

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

    /// Creates a Command of the templated and named type.
    template <typename T> std::shared_ptr<T> CreateCommand(
        const std::string &type_name) {
        return Parser::Registry::CreateObject<T>(type_name);
    }

    /// Returns the matrix converting from local coordinates (the end of the
    /// primary selection SelPath) to stage coordinates (at the root of the
    /// path). If is_inclusive is true, this includes local transformations at
    /// the tail of the path.
    Matrix4f GetLocalToStageMatrix(bool is_inclusive) const;

  private:
    std::shared_ptr<Context> context_;

    /// Current Selection. Empty except for the Tool attached to the primary
    /// selection.
    Selection                selection_;

    Util::Notifier<Tool &>   drag_started_;  ///< Notifies when a drag starts.
    Util::Notifier<Tool &>   drag_ended_;    ///< Notifies when a drag ends.
};

typedef std::shared_ptr<Tool> ToolPtr;
