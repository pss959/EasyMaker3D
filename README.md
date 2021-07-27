# IMakerVR

So far, this is a test for implementing MakerVR using the Ion library.

## VR

Experimenting with Monado

+ Added repository
  `http://ppa.launchpad.net/monado-xr/monado/ubuntu/ groovy main`
+ Installed:
  ```
  monado-gui
  libopenxr1-monado
  libopenxr-dev
  xr-hardware
  ```

### Here is the output from pss-example
Using `/local/github/tmp/openxr-simple-example/pssmain.c`

Build with `make pss-example && ./pss-example`

```
Runtime Name: SteamVR/OpenXR
Runtime Version: 0.1.0
Successfully got XrSystem with id 1157410771928351169 for HMD form factor
View Configuration View 0:
	Resolution       : Recommended 1852x2056, Max: 1852x2056
	Swapchain Samples: Recommended: 1, Max: 1)
View Configuration View 1:
	Resolution       : Recommended 1852x2056, Max: 1852x2056
	Swapchain Samples: Recommended: 1, Max: 1)
Using OpenGL version: 3.2.0 NVIDIA 460.91.03
Using OpenGL Renderer: GeForce RTX 2080 Super with Max-Q Design/PCIe/SSE2
Successfully created a session with OpenGL!
Runtime supports 8 swapchain formats
Runtime supports 8 swapchain formats
Supported GL format: 0x805b
Supported GL format: 0x881a
Supported GL format: 0x881b
Supported GL format: 0x8c41
Supported GL format: 0x8c43
Using preferred swapchain format 0x8c43
Runtime supports 8 swapchain formats
Supported GL format: 0x805b
Supported GL format: 0x881a
Supported GL format: 0x881b
Supported GL format: 0x8c41
Supported GL format: 0x8c43
Supported GL format: 0x81a5
Using preferred swapchain format 0x81a5
Successfully compiled vertex shader!
Successfully compiled fragment shader!
Successfully linked shader program!
GL CALLBACK:  type = 0x8251, severity = 0x826b, message = Buffer detailed info: Buffer object 1 (bound to GL_ARRAY_BUFFER_ARB, usage hint is GL_STATIC_DRAW) will use VIDEO memory as the source for buffer object operations.
GL CALLBACK:  type = 0x8251, severity = 0x826b, message = Buffer detailed info: Buffer object 1 (bound to GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING_ARB (0), and GL_ARRAY_BUFFER_ARB, usage hint is GL_DYNAMIC_DRAW) will use VIDEO memory as the source for buffer object operations.
```
