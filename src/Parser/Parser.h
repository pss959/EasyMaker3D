#pragma once

#include <memory>
#include <string>
#include <vector>

#include "Parser/Object.h"
#include "Util/FilePath.h"

namespace Parser {

/// Parses a file or stream to produce instances of classes derived from
/// Parser::Object. Any failure results in a Parser::Exception being thrown.
///
/// \ingroup Parser
class Parser {
  public:
    /// This struct represents a dependency created by an included file.
    struct Dependency {
        FilePath including_path;
        FilePath included_path;
    };

    Parser();
    ~Parser();

    /// Sets the directory used to turn relative include paths into absolute
    /// ones. This is FilePath::GetResourceBasePath() by default.
    void SetBasePath(const FilePath &path);

    /// Parses the contents of the file with the given path, returning the root
    /// Object in the parse graph. This will never return null; any error will
    /// result in a Parser::Exception being thrown.
    ObjectPtr ParseFile(const FilePath &path);

    /// Parses the contents of the given string, returning the root Object in
    /// the parse graph. This will never return null; any error will result in
    /// a Parser::Exception being thrown.
    ObjectPtr ParseFromString(const std::string &str);

    /// Returns a vector of all path dependencies created by included files
    /// found during parsing.
    const std::vector<Dependency> GetDependencies() const;

    /// Resets the Parser; useful for testing to ensure clean state.
    void Reset();

  private:
    class Impl_;
    std::unique_ptr<Impl_> impl_;
};

}  // namespace Parser
