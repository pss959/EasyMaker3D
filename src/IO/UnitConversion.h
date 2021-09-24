#include "Defaults.h"

/// A UnitConversion struct defines a conversion between units during import or
/// export of models.
struct UnitConversion {
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

    Units from_units = Units::kCentimeters;  ///< Units converting from.
    Units to_units   = Units::kCentimeters;  ///< Units converting to.

    /// Returns the conversion factor from centimeters (the default units) to
    // the given units. Multiply to convert to the other unit; divide to
    // convert from the other unit.
    static float GetConversionFactor(Units target_units) {
        switch (target_units) {
          case Units::kCentimeters:   return 1.f;
          case Units::kMillimeters:   return 10.f;
          case Units::kMeters:        return .01f;
          case Units::kInches:        return .393701f;
          case Units::kFeet:          return .0328084f;
          default:                    return 1.f;
        }
    }

    /// Returns a factor to multiply by to convert from from_units to to_units.
    float GetFactor() const {
        return GetConversionFactor(to_units) / GetConversionFactor(from_units);
    }
};
