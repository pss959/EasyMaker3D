#include "Util/Delay.h"

#include <chrono>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

#include "Util/Assert.h"
#include "Util/General.h"
#include "Util/KLog.h"

#include <iostream>

// ----------------------------------------------------------------------------
// Delayed thread helpers.
// ----------------------------------------------------------------------------

namespace {

/// Convenience typedef for a function to execute in a delayed thread.
typedef std::function<void()> ExecFunc_;

// ----------------------------------------------------------------------------
// Thread_ class.
// ----------------------------------------------------------------------------

/// This class wraps an std::thread and an std::condition variable so that the
/// thread can be canceled.
class Thread_ {
  public:
    /// The constructor is passed an integer ID, a function to execute, and the
    /// time in seconds to delay before executing it.
    Thread_(int id, const ExecFunc_ &func, float seconds);

    /// The destructor cancels the thread so that it is no longer waiting and
    /// joins the thread so it goes away.
    ~Thread_();

    /// Returns the ID for the thread.
    int GetID() const { return id_; }

    /// Cancels the thread; it stops waiting and will not execute the function.
    void Cancel();

    /// Returns true if the thread has finished, either by timeout or
    /// cancellation.
    bool IsFinished() const { return state_ != State_::kWaiting; }

  private:
    enum class State_ {
        kWaiting,    ///< Delaying for specified time.
        kCanceled,   ///< Canceled before time was up.
        kFinished,   ///< Wait finished, function called.
    };

    const int               id_;      ///< ID assigned in the constructor.
    State_                  state_;   ///< Maintains the current state.
    std::thread             thread_;  ///< Thread created in the constructor.
    std::condition_variable cv_;      ///< Implements wait/interrupt.
    std::mutex              mutex_;   ///< Mutex protecting state_ and cv_.

    /// Function that is executed in the worker thread.
    void ExecuteDelayed_(const ExecFunc_ &func, float seconds);
};

Thread_::Thread_(int id, const ExecFunc_ &func, float seconds) :
    id_(id),
    state_(State_::kWaiting),
    thread_([this, func, seconds]{ ExecuteDelayed_(func, seconds); }) {
}

Thread_::~Thread_() {
    // If the thread is still waiting, cancel it.
    Cancel();

    // Join the thread so it is deleted.
    KLOG('t', "Thread " << id_ << " deleted");
    thread_.join();
}

void Thread_::Cancel() {
    // Lock to protect the state and CV, update the state, and notify the CV to
    // interrupt its wait.
    std::unique_lock<std::mutex> lock(mutex_, std::defer_lock);
    if (state_ != State_::kCanceled) {
        state_ = State_::kCanceled;
        cv_.notify_all();
    }
}

void Thread_::ExecuteDelayed_(const ExecFunc_ &func, float seconds) {
    // Check the state under the mutex.
    std::unique_lock<std::mutex> lock(mutex_, std::defer_lock);

    // Start waiting if not canceled.
    if (state_ == State_::kWaiting) {
        cv_.wait_for(lock, std::chrono::duration<float>(seconds));

        // It gets here when the wait is done or canceled.
        if (state_ == State_::kWaiting) {
            state_ = State_::kFinished;
            KLOG('t', "Thread " << id_ << " invoking function");
            func();
        }
    }
}

// ----------------------------------------------------------------------------
// ThreadManager_ class.
// ----------------------------------------------------------------------------

/// The ThreadManager_ keeps track of running threads and cleans them up when
/// necessary.
class ThreadManager_ {
  public:
    /// Creates a new worker thread that delays for the given time or until it
    // is killed. If it times out, it executes the given function. Returns an
    // ID that can be used to kill the thread.
    int Create(float seconds, const ExecFunc_ &func);

    /// Cancels the thread with the given ID if it exists and is still running.
    /// Returns true if the thread was canceled.
    bool Cancel(int id);

    /// Returns the number of waiting threads.
    size_t GetWaitingCount() {
        RemoveFinished_();
        return threads_.size();
    }

    /// Resets to starting condition.
    void Reset() {
        threads_.clear();
        next_id_ = 0;
    }

  private:
    typedef std::shared_ptr<Thread_> ThreadPtr_;

    /// Vector of std::shared_ptr to Thread_ instances. Each Thread_ may or may
    /// not be waiting.
    std::vector<ThreadPtr_> threads_;

    /// Integer ID of the next thread. This increases for each new thread.
    int next_id_ = 0;

    /// This is invoked for a thread when the delay time is reached.
    void Finish_(int id, const ExecFunc_ &func);

    /// Ends and removes threads that have finished from the vector.
    void RemoveFinished_();
};

int ThreadManager_::Create(float seconds, const ExecFunc_ &func) {
    RemoveFinished_();
    const int id = next_id_++;
    threads_.push_back(ThreadPtr_(new Thread_(id, func, seconds)));
    KLOG('t', "Started thread " << id
         << " with " << seconds << "s delay");
    return id;
}

bool ThreadManager_::Cancel(int id) {
    auto match_id = [id](const ThreadPtr_ &tp){ return tp->GetID() == id; };
    auto it = std::find_if(threads_.begin(), threads_.end(), match_id);
    bool ret = false;
    if (it != threads_.end()) {
        KLOG('t', "Canceling thread " << id);
        (*it)->Cancel();
        ret = true;
    }
    RemoveFinished_();
    return ret;
}

void ThreadManager_::RemoveFinished_() {
    auto is_finished = [](const ThreadPtr_ &tp){ return tp->IsFinished(); };
    Util::EraseIf(threads_, is_finished);
}

/// Static ThreadManager_ instance.
static ThreadManager_ s_thread_manager_;

}  // anonymous namespace

// ----------------------------------------------------------------------------
// Public functions.
// ----------------------------------------------------------------------------

namespace Util {

int RunDelayed(float seconds, const ExecFunc_ &func) {
    return s_thread_manager_.Create(seconds, func);
}

bool CancelDelayed(int id) {
    return s_thread_manager_.Cancel(id);
}

bool IsAnyDelaying() {
    return s_thread_manager_.GetWaitingCount() > 0;
}

void ResetDelay() {
    s_thread_manager_.Reset();
}

}  // namespace Util
