#include "Items/Settings.h"
#include "Tests/SceneTestBase.h"
#include "Tests/Testing.h"
#include "Util/Tuning.h"

/// \ingroup Tests
class SettingsTest : public SceneTestBase {};

TEST_F(SettingsTest, Default) {
    auto settings = Settings::CreateDefault();
    const Str home = FilePath::GetHomeDirPath().ToString();
    EXPECT_FALSE(settings->GetLastSessionPath());
    EXPECT_EQ(home, settings->GetSessionDirectory().ToString());
    EXPECT_EQ(home, settings->GetImportDirectory().ToString());
    EXPECT_EQ(home, settings->GetExportDirectory().ToString());
    EXPECT_EQ(TK::kTooltipDelay, settings->GetTooltipDelay());
    EXPECT_EQ(UnitConversion::Units::kMillimeters,
              settings->GetImportUnitsConversion().GetFromUnits());
    EXPECT_EQ(UnitConversion::Units::kCentimeters,
              settings->GetImportUnitsConversion().GetToUnits());
    EXPECT_EQ(UnitConversion::Units::kCentimeters,
              settings->GetExportUnitsConversion().GetFromUnits());
    EXPECT_EQ(UnitConversion::Units::kMillimeters,
              settings->GetExportUnitsConversion().GetToUnits());
    EXPECT_EQ(TK::kBuildVolumeSize, settings->GetBuildVolumeSize());
    EXPECT_EQ(RadialMenuInfo::Count::kCount8,
              settings->GetLeftRadialMenuInfo().GetCount());
    EXPECT_EQ(RadialMenuInfo::Count::kCount8,
              settings->GetRightRadialMenuInfo().GetCount());
    EXPECT_EQ(RadialMenusMode::kIndependent, settings->GetRadialMenusMode());

    EXPECT_EQ(&settings->GetLeftRadialMenuInfo(),
              &settings->GetRadialMenuInfo(Hand::kLeft));
    EXPECT_EQ(&settings->GetRightRadialMenuInfo(),
              &settings->GetRadialMenuInfo(Hand::kRight));
}

TEST_F(SettingsTest, SetAndCopy) {
    auto settings = Settings::CreateDefault();
    settings->SetLastSessionPath("/a/b/c");
    settings->SetSessionDirectory("/d/e/f");
    settings->SetImportDirectory("/g/h/i");
    settings->SetExportDirectory("/j/k/l");
    settings->SetTooltipDelay(1.5f);
    auto imconv = UnitConversion::CreateWithUnits(
        UnitConversion::Units::kFeet, UnitConversion::Units::kInches);
    auto exconv = UnitConversion::CreateWithUnits(
        UnitConversion::Units::kInches, UnitConversion::Units::kFeet);
    settings->SetImportUnitsConversion(*imconv);
    settings->SetExportUnitsConversion(*exconv);
    settings->SetBuildVolumeSize(Vector3f(3, 4, 5));
    auto linfo = ParseTypedObject<RadialMenuInfo>(
        R"(RadialMenuInfo { count: "kCount2", actions: ["kUndo", "kRedo"] })");
    auto rinfo = ParseTypedObject<RadialMenuInfo>(
        R"(RadialMenuInfo { count: "kCount2", actions: ["kRedo", "kUndo"] })");
    settings->SetLeftRadialMenuInfo(*linfo);
    settings->SetRightRadialMenuInfo(*rinfo);
    settings->SetRadialMenusMode(RadialMenusMode::kLeftForBoth);

    EXPECT_EQ("/a/b/c", settings->GetLastSessionPath().ToString());
    EXPECT_EQ("/d/e/f", settings->GetSessionDirectory().ToString());
    EXPECT_EQ("/g/h/i", settings->GetImportDirectory().ToString());
    EXPECT_EQ("/j/k/l", settings->GetExportDirectory().ToString());
    EXPECT_EQ(1.5f, settings->GetTooltipDelay());
    EXPECT_EQ(UnitConversion::Units::kFeet,
              settings->GetImportUnitsConversion().GetFromUnits());
    EXPECT_EQ(UnitConversion::Units::kInches,
              settings->GetImportUnitsConversion().GetToUnits());
    EXPECT_EQ(UnitConversion::Units::kInches,
              settings->GetExportUnitsConversion().GetFromUnits());
    EXPECT_EQ(UnitConversion::Units::kFeet,
              settings->GetExportUnitsConversion().GetToUnits());
    EXPECT_EQ(Vector3f(3, 4, 5), settings->GetBuildVolumeSize());
    EXPECT_EQ(RadialMenuInfo::Count::kCount2,
              settings->GetLeftRadialMenuInfo().GetCount());
    EXPECT_EQ(Action::kUndo,
              settings->GetLeftRadialMenuInfo().GetButtonAction(0));
    EXPECT_EQ(Action::kRedo,
              settings->GetLeftRadialMenuInfo().GetButtonAction(1));
    EXPECT_EQ(RadialMenuInfo::Count::kCount2,
              settings->GetRightRadialMenuInfo().GetCount());
    EXPECT_EQ(Action::kRedo,
              settings->GetRightRadialMenuInfo().GetButtonAction(0));
    EXPECT_EQ(Action::kUndo,
              settings->GetRightRadialMenuInfo().GetButtonAction(1));
    EXPECT_EQ(RadialMenusMode::kLeftForBoth, settings->GetRadialMenusMode());
    EXPECT_EQ(&settings->GetLeftRadialMenuInfo(),
              &settings->GetRadialMenuInfo(Hand::kLeft));
    EXPECT_EQ(&settings->GetLeftRadialMenuInfo(),  // Using left for both.
              &settings->GetRadialMenuInfo(Hand::kRight));

    auto copy = Settings::CreateCopy(*settings);
    EXPECT_EQ("/a/b/c", copy->GetLastSessionPath().ToString());
    EXPECT_EQ("/d/e/f", copy->GetSessionDirectory().ToString());
    EXPECT_EQ("/g/h/i", copy->GetImportDirectory().ToString());
    EXPECT_EQ("/j/k/l", copy->GetExportDirectory().ToString());
    EXPECT_EQ(1.5f, copy->GetTooltipDelay());
    EXPECT_EQ(UnitConversion::Units::kFeet,
              copy->GetImportUnitsConversion().GetFromUnits());
    EXPECT_EQ(UnitConversion::Units::kInches,
              copy->GetImportUnitsConversion().GetToUnits());
    EXPECT_EQ(UnitConversion::Units::kInches,
              copy->GetExportUnitsConversion().GetFromUnits());
    EXPECT_EQ(UnitConversion::Units::kFeet,
              copy->GetExportUnitsConversion().GetToUnits());
    EXPECT_EQ(Vector3f(3, 4, 5), copy->GetBuildVolumeSize());
    EXPECT_EQ(RadialMenuInfo::Count::kCount2,
              copy->GetLeftRadialMenuInfo().GetCount());
    EXPECT_EQ(Action::kUndo,
              copy->GetLeftRadialMenuInfo().GetButtonAction(0));
    EXPECT_EQ(Action::kRedo,
              copy->GetLeftRadialMenuInfo().GetButtonAction(1));
    EXPECT_EQ(RadialMenuInfo::Count::kCount2,
              copy->GetRightRadialMenuInfo().GetCount());
    EXPECT_EQ(Action::kRedo,
              copy->GetRightRadialMenuInfo().GetButtonAction(0));
    EXPECT_EQ(Action::kUndo,
              copy->GetRightRadialMenuInfo().GetButtonAction(1));
    EXPECT_EQ(RadialMenusMode::kLeftForBoth, copy->GetRadialMenusMode());
    EXPECT_EQ(&copy->GetLeftRadialMenuInfo(),
              &copy->GetRadialMenuInfo(Hand::kLeft));
    EXPECT_EQ(&copy->GetLeftRadialMenuInfo(),  // Using left for both.
              &copy->GetRadialMenuInfo(Hand::kRight));
}
