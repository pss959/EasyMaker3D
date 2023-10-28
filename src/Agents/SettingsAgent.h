#pragma once

#include "Util/Memory.h"

DECL_SHARED_PTR(Settings);
DECL_SHARED_PTR(SettingsAgent);

/// SettingsAgent is an abstract interface class for managing application
/// settings. Settings are represented by an instance of the Settings class.
///
/// \ingroup Agents
class SettingsAgent {
  public:
    /// Returns the current settings (read-only).
    virtual const Settings & GetSettings() const = 0;

    /// Updates the settings to new values.
    virtual void SetSettings(const Settings &new_settings) = 0;
};
