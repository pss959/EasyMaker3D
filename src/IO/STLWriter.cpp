#include "IO/STLWriter.h"

#if XXXX

class STLWriterBase_ {
  public:
    /// Writes a mesh from the data, throwing an exception on error. The path is
    /// just for error messages.
    TriMesh WriteMesh(const FilePath &path, const std::string &data,
                     const UnitConversion &conv) {
        path_              = path;
        conversion_factor_ = conv.GetFactor();

        const TriMesh mesh = WriteMeshImpl(data);
        if (mesh.GetTriangleCount() == 0)
            Throw("No mesh data");
        return mesh;
    }

  protected:
    STLWriterBase_() : point_map_(0) {}  /// \todo Maybe use precision?

    /// Derived classes implement this to do the real work.
    virtual TriMesh WriteMeshImpl(const std::string &data) = 0;

    /// Adds a point to the Point3fMap, converting it first and returning the
    /// resulting index.
    size_t AddPoint(const Point3f &p) {
        return point_map_.Add(ConvertPoint_(p));
    }

    /// Returns the vector of resulting points.
    std::vector<Point3f> GetPoints() const { return point_map_.GetPoints(); }

    /// Throws an exception with the given line number and message.
    void Throw(int line, const std::string &msg) {
        throw STLException_(path_, line, msg);
    }

    /// Throws an exception with no line number.
    void Throw(const std::string &msg) { throw STLException_(path_, msg); }

  private:
    FilePath   path_;               ///< For error messages.
    float      conversion_factor_;  ///< Unit conversion factor.
    Point3fMap point_map_;          ///< Used to share common vertices.

    /// Converts a point using the UnitConversion factor and changes from STL
    /// coordinates (Z up) to ours (Y up);
    Point3f ConvertPoint_(const Point3f &p) const {
        return conversion_factor_ * Point3f(p[0], p[2], -p[1]);
    }
};

// ----------------------------------------------------------------------------
// Binary STL writeing class.
// ----------------------------------------------------------------------------

class BinarySTLWriter_ : public STLWriterBase_ {
  protected:
    virtual TriMesh WriteMeshImpl(const std::string &data) override;

  private:
    const unsigned char *data_;    ///< Input data string as unsigned chars.
    size_t               size_;    ///< Size of data string.
    size_t               offset_;  ///< Current offset in data.

    uint32_t ScanUint32_() {
        Require_(4);
        uint32_t n;
        n = (data_[offset_ + 3] << 24 |
             data_[offset_ + 2] << 16 |
             data_[offset_ + 1] <<  8 |
             data_[offset_ + 0]);
        offset_ += 4;
        return n;
    }

    float ScanFloat_() {
        uint32_t n = ScanUint32_();
        return *reinterpret_cast<float *>(&n);
    }

    void Require_(size_t n) {
        if (offset_ + n > size_)
            Throw("Not enough binary data");
    }
};

TriMesh BinarySTLWriter_::WriteMeshImpl(const std::string &data) {
    TriMesh mesh;

    data_ = reinterpret_cast<const unsigned char *>(data.c_str());
    size_ = data.size();

    // Skip the 80-byte header.
    offset_ = 80;

    // Get the number of facets (4 bytes).
    const uint32_t facet_count = ScanUint32_();

    // Write each facet:
    //   3 4-byte floats for normal (ignored)
    //   3 4-byte floats for vertex 0
    //   3 4-byte floats for vertex 1
    //   3 4-byte floats for vertex 2
    //   1 2-byte int for attributes (ignored)
    for (uint32_t i = 0; i < facet_count; ++i) {
        // Skip the normal.
        offset_ += 3 * 4;

        // Write the three triangle vertex points.
        Point3f p;
        for (int v = 0; v < 3; ++v) {
            for (int i = 0; i < 3; ++i)
                p[i] = ScanFloat_();
            mesh.indices.push_back(AddPoint(p));
        }

        // Skip the attributes.
        offset_ += 2;
    }
    mesh.points = GetPoints();
    return mesh;
}

// ----------------------------------------------------------------------------
// Text STL writeing class.
// ----------------------------------------------------------------------------

class TextSTLWriter_ : public STLWriterBase_ {
  protected:
    virtual TriMesh WriteMeshImpl(const std::string &data) override;

  private:
    /// Splits data string into lines, trims whitespace, and removes blank
    /// lines.
    static std::vector<std::string> SplitIntoLines_(const std::string &data);
};

TriMesh TextSTLWriter_::WriteMeshImpl(const std::string &data) {
    using ion::base::StartsWith;

    // Split the data into lines. Ignore whitespace and trim each line.
    const std::vector<std::string> lines = SplitIntoLines_(data);
    int cur_line = 0;
    if (! StartsWith(lines[cur_line], "solid"))
        Throw(1, "Expected 'solid'");

    TriMesh mesh;

    // Write facets.
    while (StartsWith(lines[++cur_line], "facet")) {
        if (! StartsWith(lines[++cur_line], "outer loop"))
            Throw(cur_line + 1, "Expected 'outer loop'");
        // Write 3 vertices.
        for (int i = 0; i < 3; ++i) {
            if (! StartsWith(lines[++cur_line], "vertex"))
                Throw(cur_line + 1, "Expected 'vertex'");
            std::istringstream in(lines[cur_line]);
            std::string v;
            Point3f p;
            if (! (in >> v >> p[0] >> p[1] >> p[2]))
                Throw(cur_line + 1, "Invalid vertex");
            mesh.indices.push_back(AddPoint(p));
        }
        if (! StartsWith(lines[++cur_line], "endloop"))
            Throw(cur_line + 1, "Expected 'endloop'");
        if (! StartsWith(lines[++cur_line], "endfacet"))
            Throw(cur_line + 1, "Expected 'endfacet'");
    }
    if (! StartsWith(lines[cur_line], "endsolid"))
        Throw(cur_line + 1, "Expected 'endsolid'");

    mesh.points = GetPoints();

    return mesh;
}

std::vector<std::string>
TextSTLWriter_::SplitIntoLines_(const std::string &data) {
    std::vector<std::string> lines = ion::base::SplitString(data, "\n");

    // Trim whitespace.
    for (auto &line: lines)
        line = ion::base::TrimStartAndEndWhitespace(line);

    // Delete blank lines, which should now be empty.
    Util::EraseIf(lines, [](const std::string &s){ return s.empty(); });

    return lines;
}
#endif

// ----------------------------------------------------------------------------
// Public STL writing functions.
// ----------------------------------------------------------------------------

void WriteSTLFile(const std::vector<ModelPtr> &models,
                  const FilePath &path, FileFormat format,
                  const UnitConversion &conv) {
}
