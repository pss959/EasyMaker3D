#include "UnitConversion.h"

#include "Parser/Registry.h"

void UnitConversion::AddFields() {
    AddField(from_units_);
    AddField(to_units_);
    Parser::Object::AddFields();
}

float UnitConversion::GetConversionFactor(Units target_units) {
    switch (target_units) {
      case Units::kCentimeters:   return 1.f;
      case Units::kMillimeters:   return 10.f;
      case Units::kMeters:        return .01f;
      case Units::kInches:        return .393701f;
      case Units::kFeet:          return .0328084f;
      default:                    return 1.f;
    }
}

float UnitConversion::GetFactor() const {
    return GetConversionFactor(to_units_) / GetConversionFactor(from_units_);
}
