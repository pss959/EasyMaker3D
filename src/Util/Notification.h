#pragma once

namespace Util {

//! \name Notification
//!@{

// XXXX
template <typename T> class IObserver {
  public:
    virtual void Notify(const T &t) = 0;
};

// XXXX
template <typename T> class Notifier {
  public:
    //! Notifies all observers of a change.
    void Notify_(const T &t) {
        for (auto &observer: observers_)
            observer->Notify(t);
    }

    // XXXX
    void AddObserver(IObserver<T> *observer) {
        observers_.push_back(observer);
    }

  private:
    // XXXX For notification.
    std::vector<Util::IObserver<T> *> observers_;
};

//!@}

}  // namespace Util
