#include "Enums/CSGOperation.h"
#include "Models/CombinedModel.h"

/// CSGModel is a derived CombinedModel class that represents a constructive
/// solid geometry (CSG) operation applied to two or more operand Models.
///
/// \ingroup Models
class CSGModel : public CombinedModel {
  public:
    virtual void AddFields() override;
    virtual bool IsValid(std::string &details) override;

    /// Sets the CSG operation.
    void SetOperation(CSGOperation op);

    /// Returns the CSG operation.
    CSGOperation GetOperation() const { return operation_; }

  protected:
    virtual TriMesh BuildMesh() override;

  private:
    /// \name Parsed fields.
    ///@{
    Parser::EnumField<CSGOperation> operation_{"operation",
                                               CSGOperation::kUnion};
    ///@}
};

typedef std::shared_ptr<CSGModel> CSGModelPtr;
