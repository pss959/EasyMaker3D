#include "App/VRModelLoader.h"

#include <openvr.h>

#include <cmath>
#include <string>

#include <ion/base/datacontainer.h>
#include <ion/gfx/image.h>
#include <ion/math/vectorutils.h>

#include "Base/Tuning.h"
#include "Math/Types.h"
#include "Parser/Registry.h"
#include "SG/MutableTriMeshShape.h"
#include "Util/Assert.h"
#include "Util/KLog.h"

// ----------------------------------------------------------------------------
// Helper functions.
// ----------------------------------------------------------------------------

namespace {

/// Struct storing everything needed to build a MutableTriMeshShape from a
/// loaded model.
struct ModelData_ {
    TriMesh               mesh;    ///< Stores vertices and triangle indices.
    std::vector<Vector3f> normals;
    std::vector<Point2f>  tex_coords;
};

/// Returns a string property from OpenVR.
static std::string GetStringProperty_(vr::TrackedDeviceIndex_t device,
                                      vr::TrackedDeviceProperty prop) {
    auto &vsys = *vr::VRSystem();

    const uint32_t len =
        vsys.GetStringTrackedDeviceProperty(device, prop, nullptr, 0, nullptr);
    ASSERT(len > 0);

    char name[len];
    vsys.GetStringTrackedDeviceProperty(device, prop, name, len, nullptr);
    return name;
}

/// Returns the name of the SteamVR model with the given handle.
static std::string GetModelName_(vr::VRInputValueHandle_t handle) {
    auto &vin = *vr::VRInput();
    std::string name;
    vr::InputOriginInfo_t info;
    if (vin.GetOriginTrackedDeviceInfo(handle, &info, sizeof(info)) ==
        vr::VRInputError_None) {
        name = GetStringProperty_(info.trackedDeviceIndex,
                                  vr::Prop_RenderModelName_String);
    }
    else {
        KLOG('v', "*** Unable to find model for loading");
    }
    return name;
}

/// Reports successful loading of the named model.
static void ReportModelLoad_(const std::string &name) {
    auto &vmod = *vr::VRRenderModels();
    const size_t comp_count = vmod.GetComponentCount(name.c_str());
    KLOG('v', "Loaded controller model '" << name << "' with "
         << comp_count << " component(s):");
    for (size_t i = 0; i < comp_count; ++i) {
        char comp_name[2048];
        vmod.GetComponentName(name.c_str(), i, comp_name, 2048);
        KLOG('v', "  Component [" << i << "] = '" << comp_name << "'");
    }
}

/// Loads the named SteamVR model, returning a handle to it.
static vr::RenderModel_t * LoadModel_(const std::string &name) {
    auto &vmod = *vr::VRRenderModels();

    vr::RenderModel_t       *model = nullptr;
    vr::EVRRenderModelError  error;
    do {
        error = vmod.LoadRenderModel_Async(name.c_str(), &model);
    } while (error == vr::VRRenderModelError_Loading);
    if (error == vr::VRRenderModelError_None) {
        ASSERT(model);
        if (KLogger::HasKeyCharacter('v'))
            ReportModelLoad_(name);
    }
    else {
        KLOG('v', "*** Unable to load controller model '" << name << "'");
    }
    return model;
}

/// Loads the SteamVR texture with the given ID.
static vr::RenderModel_TextureMap_t * LoadTexture_(vr::TextureID_t id) {
    auto &vmod = *vr::VRRenderModels();

    vr::RenderModel_TextureMap_t *tex = nullptr;
    vr::EVRRenderModelError       error;
    do {
        error = vmod.LoadTexture_Async(id, &tex);
    } while (error == vr::VRRenderModelError_Loading);

    if (error != vr::VRRenderModelError_None) {
        KLOG('v', "*** Unable to load controller model texture " << id);
    }
    return tex;
}

/// Stores reasonable normals in a ModelData_ instance when normals are found
/// to be bad.
/// \todo Figure out why some SteamVR models end up with bad normals.
static void FixNormals_(ModelData_ &mdata) {
    Point3f center(0, 0, 0);

    const auto &points = mdata.mesh.points;
    for (const auto &p: points)
        center += p;

    center /= points.size();

    // Set the normal to the direction from the center point to the point.
    for (size_t i = 0; i < points.size(); ++i)
        mdata.normals[i] = ion::math::Normalized(points[i] - center);
}

/// Builds and returns a MutableTriMeshShape from the given SteamVR model.
static SG::MutableTriMeshShapePtr BuildShape_(const std::string &name,
                                              vr::RenderModel_t &model) {
    auto to_point3 = [](const vr::HmdVector3_t &p){
        return Point3f(p.v[0], p.v[1], p.v[2]);
    };

    ModelData_ mdata;
    mdata.mesh.points.resize(model.unVertexCount);
    mdata.normals.resize(model.unVertexCount);
    mdata.tex_coords.resize(model.unVertexCount);

    bool fix_normals = false;
    for (size_t i = 0; i < mdata.mesh.points.size(); ++i) {
        const auto &vert = model.rVertexData[i];
        mdata.mesh.points[i] = to_point3(vert.vPosition);
        mdata.normals[i]     = Vector3f(to_point3(vert.vNormal));
        mdata.tex_coords[i].Set(vert.rfTextureCoord[0], vert.rfTextureCoord[1]);
        if (mdata.normals[i] == Vector3f::Zero())
            fix_normals = true;
    }
    mdata.mesh.indices.resize(3 * model.unTriangleCount);
    for (size_t i = 0; i < mdata.mesh.indices.size(); ++i)
        mdata.mesh.indices[i] = model.rIndexData[i];

    if (fix_normals) {
        KLOG('v', "Fixing zero-length normals for model '" << name << "'");
        FixNormals_(mdata);
    }

    // Create a MutableTriMeshShape with texture coordinates.
    auto shape = Parser::Registry::CreateObject<SG::MutableTriMeshShape>();
    shape->ChangeMeshWithVertexData(mdata.mesh,
                                    mdata.normals, mdata.tex_coords);

    return shape;
}

/// Creates an Ion Image from a SteamVR texture map.
static ion::gfx::ImagePtr BuildIonImage_(
    const vr::RenderModel_TextureMap_t &texture) {
    const size_t w = texture.unWidth;
    const size_t h = texture.unHeight;

    ion::gfx::ImagePtr image(new ion::gfx::Image);
    image->Set(ion::gfx::Image::kRgba8888, w, h,
               ion::base::DataContainer::CreateAndCopy(
                   texture.rubTextureMapData,
                   4 * w * h, true, ion::base::AllocatorPtr()));
    return image;
}

/// Finds the points in the given TriMesh that should be used to connect
/// affordances and and sets the corresponding values in the
/// Controller::CustomModel.
static void FindConnectionPoints_(Hand hand, const TriMesh &mesh,
                                  Controller::CustomModel &custom_model) {
    // Attach the laser pointer to the point that is close to the center in X
    // and is farthest forward (smallest Z).
    custom_model.pointer_pos = Point3f(0, 0, 100000);

    // Attach the grip guide to the point furthest to the palm side.
    custom_model.grip_pos = mesh.points[0];

    // Find the center in X.
    float center_x = 0;
    for (const auto &p: mesh.points)
        center_x += p[0];
    center_x /= mesh.points.size();

    for (const auto &p: mesh.points) {
        if (std::abs(p[0] - center_x) < .001f &&
            p[2] < custom_model.pointer_pos[2])
            custom_model.pointer_pos = p;

        if (hand == Hand::kLeft) {
            if (p[0] > custom_model.grip_pos[0])
                custom_model.grip_pos = p;
        }
        else {
            if (p[0] < custom_model.grip_pos[0])
                custom_model.grip_pos = p;
        }
    }

    // Attach the touch affordance to the same point as the laser pointer but a
    // little below it.
    custom_model.touch_pos = custom_model.pointer_pos;
    custom_model.touch_pos[1] -= TK::kControllerTouchYOffset;
}

}  // anonymous namespace

// ----------------------------------------------------------------------------
// VRModelLoader functions.
// ----------------------------------------------------------------------------

bool VRModelLoader::LoadControllerModel(uint64_t handle, Hand hand,
                                        Controller::CustomModel &custom_model) {
    auto &vmod   = *vr::VRRenderModels();
    bool success = false;

    const std::string name = GetModelName_(handle);
    if (! name.empty()) {
        if (vr::RenderModel_t *model = LoadModel_(name)) {
            if (vr::RenderModel_TextureMap_t *tex =
                LoadTexture_(model->diffuseTextureId)) {
                auto shape = BuildShape_(name, *model);
                custom_model.shape         = shape;
                custom_model.texture_image = BuildIonImage_(*tex);
                FindConnectionPoints_(hand, shape->GetMesh(), custom_model);
                success = true;
                vmod.FreeTexture(tex);
            }
            vmod.FreeRenderModel(model);
        }
    }
    return success;
}
