#pragma once

#include "Executors/Executor.h"
#include "Models/Model.h"

class CreateModelCommand;

/// ModelExecutorBase is an abstract base class for Executor classes dealing
/// with Model creation. It defines some protected methods for derived classes
/// to use for creating and positioning Models.
///
/// \ingroup Executors
class ModelExecutorBase : public Executor {
  public:
    virtual void Execute(Command &command, Command::Op operation) override;

  protected:
    /// Derived classes are required to implement this to actually create the
    /// Model.
    virtual ModelPtr CreateModel(Command &command) = 0;

    /// Initializes the Transform for a Model so that it is rotated and
    /// positioned correctly. This depends on whether the CommandManager is
    /// currently validating (while loading a session), in which case the
    /// target position and direction are taken from the given
    /// CreateModelCommand. Otherwise, it uses the point target's info if it is
    /// visible and sets the target position and direction in the command.
    void InitModelTransform(Model &model, float default_scale,
                            CreateModelCommand &command);

    /// Animates dropping the given Model to its position.
    void AnimateModelPlacement(Model &model);

  private:
    /// Derived Command::ExecData class that stores everything needed to undo
    /// and redo a Command that creates a Model.
    struct ExecData_ : public Command::ExecData {
        ModelPtr model;   /// The model that was created.
    };

    /// Animates placement of the given Model.
    bool AnimateModel_(Model &model, const Point3f &end_pos, float time);

    /// Creates and stores a ExecData_ in the given command if necessary,
    /// returning it either way.
    ExecData_ & GetExecData_(Command &command);
};

