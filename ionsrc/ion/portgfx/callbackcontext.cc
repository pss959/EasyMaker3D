#include "ion/portgfx/callbackcontext.h"

namespace ion {
namespace portgfx {

CallbackContext::InitCB CallbackContext::init_cb_;
CallbackContext::IsValidCB CallbackContext::is_valid_cb_;
CallbackContext::GetProcAddressCB CallbackContext::get_proc_address_cb_;
CallbackContext::SwapBuffersCB CallbackContext::swap_buffers_cb_;
CallbackContext::MakeCurrentCB CallbackContext::make_current_cb_;
CallbackContext::ClearCurrentCB CallbackContext::clear_current_cb_;
CallbackContext::GetCurrentCB CallbackContext::get_current_cb_;

bool CallbackContext::Init() {
  // Make sure all callbacks are installed.
  if (! init_cb_)
    LOG(ERROR) << "CallbackContext missing Init callback";
  if (! is_valid_cb_)
    LOG(ERROR) << "CallbackContext missing IsValid callback";
  if (! get_proc_address_cb_)
    LOG(ERROR) << "CallbackContext missing GetProcAddress callback";
  if (! swap_buffers_cb_)
    LOG(ERROR) << "CallbackContext missing SwapBuffers callback";
  if (! make_current_cb_)
    LOG(ERROR) << "CallbackContext missing MakeCurrent callback";
  if (! clear_current_cb_)
    LOG(ERROR) << "CallbackContext missing ClearCurrent callback";
  if (! get_current_cb_)
    LOG(ERROR) << "CallbackContext missing GetCurrent callback";

  id_ = init_cb_();
  if (id_) {
    SetIds(CreateId(), CreateShareGroupId(), reinterpret_cast<uintptr_t>(id_));
    return true;
  }
  return false;
}

// static
GlContextPtr GlContext::CreateGlContext(const GlContextSpec& spec) {
  // Not supported.
  LOG(ERROR) << "CallbackContext does not support owned contexts";
  return GlContextPtr();
}

// static
GlContextPtr GlContext::CreateWrappingGlContext() {
  base::SharedPtr<CallbackContext> context(new CallbackContext);
  if (!context->Init()) {
    context.Reset();
  }
  return context;
}

// static
uintptr_t GlContext::GetCurrentGlContextId() {
  return CallbackContext::GetCurrentContext();
}

}  // namespace portgfx
}  // namespace ion
