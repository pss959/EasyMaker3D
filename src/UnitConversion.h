#pragma once

#include "Base/Memory.h"
#include "Parser/Object.h"

namespace Parser { class Registry; }

DECL_SHARED_PTR(UnitConversion);

/// The UnitConversion class defines a conversion between units during import
/// or export of models. It is derived from Parser::Object so it can be read
/// from and written to files.
class UnitConversion : public Parser::Object {
  public:
    /// Supported units to convert among.
    enum class Units {
        // Metric.
        kCentimeters,     /// Default units.
        kMillimeters,
        kMeters,

        // Imperial.
        kInches,
        kFeet,
    };

    /// Creates an instance with default values.
    static UnitConversionPtr CreateDefault() {
        return CreateWithUnits(Units::kCentimeters, Units::kCentimeters);
    }

    /// Creates an instance with the given units
    static UnitConversionPtr CreateWithUnits(Units from_units, Units to_units);

    Units GetFromUnits() const      { return from_units_; }
    Units GetToUnits()   const      { return to_units_; }
    void  SetFromUnits(Units units) { from_units_ = units; }
    void  SetToUnits(Units units)   { to_units_ = units; }

    /// Returns the conversion factor from centimeters (the default units) to
    // the given units. Multiply to convert to the other unit; divide to
    // convert from the other unit.
    static float GetConversionFactor(Units target_units);

    /// Returns a factor to multiply by to convert from from_units to to_units.
    float GetFactor() const;

    /// Copies values from another instance.
    void CopyFrom(const UnitConversion &from) { CopyContentsFrom(from, false); }

  protected:
    UnitConversion() {}

    virtual void AddFields() override;

  private:
    /// \name Parsed Fields
    ///@{
    Parser::EnumField<Units> from_units_{"from_units", Units::kCentimeters};
    Parser::EnumField<Units>   to_units_{"to_units", Units::kCentimeters};
    ///@}

    friend class Parser::Registry;
};
