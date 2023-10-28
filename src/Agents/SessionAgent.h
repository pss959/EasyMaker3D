#pragma once

#include <string>

#include "Enums/FileFormat.h"
#include "Util/Memory.h"

DECL_SHARED_PTR(SessionAgent);
class FilePath;
class UnitConversion;

/// SessionAgent is an abstract interface class that handles session management.
///
/// \ingroup Agents
class SessionAgent {
  public:
    /// Creates a new session.
    virtual void NewSession() = 0;

    /// Returns true if the current session can be saved to its existing file,
    /// meaning that something has changed.
    virtual bool CanSaveSession() const = 0;

    /// Saves the current session to the given path. Returns true if all went
    /// well.
    virtual bool SaveSession(const FilePath &path) = 0;

    /// Loads an existing session from the given path. Returns false and sets
    /// error to an informative string if anything fails.
    virtual bool LoadSession(const FilePath &path, Str &error) = 0;

    /// Returns the name of the Model that can be exported, or an empty string
    /// if there is none.
    virtual Str GetModelNameForExport() const = 0;

    /// Exports the current Model to the given path in the given format using
    /// the given ConversionInfo.  Returns true if all went well.
    virtual bool Export(const FilePath &path, FileFormat format,
                        const UnitConversion &conv) = 0;

    /// Returns the name of the previous session, which may be empty.
    virtual const Str & GetPreviousSessionName() const = 0;

    /// Returns the name of the current session, which may be empty.
    virtual const Str & GetCurrentSessionName() const  = 0;
};
