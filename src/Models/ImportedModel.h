﻿#pragma once

#include <string>

#include "Base/Memory.h"
#include "Models/Model.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(ImportedModel);
DECL_SHARED_PTR(UnitConversion);

/// ImportedModel represents a Model imported from an STL file. The path field
/// stores the path to the STL file. If this path is empty, invalid, or refers
/// to an invalid STL file, the ImportedModel shows a placeholder Mesh
/// (tetrahedron) and marks it as invalid.
///
/// \ingroup Models
class ImportedModel : public Model {
  public:
    /// Sets the UnitConversion to use when importing STL data.
    void SetUnitConversion(const UnitConversion &conv);

    /// Sets the path to the STL file as a string. This will cause the STL data
    /// to be imported to create the Mesh.
    void SetPath(const std::string &path);

    /// Returns the path to the STL file as a string.
    const std::string & GetPath() const { return path_; }

  protected:
    ImportedModel();
    virtual void AddFields() override;
    virtual TriMesh BuildMesh() override;

    /// Redefines this to add import-specific reasons.
    virtual bool ValidateMesh(std::string &reason) const override;

  private:
    /// \name Parsed fields.
    ///@{
    Parser::TField<std::string> path_;
    ///@}

    /// UnitConversion used for converting imported STL data.
    UnitConversionPtr unit_conversion_;

    /// When the ImportedModel is invalid, this stores the reason.
    std::string import_error_;

    friend class Parser::Registry;
};
