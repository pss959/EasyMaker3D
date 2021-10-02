#pragma once

#include <memory>

#include "Math/Types.h"
#include "Models/Model.h"
#include "Selection.h"
#include "Util/Notifier.h"

class CommandManager;

/// Tool is a derived SG::Node class that serves as an abstract base class for
/// all interactive tools that can be attached to Models in the scene.
///
/// \ingroup Tools
class Tool /* : public IGrippable */ {
  public:
    /// The Context is provided by the ToolManager. It allows Tool classes to
    /// access the available manager instances and other info during their
    /// operation.
    struct Context {
        CommandManager  &command_manager;
        //FeedbackManager &feedback_manager;
        //TargetManager   &target_manager;
        //ModelManager    &model_manager;
        //PanelManager    &panel_manager;

        /// Flag indicating how to transform Models.
        bool            is_axis_aligned;

        /// Flag indicating whether the app is in alternate input mode.
        bool            is_alternate_mode;

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

    /// Attaches the tool to the given Model. This should not be called if
    /// CanBeUsedFor() returned false or if the Tool is already attached to a
    /// Model. The Selection is passed in in case the Tool needs to operate on
    /// multiple Models.
    void AttachToModel(const ModelPtr &model, const Selection &sel);

    /// Detaches from the Model the Tool is currently attached to. This should
    /// not be called if the Tool is not attached.
    void DetachFromModel();

    /// Reattaches the Tool to the current Model. The base class defines this
    /// to detach and attach again. Derived classes may choose not to reattach
    /// if in the middle of something.
    virtual void ReattachToModel();

    /// Returns the Model the Tool is attached to, which may be null.
    ModelPtr GetModel() const { return model_; }

#if XXXX
    // ------------------------------------------------------------------------
    // IGrippable interface.
    // ------------------------------------------------------------------------

    /// Defines this to return true if the tool is attached to a Model.
    virtual bool IsGrippableEnabled() override { return GetModel(); }

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

    /// Attaches to the given Model.
    virtual void Attach(const ModelPtr &model) = 0;

    /// Detaches from the Model it is attached to.
    virtual void Detach() = 0;

    // ------------------------------------------------------------------------
    // Helper functions for derived classes.
    // ------------------------------------------------------------------------

    /// Returns the Selection passed to SetSelection() containing all selected
    /// Models. This will be an empty Selection except for the Tool attached to
    /// the primary selection.
    const Selection & GetSelection() const { return selection_; }

  private:
    std::shared_ptr<Context> context_;
    ModelPtr                 model_;

    /// Current Selection. Empty except for the Tool attached to the primary
    /// selection.
    Selection                selection_;

    Util::Notifier<Tool &>   drag_started_;  ///< Notifies when a drag starts.
    Util::Notifier<Tool &>   drag_ended_;    ///< Notifies when a drag ends.
};
