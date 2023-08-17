#pragma once

#include <string>

#include "Enums/Dim.h"
#include "Util/Enum.h"
#include "Util/Assert.h"

/// The Dimensionality class is used to indicate 1, 2, or 3 dimensions and axes
/// within those dimensions. It has helpful functions to make certain
/// operations easier.
///
/// \ingroup Math
class Dimensionality {
  public:
    /// The default constructor creates a 0-dimensional (empty) instance.
    Dimensionality() { Clear(); }

    /// Convenience constructor sets a single dimension.
    explicit Dimensionality(int dim) {
        Clear();
        AddDimension(dim);
    }

    /// Convenience constructor that returns an instance set to the dimensions
    /// in the given string, which must contain only the characters 'X', 'Y',
    /// and 'Z'.
    explicit Dimensionality(const Str &s) {
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

    /// Does the opposite of the constructor taking a string. Useful for
    /// testing.
    Str GetAsString() const {
        Str s;
        for (int dim = 0; dim < 3; ++dim)
            if (dimensions_[dim])
                s += static_cast<char>('X' + dim);
        return s;
    }

    /// For debugging.
    Str ToString() const {
        if (count_ == 0)
            return "DIMS[0]";
        return "DIMS[" + Util::ToString(count_) + ":" + GetAsString() + "]";
    }

  private:
    int  count_;          ///< Number of dimensions.
    bool dimensions_[3];  ///< Dimension flags.
};
