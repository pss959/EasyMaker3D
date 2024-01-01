#pragma once

#include <docopt.h>

#include <map>

/// The Args class wraps DocOpt to manage command-line arguments and provide
/// some convenience functions.
///
/// \ingroup App
class Args {
  public:
    /// The constructor is passed argc and argv from the command line along
    /// with the DocOpt usage string.
    Args(int argc, const char **argv, const Str &usage);

    /// Returns true if the named argument exists.
    bool HasArg(const Str &name) const;

    /// Returns the value of the string argument with the given name. Returns
    /// an empty string if there is no such argument.
    Str GetString(const Str &name) const;

    /// Returns the value of the string argument with the given name, where the
    /// value must be one of the given choices. Throws a DocOpt exception if
    /// the choice is invalid. Returns the first choice if there is no such
    /// argument.
    Str GetStringChoice(const Str &name, const StrVec &choices) const;

    /// Returns the value of the bool argument with the given name. Returns
    /// false if there is no such argument.
    bool GetBool(const Str &name) const;

    /// Convenience that returns the string argument with the given name
    /// converted to an integer if possible. Returns \p default_value if there
    /// is no such argument or if conversion fails.
    int GetAsInt(const Str &name, int default_value) const;

  private:
    std::map<Str, docopt::value> args_;

    const docopt::value & GetArg_(const Str &name) const;
};
