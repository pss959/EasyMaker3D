#pragma once

namespace Util {

//! \name Notification
//!@{

//! Interface for any class that needs to be notified of some occurrence.
template <typename T> class IObserver {
  public:
    virtual void ProcessChange(const T &t) = 0;
};

//! A Notifier maintains a collection of IObserver instances that are notified
//! when the Notifier's Notify() function is called. The class is templated on
//! the same type that the IObserver expects.
template <typename T> class Notifier {
  public:
    //! Notifies all observers of a change.
    void Notify(const T &t) {
        for (auto &observer: observers_)
            observer->ProcessChange(t);
    }

    //! Adds an IObserver to notify.
    void AddObserver(IObserver<T> *observer) {
        observers_.push_back(observer);
    }

  private:
    //! List of IObserver instances to notify.
    std::vector<Util::IObserver<T> *> observers_;
};

//!@}

}  // namespace Util
