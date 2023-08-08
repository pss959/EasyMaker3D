#include "Base/Event.h"
#include "Tests/TestBase.h"
#include "Tests/Testing.h"

/// \ingroup Tests
class EventTest : public TestBase {
  protected:
    /// Convenience to build Event::Modifiers flags from a string containing
    /// character codes.
    Event::Modifiers GetMods(const std::string &mod_str) {
        Event::Modifiers mods;
        for (const char c: mod_str) {
            if      (c == 'S') mods.Set(Event::ModifierKey::kShift);
            else if (c == 'C') mods.Set(Event::ModifierKey::kControl);
            else if (c == 'A') mods.Set(Event::ModifierKey::kAlt);
            else {
                EXPECT_TRUE(false) << "Bad modifier char";
            }
        }
        return mods;
    }
};

TEST_F(EventTest, Defaults) {
    Event ev;
    EXPECT_ENUM_EQ(Event::Device::kUnknown, ev.device);
    EXPECT_FALSE(ev.flags.HasAny());
    EXPECT_ENUM_EQ(Event::Button::kNone, ev.button);
    EXPECT_TRUE(ev.GetKeyString().empty());
    EXPECT_TRUE(ev.GetControllerButtonString().empty());
    EXPECT_EQ(0, ev.position1D);
    EXPECT_EQ(Point2f::Zero(), ev.position2D);
    EXPECT_EQ(Point3f::Zero(), ev.position3D);
    EXPECT_TRUE(ev.orientation.IsIdentity());
}

TEST_F(EventTest, IsTrackpadButton) {
    EXPECT_TRUE(Event::IsTrackpadButton(Event::Button::kCenter));
    EXPECT_TRUE(Event::IsTrackpadButton(Event::Button::kLeft));
    EXPECT_TRUE(Event::IsTrackpadButton(Event::Button::kRight));
    EXPECT_TRUE(Event::IsTrackpadButton(Event::Button::kUp));
    EXPECT_TRUE(Event::IsTrackpadButton(Event::Button::kDown));

    EXPECT_FALSE(Event::IsTrackpadButton(Event::Button::kMouse1));
    EXPECT_FALSE(Event::IsTrackpadButton(Event::Button::kMouse2));
    EXPECT_FALSE(Event::IsTrackpadButton(Event::Button::kMouse3));
    EXPECT_FALSE(Event::IsTrackpadButton(Event::Button::kPinch));
    EXPECT_FALSE(Event::IsTrackpadButton(Event::Button::kGrip));
    EXPECT_FALSE(Event::IsTrackpadButton(Event::Button::kMenu));
    EXPECT_FALSE(Event::IsTrackpadButton(Event::Button::kHeadset));
    EXPECT_FALSE(Event::IsTrackpadButton(Event::Button::kOther));
    EXPECT_FALSE(Event::IsTrackpadButton(Event::Button::kNone));
}

TEST_F(EventTest, GetKeyString) {
    // This also tests Event::BuildKeyString().

    Event ev;
    ev.device = Event::Device::kKeyboard;
    ev.flags.Set(Event::Flag::kKeyPress);

    ev.key_name = "q";
    EXPECT_EQ("q", ev.GetKeyString());
    ev.modifiers.Set(Event::ModifierKey::kControl);
    EXPECT_EQ("Ctrl-q", ev.GetKeyString());
    ev.modifiers.Set(Event::ModifierKey::kShift);
    EXPECT_EQ("Shift-Ctrl-q", ev.GetKeyString());
    ev.modifiers.Set(Event::ModifierKey::kAlt);
    EXPECT_EQ("Shift-Ctrl-Alt-q", ev.GetKeyString());
    ev.modifiers.Reset(Event::ModifierKey::kControl);
    EXPECT_EQ("Shift-Alt-q", ev.GetKeyString());

    // Try a key release with the same name and modifiers.
    ev.flags.SetAll(false);
    ev.flags.Set(Event::Flag::kKeyRelease);
    EXPECT_EQ("Shift-Alt-q", ev.GetKeyString());
}

TEST_F(EventTest, ParseKeyString) {
    Event::Modifiers mods;
    std::string      name;
    std::string      error;

    const auto test_s = [&](const std::string &s,
                            bool exp_ok,
                            const std::string &exp_mod_str,
                            const std::string &exp_name,
                            const std::string &exp_error_pattern){
        Event::Modifiers exp_mods = GetMods(exp_mod_str);
        const bool ok = Event::ParseKeyString(s, mods, name, error);
        EXPECT_EQ(exp_ok, ok);
        if (exp_ok) {
            EXPECT_EQ(exp_mods, mods);
            EXPECT_EQ(exp_name, name);
            EXPECT_TRUE(error.empty());
        }
        else {
            EXPECT_TRUE(error.contains(exp_error_pattern));
        }
    };

    test_s("z",                true,  "",    "z",     "");
    test_s("Ctrl-Space",       true,  "C",   "Space", "");
    test_s("Shift-f",          true,  "S",   "f",     "");
    test_s("Shift-Alt-n",      true,  "SA",  "n",     "");
    test_s("Shift-CTRL-p",     true,  "SC",  "p",     "");
    test_s("ShIFt-Ctrl-Alt-n", true,  "SCA", "n",     "");
    test_s("ctrl-Alt-SHIFT-q", true,  "SCA", "q",     "");

    // Unknown key name is not an error.
    test_s("xxx", true,  "", "xxx", "");

    // Errors.
    test_s("",         false, "", "", "Missing key name");
    test_s(" \t",      false, "", "", "Missing key name");
    test_s("Ctralt-x", false, "", "", "Invalid key modifier");
    test_s("x-Shift",  false, "", "", "Invalid key modifier");
}

TEST_F(EventTest, BuildKeyText) {
    // Special case for space key.
    EXPECT_EQ(" ",  Event::BuildKeyText(GetMods(""), "Space"));

    // Single letters, unshifted and shifted.
    EXPECT_EQ("p",  Event::BuildKeyText(GetMods(""),   "p"));
    EXPECT_EQ("t",  Event::BuildKeyText(GetMods(""),   "t"));
    EXPECT_EQ("U",  Event::BuildKeyText(GetMods("S"),  "u"));
    EXPECT_EQ("Z",  Event::BuildKeyText(GetMods("S"),  "z"));

    // Special characters, unshifted and shifted.
    EXPECT_EQ(",",  Event::BuildKeyText(GetMods(""),   ","));
    EXPECT_EQ("-",  Event::BuildKeyText(GetMods(""),   "-"));
    EXPECT_EQ(".",  Event::BuildKeyText(GetMods(""),   "."));
    EXPECT_EQ("/",  Event::BuildKeyText(GetMods(""),   "/"));
    EXPECT_EQ("0",  Event::BuildKeyText(GetMods(""),   "0"));
    EXPECT_EQ("1",  Event::BuildKeyText(GetMods(""),   "1"));
    EXPECT_EQ("2",  Event::BuildKeyText(GetMods(""),   "2"));
    EXPECT_EQ("3",  Event::BuildKeyText(GetMods(""),   "3"));
    EXPECT_EQ("4",  Event::BuildKeyText(GetMods(""),   "4"));
    EXPECT_EQ("5",  Event::BuildKeyText(GetMods(""),   "5"));
    EXPECT_EQ("6",  Event::BuildKeyText(GetMods(""),   "6"));
    EXPECT_EQ("7",  Event::BuildKeyText(GetMods(""),   "7"));
    EXPECT_EQ("8",  Event::BuildKeyText(GetMods(""),   "8"));
    EXPECT_EQ("9",  Event::BuildKeyText(GetMods(""),   "9"));
    EXPECT_EQ(";",  Event::BuildKeyText(GetMods(""),   ";"));
    EXPECT_EQ("=",  Event::BuildKeyText(GetMods(""),   "="));
    EXPECT_EQ("[",  Event::BuildKeyText(GetMods(""),   "["));
    EXPECT_EQ("'",  Event::BuildKeyText(GetMods(""),   "'"));
    EXPECT_EQ("\\", Event::BuildKeyText(GetMods(""),   "\\"));
    EXPECT_EQ("]",  Event::BuildKeyText(GetMods(""),   "]"));
    EXPECT_EQ("`",  Event::BuildKeyText(GetMods(""),   "`"));

    EXPECT_EQ("<",  Event::BuildKeyText(GetMods("S"),  ","));
    EXPECT_EQ("_",  Event::BuildKeyText(GetMods("S"),  "-"));
    EXPECT_EQ(">",  Event::BuildKeyText(GetMods("S"),  "."));
    EXPECT_EQ("?",  Event::BuildKeyText(GetMods("S"),  "/"));
    EXPECT_EQ(")",  Event::BuildKeyText(GetMods("S"),  "0"));
    EXPECT_EQ("!",  Event::BuildKeyText(GetMods("S"),  "1"));
    EXPECT_EQ("@",  Event::BuildKeyText(GetMods("S"),  "2"));
    EXPECT_EQ("#",  Event::BuildKeyText(GetMods("S"),  "3"));
    EXPECT_EQ("$",  Event::BuildKeyText(GetMods("S"),  "4"));
    EXPECT_EQ("%",  Event::BuildKeyText(GetMods("S"),  "5"));
    EXPECT_EQ("^",  Event::BuildKeyText(GetMods("S"),  "6"));
    EXPECT_EQ("&",  Event::BuildKeyText(GetMods("S"),  "7"));
    EXPECT_EQ("*",  Event::BuildKeyText(GetMods("S"),  "8"));
    EXPECT_EQ("(",  Event::BuildKeyText(GetMods("S"),  "9"));
    EXPECT_EQ(":",  Event::BuildKeyText(GetMods("S"),  ";"));
    EXPECT_EQ("+",  Event::BuildKeyText(GetMods("S"),  "="));
    EXPECT_EQ("{",  Event::BuildKeyText(GetMods("S"),  "["));
    EXPECT_EQ("\"", Event::BuildKeyText(GetMods("S"),  "'"));
    EXPECT_EQ("|",  Event::BuildKeyText(GetMods("S"),  "\\"));
    EXPECT_EQ("}",  Event::BuildKeyText(GetMods("S"),  "]"));
    EXPECT_EQ("~",  Event::BuildKeyText(GetMods("S"),  "`"));

    // Other special characters are passed through as is.
    EXPECT_EQ("\t", Event::BuildKeyText(GetMods(""),  "\t"));
    EXPECT_EQ("\t", Event::BuildKeyText(GetMods("S"), "\t"));

    // Other modifiers are not handled.
    EXPECT_EQ("", Event::BuildKeyText(GetMods("C"),  "a"));
    EXPECT_EQ("", Event::BuildKeyText(GetMods("A"),  "b"));
    EXPECT_EQ("", Event::BuildKeyText(GetMods("SC"), "q"));
    EXPECT_EQ("", Event::BuildKeyText(GetMods("CA"), "z"));
}

TEST_F(EventTest, GetControllerButtonString) {
    Event ev;

    ev.flags.Set(Event::Flag::kButtonPress);

    ev.device = Event::Device::kLeftController;
    ev.button = Event::Button::kCenter;
    EXPECT_EQ("L:Center", ev.GetControllerButtonString());

    ev.device = Event::Device::kRightController;
    ev.button = Event::Button::kMenu;
    EXPECT_EQ("R:Menu", ev.GetControllerButtonString());

    // Same with button release.
    ev.flags.Reset(Event::Flag::kButtonPress);
    ev.flags.Set(Event::Flag::kButtonRelease);
    EXPECT_EQ("R:Menu", ev.GetControllerButtonString());
}
