#pragma once

#include <X11/Xlib.h>
#include <GL/glx.h>

#define XR_USE_PLATFORM_XLIB
#define XR_USE_GRAPHICS_API_OPENGL
#include <openxr/openxr.h>
#include <openxr/openxr_platform.h>

#include <exception>
#include <string>

#include <ion/math/range.h>
#include <ion/math/rotation.h>
#include <ion/math/vector.h>

//! Abstract base class for OpenXRVR classes, containing utilities.
class OpenXRVRBase {
  protected:
    //! Exception thrown when any function fails.
    class VRException_ : public std::exception {
      public:
        VRException_(const std::string &msg) : msg_(msg) {}
        const char * what() const throw() override { return msg_.c_str(); }
      private:
        std::string msg_;
    };

    //! Sets the XrInstance to use for error checking and reporting.
    void SetInstance(XrInstance instance) {
        instance_ = instance;
    }

    //! \name Conversion utility functions
    //!@{

    //! Converts an XrVector3f to an Ion Vector3f.
    static inline ion::math::Vector3f ToVector3f(const XrVector3f &v) {
        return ion::math::Vector3f(v.x, v.y, v.z);
    }

    //! Converts an XrVector3f to an Ion Point3f.
    static inline ion::math::Point3f ToPoint3f(const XrVector3f &v) {
        return ion::math::Point3f(v.x, v.y, v.z);
    }

    //! Converts an XrQuaternionf to an Ion Rotationf.
    static inline ion::math::Rotationf ToRotationf(const XrQuaternionf &q) {
        return ion::math::Rotationf::FromQuaternion(
            ion::math::Vector4f(q.x, q.y, q.z, q.w));
    }

    //! Converts an XrRect2Di to an Ion Range2i.
    static ion::math::Range2i ToRange2i(const XrRect2Di &r) {
        return ion::math::Range2i::BuildWithSize(
            ion::math::Point2i(r.offset.x, r.offset.y),
            ion::math::Vector2i(r.extent.width, r.extent.height));
    }

    //!@}

    // Error checking and reporting.
    void CheckXr_(XrResult res, const char *cmd, const char *file, int line);
    void Assert_(bool exp, const char *expstr, const char *file, int line);
    void Throw_(const std::string &msg);
    void ReportException_(const VRException_ &ex);
    void ReportDisaster_(const char *msg);

  private:
    XrInstance instance_ = XR_NULL_HANDLE;
};

// ----------------------------------------------------------------------------
// Handy macros.
// ----------------------------------------------------------------------------

//! Calls the given function and throws a VRException_ if the result indicates
//! failure.
#define CHECK_XR_(cmd) CheckXr_(cmd, #cmd, __FILE__, __LINE__)

//! Assertion checking - failure results in throwing a VRException_.
#define ASSERT_(exp) Assert_(exp, #exp, __FILE__, __LINE__)

//! Shorthand for OpenXR-required casts.
#define CAST_(to_type, var) reinterpret_cast<to_type>(var)

