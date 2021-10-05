#pragma once

#include <memory>
#include <string>
#include <vector>

#include "Commands/Command.h"
#include "Util/General.h"

/// MultiModelCommand is an abstract base class for command classes that
/// operate on multiple Models. It stores the names of all of the Models.
///
/// \ingroup Commands
class MultiModelCommand : public Command {
  public:
    virtual void AddFields() override;
    virtual bool IsValid(std::string &details) override;

    /// Sets the command to operate on the given vector of Models of some type.
    template <typename Model> void SetModels(
        const std::vector<std::shared_ptr<Model>> &models) {
        model_names_ = Util::ConvertVector<std::string, ModelPtr>(
            models, [](const ModelPtr &model){ return model->GetName(); });
    }

    /// Returns the names of the operand Models.
    const std::vector<std::string> & GetModelNames() const {
        return model_names_;
    }

  private:
    /// \name Parsed Fields
    ///@{
    Parser::VField<std::string> model_names_{"model_names"};
    ///@}
};
