#pragma once

//! Supported units. Conversion is available for import and export.
enum class Units {
    // Metric.
    kCentimeters,     //! Default units.
    kMillimeters,
    kMeters,

    // Imperial.
    kInches,
    kFeet,
};
