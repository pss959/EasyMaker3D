#ifndef ION_PORTGFX_CALLBACKCONTEXT_H_
#define ION_PORTGFX_CALLBACKCONTEXT_H_

#include <functional>

#include "ion/portgfx/glcontext.h"

namespace ion {
namespace portgfx {

// The CallbackContext can be used in applications where the GL context is
// managed by a higher-level library such as GLFW. It implements an
// ion::portgfx::GlContext by delegating all of the work via callbacks. All
// callbacks must be set before a CallbackContext can be created successfully.
class CallbackContext : public GlContext {
 public:
  // Opaque context id.
  typedef void* ContextID;

  // GlContext callback typedefs.
  typedef std::function<ContextID(void)> InitCB;
  typedef std::function<bool(void)> IsValidCB;
  typedef std::function<void*(const char*, uint32_t)> GetProcAddressCB;
  typedef std::function<void(void)> SwapBuffersCB;
  typedef std::function<bool(ContextID)> MakeCurrentCB;
  typedef std::function<void(void)> ClearCurrentCB;
  typedef std::function<uintptr_t(void)> GetCurrentCB;

  // Sets the callback to initialize the context. It must return a ContextID
  // that is passed to other functions.
  static void SetInitCB(const InitCB &cb) {
    init_cb_ = cb;
  }

  // Each of these sets the callback for a GlContext function.
  static void SetIsValidCB(const IsValidCB &cb) {
    is_valid_cb_ = cb;
  }
  static void SetGetProcAddressCB(const GetProcAddressCB &cb) {
    get_proc_address_cb_ = cb;
  }
  static void SetSwapBuffersCB(const SwapBuffersCB &cb) {
    swap_buffers_cb_ = cb;
  }
  static void SetMakeCurrentCB(const MakeCurrentCB &cb) {
    make_current_cb_ = cb;
  }
  static void SetClearCurrentCB(const ClearCurrentCB &cb) {
    clear_current_cb_ = cb;
  }
  static void SetGetCurrentCB(const GetCurrentCB &cb) {
    get_current_cb_ = cb;
  }

  // GlContext implementation.
  bool IsValid() const override { return is_valid_cb_(); }
  void* GetProcAddress(const char* proc_name, uint32_t flags) const override {
    return get_proc_address_cb_(proc_name, flags);
  }
  void SwapBuffers() override { swap_buffers_cb_(); }
  bool MakeContextCurrentImpl() override { return make_current_cb_(id_); }
  void ClearCurrentContextImpl() override { clear_current_cb_(); }

  GlContextPtr CreateGlContextInShareGroupImpl(
      const GlContextSpec& spec) override {
    // Not supported.
    LOG(ERROR) << "CallbackContext does not support contexts in share group";
    return GlContextPtr();
  }
  bool IsOwned() const override { return true; }
  bool Init();

  static uintptr_t GetCurrentContext() {
    if (! get_current_cb_)
      LOG(ERROR) << "CallbackContext missing GetCurrent callback";
    return get_current_cb_();
  }

 private:
  // ID returned by the initialization callback.
  ContextID id_ = nullptr;

  // Callbacks.
  static InitCB init_cb_;
  static IsValidCB is_valid_cb_;
  static GetProcAddressCB get_proc_address_cb_;
  static SwapBuffersCB swap_buffers_cb_;
  static MakeCurrentCB make_current_cb_;
  static ClearCurrentCB clear_current_cb_;
  static GetCurrentCB get_current_cb_;
};

}  // namespace portgfx
}  // namespace ion

#endif  // ION_PORTGFX_CALLBACKCONTEXT_H_
