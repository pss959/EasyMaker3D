#pragma once

#include <algorithm>
#include <functional>
#include <string>

#include "Util/Assert.h"
#include "Util/String.h"

namespace Util {

/// \name Notification
///@{

/// A Notifier maintains a collection of callback functions that are invoked
/// when the Notifier's Notify() function is called. This is a variadic
/// template; the function parameters are defined when the Notifier is
/// defined.
///
/// Each observer is added with a string key that identifies it for later
/// removal.
template <typename... ARGS> class Notifier {
  public:
    typedef std::function<void(ARGS...)> ObserverFunc;

    /// Notifies all observers of a change.
    void Notify(ARGS... args) {
        for (auto &observer: observers_)
            if (observer.is_enabled)
                observer.func(args...);
    }

    /// Adds an observer function to invoke. This asserts if the key is already
    /// in use.
    void AddObserver(const std::string &key, const ObserverFunc &func) {
        ASSERTM(FindObserverIndex_(key) == -1, "Duplicate Observer key " + key);
        ObserverData_ data;
        data.key        = key;
        data.func       = func;
        data.is_enabled = true;
        observers_.push_back(data);
    }

    /// Removes the observer function with the given key. Asserts if it is not
    /// found.
    void RemoveObserver(const std::string &key) {
        const int index = FindObserverIndex_(key);
        ASSERT(index >= 0);
        observers_.erase(observers_.begin() + index);
    }

    /// Enables or disables the observer with the given key. A disabled
    /// observer will not be notified of changes. Observers are enabled by
    /// default.
    void EnableObserver(const std::string &key, bool is_enabled) {
        const int index = FindObserverIndex_(key);
        ASSERT(index >= 0);
        observers_[index].is_enabled = is_enabled;
    }

    /// Returns true if there is an observer for the given key.
    bool HasObserver(const std::string &key) const {
        return FindObserverIndex_(key) >= 0;
    }

    /// \name Pointer-based Convenience Functions
    /// Many uses of observers are within classes that want to use the "this"
    /// pointer as a key. Other situations may also involve using a unique
    /// pointer as a key. Each of these allows the corresponding function to be
    /// called using the string representation of a \c void* pointer as a key.
    ///@{

    void AddObserver(const void *ptr, const ObserverFunc &func) {
        AddObserver(Util::ToString(ptr), func);
    }

    void RemoveObserver(const void *ptr) {
        RemoveObserver(Util::ToString(ptr));
    }

    void EnableObserver(const void *ptr, bool is_enabled) {
        EnableObserver(Util::ToString(ptr), is_enabled);
    }

    bool HasObserver(const void *ptr) const {
        return HasObserver(Util::ToString(ptr));
    }

    ///@}

  private:
    /// Data stored for each observer.
    struct ObserverData_ {
        std::string  key;         ///< Key string identifying the observer.
        ObserverFunc func;        ///< The observer function to invoke.
        bool         is_enabled;  ///< Whether to nofify the observer.
    };

    /// Vector of observer functions to invoke. That they could be stored as a
    /// map of some sort to make key lookup faster, but looking up keys is
    /// extremely rare compared to iterating over the observers, so a vector
    /// makes much more sense.
    std::vector<ObserverData_> observers_;

    /// Returns the index of the ObserverData_ with the given key, or -1 if it
    /// is not found.
    int FindObserverIndex_(const std::string &key) const {
        auto it = std::find_if(
            observers_.begin(), observers_.end(),
            [key](const ObserverData_ &data){ return data.key == key; });
        return it == observers_.end() ? -1 : it - observers_.begin();
    }
};

///@}

}  // namespace Util
