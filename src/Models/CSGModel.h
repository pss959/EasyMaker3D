#pragma once

#include "Enums/CSGOperation.h"
#include "Models/CombinedModel.h"
#include "Util/Memory.h"

DECL_SHARED_PTR(CSGModel);

namespace Parser { class Registry; }

/// CSGModel is a derived CombinedModel class that represents a constructive
/// solid geometry (CSG) operation applied to two or more operand Models.
///
/// \ingroup Models
class CSGModel : public CombinedModel {
  public:
    /// Sets the CSG operation.
    void SetOperation(CSGOperation op);

    /// Returns the CSG operation.
    CSGOperation GetOperation() const { return operation_; }

    virtual size_t GetMinChildCount() const { return 2; }

  protected:
    CSGModel() {}
    virtual void AddFields() override;
    virtual TriMesh CombineMeshes(const std::vector<TriMesh> &meshes) override;

  private:
    /// \name Parsed fields.
    ///@{
    Parser::EnumField<CSGOperation> operation_;
    ///@}

    friend class Parser::Registry;
};
