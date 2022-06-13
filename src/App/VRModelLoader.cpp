#include "App/VRModelLoader.h"

#include <openvr.h>

#include <string>

#include <ion/base/datacontainer.h>
#include <ion/gfx/image.h>

#include "Math/Types.h"
#include "Parser/Registry.h"
#include "SG/MutableTriMeshShape.h"
#include "Util/Assert.h"
#include "Util/KLog.h"

// ----------------------------------------------------------------------------
// Helper functions.
// ----------------------------------------------------------------------------

namespace {

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

/// Loads the SteamVR model with the given handle, returning a handle to it.
static vr::RenderModel_t * LoadModel_(vr::VRInputValueHandle_t handle) {
    auto &vin  = *vr::VRInput();
    auto &vmod = *vr::VRRenderModels();

    vr::InputOriginInfo_t info;
    if (vin.GetOriginTrackedDeviceInfo(handle, &info, sizeof(info)) !=
        vr::VRInputError_None) {
        KLOG('v', "*** Unable to find model origin for loading");
        return nullptr;
    }

    const std::string name = GetStringProperty_(
        info.trackedDeviceIndex, vr::Prop_RenderModelName_String);
    KLOG('v', "Loading controller model with name '" << name << "'");

    vr::RenderModel_t       *model = nullptr;
    vr::EVRRenderModelError  error;
    do {
        error = vmod.LoadRenderModel_Async(name.c_str(), &model);
    } while (error == vr::VRRenderModelError_Loading);

    if (error == vr::VRRenderModelError_None) {
        KLOG('v', "Loaded controller model '" << name << "'");
        ASSERT(model);
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

/// Builds and returns a MutableTriMeshShape from the given SteamVR model.
static SG::ShapePtr BuildShape_(vr::RenderModel_t &model) {
    auto to_point3 = [](const vr::HmdVector3_t &p){
        return Point3f(p.v[0], p.v[1], p.v[2]);
    };

    // Create a TriMesh with all vertices and triangles. Also store vertex
    // normals and texture coordinates.
    TriMesh mesh;
    mesh.points.resize(model.unVertexCount);
    std::vector<Vector3f> normals(model.unVertexCount);
    std::vector<Point2f>  tex_coords(model.unVertexCount);
    for (size_t i = 0; i < mesh.points.size(); ++i) {
        const auto &vert = model.rVertexData[i];
        mesh.points[i] = to_point3(vert.vPosition);
        normals[i]     = Vector3f(to_point3(vert.vNormal));
        tex_coords[i]  = Point2f(vert.rfTextureCoord[0],
                                 vert.rfTextureCoord[1]);
    }
    mesh.indices.resize(3 * model.unTriangleCount);
    for (size_t i = 0; i < mesh.indices.size(); ++i)
        mesh.indices[i] = model.rIndexData[i];

    // Create a MutableTriMeshShape with texture coordinates.
    auto shape = Parser::Registry::CreateObject<SG::MutableTriMeshShape>();
    shape->ChangeMeshWithVertexData(mesh, normals, tex_coords);

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

}  // anonymous namespace

// ----------------------------------------------------------------------------
// VRModelLoader functions.
// ----------------------------------------------------------------------------

bool VRModelLoader::LoadControllerModel(uint64_t handle,
                                        Controller::CustomModel &custom_model) {
    auto &vmod = *vr::VRRenderModels();

    bool success = false;
    if (vr::RenderModel_t *model = LoadModel_(handle)) {
        if (vr::RenderModel_TextureMap_t *tex =
            LoadTexture_(model->diffuseTextureId)) {
            custom_model.shape         = BuildShape_(*model);
            custom_model.texture_image = BuildIonImage_(*tex);
            success = true;
            vmod.FreeTexture(tex);
        }
        vmod.FreeRenderModel(model);
    }
    return success;
}
