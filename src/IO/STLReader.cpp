#include "IO/STLReader.h"

#include <cctype>
#include <sstream>

#include <ion/base/stringutils.h>

#include "ExceptionBase.h"
#include "Math/Point3fMap.h"
#include "Util/General.h"
#include "Util/Read.h"

// ----------------------------------------------------------------------------
// Helper classes.
// ----------------------------------------------------------------------------

//! Exceptions thrown during reading.
class STLException_ : public ExceptionBase {
  public:
    STLException_(const Util::FilePath &path, int line_number,
                  const std::string &msg) :
        ExceptionBase(path, line_number, msg) {}
};

// ----------------------------------------------------------------------------
// General STL reading functions.
// ----------------------------------------------------------------------------

//! Returns true if the data in the given string most likely represents text
//! STL.
static bool IsTextSTL_(const std::string &data) {
    // An STL text file must start with the word "solid" after optional
    // whitespace.
    size_t start = data.size();
    for (size_t i = 0; i < data.size(); ++i) {
        if (! isspace(data[i])) {
            start = i;
            break;
        }
    }
    return start + 5U < data.size() &&
        data[start + 0] == 's' &&
        data[start + 1] == 'o' &&
        data[start + 2] == 'l' &&
        data[start + 3] == 'i' &&
        data[start + 4] == 'd';
}

// ----------------------------------------------------------------------------
// Binary STL reading class.
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// Text STL reading class.
// ----------------------------------------------------------------------------

class TextSTLReader_ {
  public:
    TextSTLReader_() : point_map_(0) {}  // TODO: Maybe use precision?

    TriMesh ReadMesh(const Util::FilePath &path, const std::string &data,
                     const UnitConversion &conv, std::string &error_message);

  private:
    float      conversion_factor_;  //!< Unit conversion factor.
    Point3fMap point_map_;          //!< Used to share common vertices.

    //! Splits data string into lines, trims whitespace, and removes blank
    //! lines.
    static std::vector<std::string> SplitIntoLines_(const std::string &data);

    //! Converts a point using the UnitConversion factor and changes from STL
    //! coordinates (Z up) to ours (Y up);
    Point3f ConvertPoint_(const Point3f &p) const {
        return conversion_factor_ * Point3f(p[0], p[2], p[1]);
    }
};

TriMesh TextSTLReader_::ReadMesh(const Util::FilePath &path,
                                 const std::string &data,
                                 const UnitConversion &conv,
                                 std::string &error_message) {
    using ion::base::StartsWith;

    conversion_factor_ = conv.GetFactor();

    // Split the data into lines. Ignore whitespace and trim each line.
    const std::vector<std::string> lines = SplitIntoLines_(data);
    int cur_line = 0;
    if (! StartsWith(lines[cur_line], "solid"))
        throw STLException_(path, 1, "Expected 'solid'");

    TriMesh mesh;

    // Read facets.
    while (StartsWith(lines[++cur_line], "facet")) {
        if (! StartsWith(lines[++cur_line], "outer loop"))
            throw STLException_(path, cur_line + 1, "Expected 'outer loop'");
        // Read 3 vertices.
        for (int i = 0; i < 3; ++i) {
            if (! StartsWith(lines[++cur_line], "vertex"))
                throw STLException_(path, cur_line + 1, "Expected 'vertex'");
            std::istringstream in(lines[cur_line]);
            std::string v;
            Point3f p;
            if (! (in >> v >> p[0] >> p[1] >> p[2]))
                throw STLException_(path, cur_line + 1, "Invalid vertex");
            mesh.indices.push_back(point_map_.Add(ConvertPoint_(p)));
        }
        if (! StartsWith(lines[++cur_line], "endloop"))
            throw STLException_(path, cur_line + 1, "Expected 'endloop'");
        if (! StartsWith(lines[++cur_line], "endfacet"))
            throw STLException_(path, cur_line + 1, "Expected 'endfacet'");
    }
    if (! StartsWith(lines[cur_line], "endsolid"))
        throw STLException_(path, cur_line + 1, "Expected 'endsolid'");

    mesh.points = point_map_.GetPoints();

    return mesh;
}

std::vector<std::string>
TextSTLReader_::SplitIntoLines_(const std::string &data) {
    std::vector<std::string> lines = ion::base::SplitString(data, "\n");

    // Trim whitespace.
    for (auto &line: lines)
        line = ion::base::TrimStartAndEndWhitespace(line);

    // Delete blank lines, which should now be empty.
    Util::EraseIf(lines, [](const std::string &s){ return s.empty(); });

    return lines;
}

// ----------------------------------------------------------------------------
// Public STL reading functions.
// ----------------------------------------------------------------------------

TriMesh ReadSTLFile(const Util::FilePath &path, const UnitConversion &conv,
                    std::string &error_message) {
    TriMesh mesh;

    try {
        // Read the file into a string.
        std::string data;
        if (Util::ReadFile(path, data)) {
            if (IsTextSTL_(data))
                mesh = TextSTLReader_().ReadMesh(path, data, conv,
                                                 error_message);
            else
                std::cerr << "XXXX Got STL Binary\n";
        }
    }
    catch (STLException_ &ex) {
        error_message = ex.what();
        mesh.points.clear();
        mesh.indices.clear();
    }

    return mesh;
}
