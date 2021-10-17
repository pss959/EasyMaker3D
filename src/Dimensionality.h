#pragma once

#include <string>

#include "Assert.h"
#include "Util/Enum.h"

/// The Dimensionality class is used to indicate 1, 2, or 3 dimensions and axes
/// within those dimensions. It has helpful functions to make certain
/// operations easier.
class Dimensionality {
  public:
    /// This enum is used to indicate one of the 3 principal dimensions.
    enum class Dim { X, Y, Z };

    /// The default constructor creates a 0-dimensional (empty) instance.
    Dimensionality() { Clear(); }

    /// Convenience constructor that returns an instance set to the dimensions
    /// in the given string, which must contain only the characters 'X', 'Y',
    /// and 'Z'.
    Dimensionality(const std::string &s) {
        Clear();
        for (char c: s) {
            ASSERTM(c == 'X' || c == 'Y' || c == 'Z', "Invalid character");
            AddDimension(static_cast<Dim>(c - 'X'));
        }
    }

    /// Clears to 0 dimensions.
    void Clear() {
        count_ = 0;
        for (int dim = 0; dim < 3; ++dim)
            dimensions_[dim] = false;
    }

    /// Adds the given dimension to the instance if it is not already there.
    void AddDimension(Dim dim) {
        AddDimension(Util::EnumInt(dim));
    }

    /// Adds the given dimension (0, 1, or 2) to the instance if it is not
    /// already there.
    void AddDimension(int dim) {
        ASSERT(dim >= 0 && dim <= 2);
        if (! dimensions_[dim]) {
            dimensions_[dim] = true;
            ++count_;
        }
    }

    /// Returns the number of dimensions represented.
    int GetCount() const { return count_; }

    /// Returns true if the given dimension is represented.
    bool HasDimension(Dim dim) const { return dimensions_[Util::EnumInt(dim)]; }

    /// Returns true if the given indexed dimension is represented. Asserts if
    /// dim is not 0, 1, or 2.
    bool HasDimension(int dim) const {
        ASSERT(dim >= 0 && dim <= 2);
        return dimensions_[dim];
    }

    /// If this instance is 1-dimensional, this returns the index of that
    /// dimension (0, 1, or 2). Otherwise, it returns -1.
    int GetIndex() const {
        return count_ != 1 ? -1 : (dimensions_[0] ? 0 : dimensions_[1] ? 1 : 2);
    }

    /// For debugging.
    std::string ToString() const {
        if (count_ == 0)
            return "DIMS[0]";
        std::string s = "DIMS[" + Util::ToString(count_) + ":";
        for (int dim = 0; dim < 3; ++dim)
            if (dimensions_[dim])
                s += static_cast<char>('X' + dim);
        s += ']';
        return s;
    }

  private:
    int  count_;          ///< Number of dimensions.
    bool dimensions_[3];  ///< Dimension flags.
};
