#pragma once

#include <vector>

#include "Executors/Executor.h"
#include "Math/Types.h"
#include "SG/CoordConv.h"
#include "Selection/SelPath.h"

class Model;

/// ChangePlaneExecutor is an abstract base class for executors that execute a
/// derived ChangePlaneCommand.
///
/// \ingroup Executors
class ChangePlaneExecutor : public Executor {
  public:
    virtual void Execute(Command &command, Command::Op operation) override;

  protected:
    /// Derived classes must implement this to return the current Plane from
    /// the given Model.
    virtual Plane GetModelPlane(const Model &model) const = 0;

    /// Derived classes must implement this to change the current Plane in the
    /// given Model. If the \p cc pointer is not null, the derived class should
    /// also update the Model's translation using the coordinate conversion
    /// from stage to the Model's object coordinates. Otherwise, the Plane is
    /// assumed to already be in object coordinates.
    virtual void SetModelPlane(Model &model, const Plane &plane,
                               const SG::CoordConv *cc) const = 0;

  private:
    /// Derived Command::ExecData class that stores everything needed to undo
    /// and redo a derived ChangePlaneCommand.
    struct ExecData_ : public Command::ExecData {
        /// Data per Model to operate on.
        struct PerModel {
            SelPath  path_to_model;
            Plane    old_object_plane;
            Vector3f old_translation;
        };
        std::vector<PerModel> per_model;
    };

    /// Creates and stores a ExecData_ in the given command if necessary,
    /// returning it either way.
    ExecData_ & GetExecData_(Command &command);
};
