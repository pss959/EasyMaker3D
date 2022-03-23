#pragma once

#include "Models/Model.h"

namespace Parser { class Registry; }

/// ImportedModel represents a Model imported from an STL file.
///
/// \ingroup Models
class ImportedModel : public Model {
  public:
    /// Sets the path to the STL file as a string. This will cause the STL data
    /// to be imported.
    void SetPath(const std::string &path);

    /// Returns the path to the STL file as a string.
    const std::string & GetPath() const { return path_; }

  protected:
    ImportedModel() {}
    virtual TriMesh BuildMesh() override;

  private:
    /// \name Parsed fields.
    ///@{
    Parser::TField<std::string> path_{"path"};
    ///@}

    friend class Parser::Registry;
};

typedef std::shared_ptr<ImportedModel> ImportedModelPtr;
