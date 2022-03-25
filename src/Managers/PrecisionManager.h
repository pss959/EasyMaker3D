#pragma once

#include <algorithm>

#include "Math/Linear.h"
#include "Memory.h"
#include "Math/Types.h"

DECL_SHARED_PTR(PrecisionManager);

/// The PrecisionManager is in charge of the current precision setting and
/// applying it to various interactive operations.
///
/// \ingroup Managers
class PrecisionManager {
  public:
    /// Returns the current linear precision.
    float GetLinearPrecision() const {
        return precisions_[current_index_].linear;
    }

    /// Returns the current angular precision.
    float GetAngularPrecision() const {
        return precisions_[current_index_].angular;
    }

    /// Applies the current linear precision to the given value, returning the
    /// result.
    float Apply(float value) const {
        return RoundToPrecision(value, GetLinearPrecision());
    }

    /// Returns the result of applying the current linear precision to a
    /// Vector3f.
    Vector3f Apply(const Vector3f &vec) const {
        return Vector3f(Apply(vec[0]), Apply(vec[1]), Apply(vec[2]));
    }

    /// Returns the result of applying the current linear precision to a
    /// Point3f.
    Point3f Apply(const Point3f &pt) const {
        return Point3f(Apply(pt[0]), Apply(pt[1]), Apply(pt[2]));
    }

    /// Variant of Apply that makes sure the value is always positive.
    float ApplyPositive(float value) const {
        const float prec = GetLinearPrecision();
        return std::max(prec, RoundToPrecision(value, prec));
    }

    /// Variant of Apply that makes sure the value is always positive.
    Vector3f ApplyPositive(const Vector3f &vec) const {
        return Vector3f(ApplyPositive(vec[0]),
                        ApplyPositive(vec[1]),
                        ApplyPositive(vec[2]));
    }

    /// Variant of Apply that makes sure the value is always positive.
    Point3f ApplyPositive(const Point3f &pt) const {
        return Point3f(ApplyPositive(pt[0]),
                       ApplyPositive(pt[1]),
                       ApplyPositive(pt[2]));
    }

    /// Returns true if the given values are within the current precision of
    /// each other. In either case, sets distance to the distance between the
    /// values.
    bool AreClose(float value0, float value1, float &distance) const {
        distance = std::fabs(value0 - value1);
        return distance <= GetLinearPrecision();
    }

    /// Returns true if the given points are within the current precision of
    /// each other. In either case, sets distance to the distance between the
    /// points.
    bool AreClose(const Point3f &pt0,
                  const Point3f &pt1, float &distance) const {
        distance = ion::math::Distance(pt0, pt1);
        return distance <= GetLinearPrecision();
    }

    /// Returns true if the given angles are within the current angular
    /// precision of each other. In either case, sets difference to the
    /// anglular difference.
    bool AreAnglesClose(const Anglef &angle0, const Anglef &angle1,
                        Anglef &difference) {
        difference = angle0 - angle1;
        if (difference.Radians() < 0)
            difference = -difference;
        return difference.Degrees() <= GetAngularPrecision();
    }

    /// Applies the current angular precision to the given angle, returning the
    /// result.
    Anglef ApplyAngle(const Anglef &angle) {
        return Anglef::FromDegrees(RoundToPrecision(angle.Degrees(),
                                                    GetAngularPrecision()));
    }

    /// Returns true if the precision can be increased.
    bool CanIncrease() const { return current_index_ + 1 < precisions_.size(); }

    /// Returns true if the precision can be decreased.
    bool CanDecrease() const { return current_index_ > 0; }

    /// Switches to the next higher precision, if possible. Returns whether it
    /// did.
    bool Increase() {
        if (CanIncrease()) {
            ++current_index_;
            return true;
        }
        return false;
    }

    /// Switches to the next lower precision, if possible. Returns whether it
    /// did.
    bool Decrease() {
        if (CanDecrease()) {
            --current_index_;
            return true;
        }
        return false;
    }

  private:
    /// The Precision_ struct stores the linear and angular precision values.
    struct Precision_ {
        float linear;    ///< Linear precision.
        float angular;   ///< Angular precision in degrees.
    };

    /// Allowable precision values for lengths and angles, from lowest precision
    /// to highest.
    std::vector<Precision_> precisions_{
        {  1.f, 15.f },
        {  .1f,  5.f },
        { .01f,  1.f },
    };

    /// Index of the current precision used in the application.
    size_t current_index_ = 0;
};
