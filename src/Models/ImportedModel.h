//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#pragma once

#include "Models/PrimitiveModel.h"
#include "Util/Memory.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(ImportedModel);
DECL_SHARED_PTR(UnitConversion);

/// ImportedModel represents a Model imported from an STL file. The path field
/// stores the path to the STL file. If this path is empty, invalid, or refers
/// to an invalid STL file, the ImportedModel shows a placeholder Mesh
/// (tetrahedron) and marks it as invalid.
///
/// \ingroup Models
class ImportedModel : public PrimitiveModel {
  public:
    /// Sets the UnitConversion to use when importing STL data.
    void SetUnitConversion(const UnitConversion &conv);

    /// Returns the UnitConversion used when importing STL data.
    const UnitConversion & GetUnitConversion() const {
        return *unit_conversion_;
    }

    /// Sets the path to the STL file as a string. This will cause the STL data
    /// to be imported to create the Mesh.
    void SetPath(const Str &path);

    /// Returns the path to the STL file as a string.
    const Str & GetPath() const { return path_; }

    /// Returns true if STL data was ever successfully loaded. This is used to
    /// tell whether to initialize the ImportedModel's position.
    bool WasLoadedSuccessfully() const { return was_loaded_successfully_; }

    /// Returns the error message resulting from importing the STL data from
    /// the current path. This will be empty if the import was successful.
    const Str & GetErrorMessage() const { return import_error_; }

  protected:
    ImportedModel();
    virtual void AddFields() override;
    virtual TriMesh BuildMesh() override;

    /// Redefines this to add import-specific reasons.
    virtual bool ValidateMesh(TriMesh &mesh, Str &reason) override;

  private:
    /// \name Parsed fields.
    ///@{
    Parser::TField<Str> path_;
    ///@}

    /// UnitConversion used for converting imported STL data.
    UnitConversionPtr unit_conversion_;

    /// Set to true if STL data is ever loaded successfully.
    bool was_loaded_successfully_ = false;

    /// When the ImportedModel is invalid, this stores the reason.
    Str import_error_;

    friend class Parser::Registry;
};
