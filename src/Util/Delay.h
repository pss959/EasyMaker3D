#pragma once

#include <functional>

namespace Util {

/// \name Delayed Action
///@{

/// Executes a function after a given delay (specified in seconds). This
/// operates asynchronously so that the main thread does not block. Returns an
/// integer ID that can be used to cancel with CancelDelayed().
int RunDelayed(float seconds, const std::function<void()> &func);

/// Cancels the function with the given ID that was returned by RunDelayed()
/// and returns true if it is still active. Does nothing but return false if
/// the ID is bad or if the function is not active.
bool CancelDelayed(int id);

/// Returns true if any delayed function is waiting to execute.
bool IsAnyDelaying();

/// Resets (for tests to start with pristine state.)
void ResetDelay();

///@}

}  // namespace Util
