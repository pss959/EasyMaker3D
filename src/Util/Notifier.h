#pragma once

#include <algorithm>
#include <functional>
#include <string>

#include "Assert.h"
#include "Util/String.h"

namespace Util {

//! \name Notification
//!@{

//! A Notifier maintains a collection of callback functions that are invoked
//! when the Notifier's Notify() function is called. This is a variadic
//! template; the function parameters are defined when the Notifier is
//! defined.
//!
//! Each observer is added with a string key that identifies it for later
//! removal.
template <typename... ARGS> class Notifier {
  public:
    typedef std::function<void(ARGS...)> ObserverFunc;

    //! Notifies all observers of a change.
    void Notify(ARGS... args) {
        for (auto &observer: observers_)
            observer.func(args...);
    }

    //! Adds an observer function to invoke. This asserts if the key is already
    //! in use.
    void AddObserver(const std::string &key, const ObserverFunc &func) {
        ASSERT(FindObserverIndex_(key) == -1);
        ObserverData_ data;
        data.key  = key;
        data.func = func;
        observers_.push_back(data);
    }

    //! Convenience for AddObserver() that uses the string representation of a
    //! void* pointer as a key.
    void AddObserver(const void *ptr, const ObserverFunc &func) {
        AddObserver(Util::ToString(ptr), func);
    }

    //! Removes the observer function with the given key. Asserts if it is not
    //! found.
    void RemoveObserver(const std::string &key) {
        const int index = FindObserverIndex_(key);
        ASSERT(index >= 0);
        observers_.erase(observers_.begin() + index);
    }

    //! Convenience for RemoveObserver() that uses the string representation of
    //! a void* pointer as a key.
    void RemoveObserver(const void *ptr) {
        RemoveObserver(Util::ToString(ptr));
    }

  private:
    //! Data stored for each observer.
    struct ObserverData_ {
        std::string  key;   //!< Key string identifying the observer.
        ObserverFunc func;  //!< The observer function to invoke.
    };

    //! Vector of observer functions to invoke. That they could be stored as a
    //! map of some sort to make key lookup faster, but looking up keys is
    //! extremely rare compared to iterating over the observers, so a vector
    //! makes much more sense.
    std::vector<ObserverData_> observers_;

    //! Returns the index of the ObserverData_ with the given key, or -1 if it
    //! is not found.
    int FindObserverIndex_(const std::string &key) const {
        auto it = std::find_if(
            observers_.begin(), observers_.end(),
            [key](const ObserverData_ &data){ return data.key == key; });
        return it == observers_.end() ? -1 : it - observers_.begin();
    }
};

//!@}

}  // namespace Util
