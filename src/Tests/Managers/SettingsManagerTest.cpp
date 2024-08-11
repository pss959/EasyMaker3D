//@@@@
// SPDX-FileCopyrightText:  2021-2024 Paul S. Strauss
// SPDX-License-Identifier: AGPL-3.0-or-later
//@@@@

#include "Items/Settings.h"
#include "Managers/SettingsManager.h"
#include "Tests/TempFile.h"
#include "Tests/TestBaseWithTypes.h"
#include "Tests/Testing.h"

// ----------------------------------------------------------------------------
// SettingsManagerTest class.
// ----------------------------------------------------------------------------

/// \ingroup Tests
class SettingsManagerTest : public TestBaseWithTypes {
  protected:
    /// Creates a Settings instance with specific values.
    SettingsPtr BuildSettings() const;

    /// Convenience to test Settings against expected values.
    void TestSettings(const Settings &exp_s, const Settings &s);

    /// Convenience to compare two UnitConversion instances.
    static bool CompareUC(const UnitConversion &exp_uc,
                          const UnitConversion &uc);
    /// Convenience to compare two RadialMenuInfo instances.
    static bool CompareRMI(const RadialMenuInfo &exp_rmi,
                           const RadialMenuInfo &rmi);
};

SettingsPtr SettingsManagerTest::BuildSettings() const {
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

    return settings;
}

void SettingsManagerTest::TestSettings(const Settings &exp_s,
                                       const Settings &s) {
    EXPECT_EQ(exp_s.GetLastSessionPath(),  s.GetLastSessionPath());
    EXPECT_EQ(exp_s.GetSessionDirectory(), s.GetSessionDirectory());
    EXPECT_EQ(exp_s.GetImportDirectory() , s.GetImportDirectory());
    EXPECT_EQ(exp_s.GetExportDirectory(),  s.GetExportDirectory());
    EXPECT_EQ(exp_s.GetTooltipDelay(),     s.GetTooltipDelay());
    EXPECT_PRED2(CompareUC, exp_s.GetImportUnitsConversion(),
                 s.GetImportUnitsConversion());
    EXPECT_PRED2(CompareUC, exp_s.GetExportUnitsConversion(),
                 s.GetExportUnitsConversion());
    EXPECT_EQ(exp_s.GetBuildVolumeSize(), s.GetBuildVolumeSize());
    EXPECT_PRED2(CompareRMI, exp_s.GetLeftRadialMenuInfo(),
                 s.GetLeftRadialMenuInfo());
    EXPECT_PRED2(CompareRMI, exp_s.GetRightRadialMenuInfo(),
                 s.GetRightRadialMenuInfo());
}

bool SettingsManagerTest::CompareUC(const UnitConversion &exp_uc,
                                    const UnitConversion &uc) {
    return uc.GetFromUnits() == exp_uc.GetFromUnits() &&
        uc.GetToUnits() == exp_uc.GetToUnits();
}

bool SettingsManagerTest::CompareRMI(const RadialMenuInfo &exp_rmi,
                                     const RadialMenuInfo &rmi) {
    if (rmi.GetCount() != exp_rmi.GetCount())
        return false;
    for (size_t i = 0; i < RadialMenuInfo::kMaxCount; ++i)
        if (rmi.GetButtonAction(i) != exp_rmi.GetButtonAction(i))
            return false;
    return true;
}

// ----------------------------------------------------------------------------
// SettingsManagerTest tests.
// ----------------------------------------------------------------------------

TEST_F(SettingsManagerTest, Defaults) {
    // Default SettingsManager instance should have default settings.
    SettingsManager sm;
    TestSettings(*Settings::CreateDefault(), sm.GetSettings());
}

TEST_F(SettingsManagerTest, Load) {
    // Create a temporary file with known settings.
    auto settings = BuildSettings();
    TempFile tmp(*settings);

    // Need a real FileSystem for this.
    UseRealFileSystem(true);

    // Load the temporary file.
    SettingsManager sm;
    EXPECT_TRUE(sm.SetPath(tmp.GetPath(), false));  // No save on set.
    EXPECT_TRUE(sm.GetLoadError().empty());
    EXPECT_EQ(tmp.GetPath(), sm.GetPath());
    TestSettings(*settings, sm.GetSettings());
}

TEST_F(SettingsManagerTest, LoadFail) {
    // Need a real FileSystem for this.
    UseRealFileSystem(true);

    SettingsManager sm;

    // Nonexistent file does not result in an error, but does not succeed.
    EXPECT_FALSE(sm.SetPath("/no/such/path", false));
    EXPECT_TRUE(sm.GetLoadError().empty());

    // Parse error.
    {
        TempFile tmp("Blah blah blah");
        EXPECT_FALSE(sm.SetPath(tmp.GetPath(), false));
        EXPECT_TRUE(sm.GetLoadError().contains("Blah"));
    }

    // Wrong type of object.
    {
        TempFile tmp("UnitConversion {}");
        EXPECT_FALSE(sm.SetPath(tmp.GetPath(), false));
        EXPECT_TRUE(sm.GetLoadError().contains("Got UnitConversion"));
    }
}

TEST_F(SettingsManagerTest, Set) {
    auto def_settings  = Settings::CreateDefault();

    // Set up a change function to update func_settings.
    auto func_settings = def_settings;
    auto change_func = [&](const Settings &s){ func_settings->CopyFrom(s); };

    SettingsManager sm;
    sm.SetChangeFunc(change_func);

    // Default Settings before being updated by the change function.
    TestSettings(*def_settings, *func_settings);

    auto settings = BuildSettings();
    sm.SetSettings(*settings);
    TestSettings(*settings, sm.GetSettings());

    // Change function should have been invoked.
    TestSettings(*settings, *func_settings);
}

TEST_F(SettingsManagerTest, Save) {
    // Need a real FileSystem for this.
    UseRealFileSystem(true);

    // Create a temporary file with default settings.
    auto def_settings = Settings::CreateDefault();
    TempFile tmp1(*def_settings);

    // Load the temporary file and pass true for save_on_set.
    SettingsManager sm;
    Str error;
    EXPECT_TRUE(sm.SetPath(tmp1.GetPath(), true));
    EXPECT_TRUE(sm.GetLoadError().empty());
    TestSettings(*def_settings, sm.GetSettings());

    // Set to new known settings.
    auto settings = BuildSettings();
    sm.SetSettings(*settings);
    TestSettings(*settings, sm.GetSettings());

    // The temporary file should now contain the new settings.
    TempFile tmp2(tmp1.GetContents());
    EXPECT_TRUE(sm.SetPath(tmp2.GetPath(), false));
    EXPECT_TRUE(sm.GetLoadError().empty());
    TestSettings(*settings, sm.GetSettings());
}
