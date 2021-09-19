#pragma once

#include <memory>
#include <vector>

#include "Models/Model.h"

//! ClipboardManager manages the clipboard, which is essentially a collection
//! of Models that have been cut or copied.
//!
//! \ingroup Managers
class ClipboardManager {
  public:
    //! Clears the clipboard.
    void Reset() { models_.clear(); }

    //! Copies the given Models to the clipboard, erasing whatever had been in
    //! there.
    void StoreCopies(const std::vector<ModelPtr> &models);

    //! Returns the current Models in the clipboard.
    const std::vector<ModelPtr> & Get() { return models_; }

    //! Returns a vector of clones of all Models currently in the clipboard.
    //! The vector may be empty.
    std::vector<ModelPtr> CreateClones() const { return CloneModels_(models_); }

  private:
    typedef std::vector<ModelPtr> ModelVec_;  //!< For shorthand.

    ModelVec_ models_;  //!< Models stored in clipboard.

    //! Returns a vector containing clones of the given Models.
    static ModelVec_ CloneModels_(const ModelVec_ &models);
};
