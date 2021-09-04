#pragma once

#include <functional>

namespace Util {

//! \name Notification
//!@{

//! A Notifier maintains a collection of callback functions that are invoked
//! when the Notifier's Notify() function is called. This is a variadic
//! template; the function parameters are defined when the Notifier is
//! defined.
template <typename... ARGS> class Notifier {
  public:
    typedef std::function<void(ARGS...)> ObserverFunc;

    //! Notifies all observers of a change.
    void Notify(ARGS... args) {
        for (auto &observer: observers_)
            observer(args...);
    }

    //! Adds an observer function to invoke.
    void AddObserver(const ObserverFunc &observer) {
        observers_.push_back(observer);
    }

  private:
    //! List of observer functions to invoke.
    std::vector<ObserverFunc> observers_;
};

//!@}

}  // namespace Util
