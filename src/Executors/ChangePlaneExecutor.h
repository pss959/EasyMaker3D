//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

#include <vector>

#include "Executors/Executor.h"
#include "Math/Plane.h"
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
    /// This struct contains everything needed for a derived class to implement
    /// UpdateModel().
    struct PlaneData {
        const SG::CoordConv &cc;        ///< For stage/object conversion.
        Plane                old_object_plane;
        Plane                new_stage_plane;
        Vector3f             old_translation;
        explicit PlaneData(const SG::CoordConv &cc_) : cc(cc_) {}
    };

    /// Derived classes must implement this to return the current Plane from
    /// the given Model.
    virtual Plane GetModelPlane(const Model &model) const = 0;

    /// Derived classes must implement this to update the given Model based on
    /// the given PlaneData.
    virtual void UpdateModel(Model &model, const PlaneData &data) const = 0;

    /// Derived classes must implement this to set the current Plane in the
    /// given Model.
    virtual void SetModelPlane(Model &model, const Plane &plane) const = 0;

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
