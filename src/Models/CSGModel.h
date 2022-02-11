#pragma once

#include "Enums/CSGOperation.h"
#include "Models/CombinedModel.h"

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
    virtual bool IsValid(std::string &details) override;
    virtual TriMesh BuildMesh() override;

  private:
    /// \name Parsed fields.
    ///@{
    Parser::EnumField<CSGOperation> operation_{"operation",
                                               CSGOperation::kUnion};
    ///@}

    friend class Parser::Registry;
};

typedef std::shared_ptr<CSGModel> CSGModelPtr;
