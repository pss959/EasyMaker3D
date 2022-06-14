#pragma once

#include <docopt.h>

#include <map>
#include <string>

/// The Args class wraps DocOpt to manage command-line arguments and provide
/// some convenience functions.
///
/// \ingroup App
class Args {
  public:
    /// The constructor is passed argc and argv from the command line along
    /// with the Docopt usage string.
    Args(int argc, const char **argv, const std::string &usage);

    /// Returns the value of the string argument with the given name. Returns
    /// an empty string if there is no such argument.
    std::string GetString(const std::string &name) const;

    /// Returns the value of the bool argument with the given name. Returns
    /// false if there is no such argument.
    bool GetBool(const std::string &name) const;

  private:
    std::map<std::string, docopt::value> args_;
};
