#pragma once

#include "Models/ParentModel.h"
#include "Util/Notifier.h"

namespace Parser { class Registry; }

/// RootModel is a derived ParentModel that is used only as the root containing
/// all Models in the scene. It does not supply a mesh because it is not
/// rendered directly.
///
/// \ingroup Models
class RootModel : public ParentModel {
  public:
    virtual TriMesh BuildMesh() { return TriMesh(); }

    /// Returns a Notifier that is invoked when any of the following changes is
    /// made to a top-level Model (one of the child Models of the RootModel):
    /// adding a Model, removing a Model, hiding a Model, showing a Model.
    Util::Notifier<> & GetTopLevelChanged() { return top_level_changed_; }

  protected:
    RootModel() {}

  private:
    /// Notifies when a change is made to a top-level Model.
    Util::Notifier<> top_level_changed_;

    friend class Parser::Registry;
};

typedef std::shared_ptr<RootModel> RootModelPtr;
