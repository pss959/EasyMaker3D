//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Items/UnitConversion.h"

#include "Parser/Registry.h"

void UnitConversion::AddFields() {
    AddField(from_units_.Init("from_units", Units::kCentimeters));
    AddField(to_units_.Init("to_units",     Units::kCentimeters));

    Parser::Object::AddFields();
}

float UnitConversion::GetConversionFactor(Units target_units) {
    switch (target_units) {
      case Units::kCentimeters:   return 1.f;
      case Units::kMillimeters:   return 10.f;
      case Units::kMeters:        return .01f;
      case Units::kInches:        return .393701f;
      case Units::kFeet:          return .0328084f;
      default:                    return 1.f;  // LCOV_EXCL_LINE [cannot happen]
    }
}

UnitConversionPtr UnitConversion::CreateWithUnits(Units from_units,
                                                  Units to_units) {
    auto conv =  Parser::Registry::CreateObject<UnitConversion>();
    conv->SetFromUnits(from_units);
    conv->SetToUnits(to_units);
    return conv;
}

float UnitConversion::GetFactor() const {
    return GetConversionFactor(to_units_) / GetConversionFactor(from_units_);
}
