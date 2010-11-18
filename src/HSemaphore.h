#ifndef H_SEMAPHORE_H_
#define H_SEMAPHORE_H_
#ifdef __cplusplus

#import <dispatch/dispatch.h>

class HSemaphore {
 protected:
  dispatch_semaphore_t dsema_;
 public:
  HSemaphore(long initialValue=0) {
    dsema_ = dispatch_semaphore_create(initialValue);
  }
  ~HSemaphore() {
    dispatch_release(dsema_);
    dsema_ = NULL;
  }
  
  /// Access the underlying dispatch_semaphore_t struct
  inline dispatch_semaphore_t dsema() const { return dsema_; }
  
  /// Waits for (decrements) a semaphore.
  /// Returns zero on success, or non-zero if the timeout occurred.
  inline long get(dispatch_time_t timeout=DISPATCH_TIME_FOREVER) {
    return dispatch_semaphore_wait(dsema_, timeout);
  }
  
  /// Like wait, but returns immediately
  inline long tryGet() { return wait(DISPATCH_TIME_NOW); }
  
  /// Signals (increments) a semaphore.
  /// Returns non-zero if a thread is woken. Otherwise, zero is returned.
  inline long put() { return dispatch_semaphore_signal(dsema_); }
  
  // get-put scope
  class Scope {
   private:
    HSemaphore &sem_;
    Scope(Scope const &);
    Scope & operator=( Scope const & );
   public:
    explicit Scope(HSemaphore & sem,
                   dispatch_time_t timeout=DISPATCH_TIME_FOREVER) : sem_(sem) {
      sem_.get(timeout);
    }
    ~Scope() { sem_.put(); }
  };
};


/**
 * Critical section helper.
 *
 * Note that this is a light-weight construct which does not handle premature
 * break, like a return statement.
 *
 * Example:
 *
 *   HSemaphore sem;
 *   // do something non-critical here
 *   HSemaphoreSection(sem) {
 *     // we have aquired a reference to the semaphore
 *     // do critical stuff
 *   }
 *   // our reference to the semaphore has been released
 *
 */
#define HSemaphoreSection(sem) \
  for (_HSemaphoreSectionScope __ksss(sem); __ksss.getOnce() ; )
// See HSpinLock.h for a description of the algorithm


class _HSemaphoreSectionScope {
  HSemaphore &sem_;
  bool used_;
 public:
  explicit _HSemaphoreSectionScope(HSemaphore & sem) : sem_(sem), used_(false) {
  }
  ~_HSemaphoreSectionScope() { sem_.put(); }
  inline bool getOnce() {
    if (used_) return false;
    sem_.get();
    return (used_ = true);
  }
};

__attribute__((constructor)) void __HSemaphore_init() {
  HSemaphore sem;
  HSemaphoreSection(sem) {
    // lkalala
  }
}



#endif  // __cplusplus
#endif  // H_SEMAPHORE_H_