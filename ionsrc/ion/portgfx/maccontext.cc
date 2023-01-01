#include <dlfcn.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <cstring>
#include <memory>
#include <mutex>  // NOLINT(build/c++11)
#include <string>

#include "ion/base/lockguards.h"
#include "ion/base/logging.h"
#include "ion/base/sharedptr.h"
#include "ion/base/staticsafedeclare.h"
#include "ion/port/environment.h"
#include "ion/portgfx/glcontext.h"

#define GL_GLEXT_FUNCTION_POINTERS
#include "ion/portgfx/glheaders.h"

#include <CoreFoundation/CFBundle.h>

// This is defined in macfuncs.mm:
extern uintptr_t MacGetCurrentGlContextId();

namespace ion {
namespace portgfx {
namespace {

// This class wraps a MAC context in an ion::portgfx::GlContext implementation.
class MacContext : public GlContext {
 public:
  MacContext();
  ~MacContext() override {}

  void WrapContext(uintptr_t id) {
    id_ = id;
    SetIds(CreateId(), CreateShareGroupId(), id);
  }

  // GlContext implementation.
  bool IsValid() const override { return id_; }
  void* GetProcAddress(const char* proc_name, uint32_t flags) const override;
  void SwapBuffers() override {}
  bool MakeContextCurrentImpl() override {
    return false;
  }
  void ClearCurrentContextImpl() override {}
  GlContextPtr CreateGlContextInShareGroupImpl(
      const GlContextSpec& spec) override {
    return GlContextPtr(nullptr);
  }
  bool IsOwned() const override { return false; }  // Always wrapped.

 private:
  uintptr_t   id_;
  CFBundleRef framework_;
};

MacContext::MacContext() : id_(0) {
  framework_ = CFBundleGetBundleWithIdentifier(CFSTR("com.apple.opengl"));
}

void* MacContext::GetProcAddress(const char* proc_name, uint32_t flags) const {
  if (framework_ == nullptr) {
    LOG(ERROR) << "Failed to get the Framework.";
    return nullptr;
  }

  // This is adapted from GLFW so it is consistent.
  CFStringRef sym_name = CFStringCreateWithCString(
      kCFAllocatorDefault, proc_name, kCFStringEncodingASCII);
  auto symbol = CFBundleGetFunctionPointerForName(framework_, sym_name);
  CFRelease(sym_name);

  return symbol;
}

}  // anonymous namespace

// static
GlContextPtr GlContext::CreateGlContext(const GlContextSpec& spec) {
  LOG(ERROR) << "(mac) GlContext::CreateGlContext should not be called.";
  return GlContextPtr(nullptr);
}

// static
GlContextPtr GlContext::CreateWrappingGlContext() {
  uintptr_t id = MacGetCurrentGlContextId();
  base::SharedPtr<MacContext> context(new MacContext);
  context->WrapContext(id);
  return context;
}

// static
uintptr_t GlContext::GetCurrentGlContextId() {
  return MacGetCurrentGlContextId();
}

}  // namespace portgfx
}  // namespace ion
