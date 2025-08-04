#pragma once

#include <cassert>
#include <condition_variable>

#if defined(__x86_64__) || defined(_M_X64) || defined(__i386) || \
    defined(_M_IX86)
#include <emmintrin.h>
#define MY_SIGNAL_MFENCE() _mm_mfence()
#else
// ARM等平台，用GCC内建原子指令替代
#define MY_SIGNAL_MFENCE() __sync_synchronize()
#endif

#include <memory>
#include <mutex>
#include <vector>

namespace my_signal {

class MySignal {
 public:
  class ClearFn {
   public:
    ~ClearFn();

    void (*fn)(void* arg);
    void* arg;
  };

  MySignal() = default;

  ~MySignal() = default;

  void Wait();

  void Notify();

  void RegisgerClearFn(void (*fn)(void* arg), void* arg);

  MySignal* RegisterSubSignal();

  void RegisterWaitFn(void (*fn)(void* arg), void* arg);

  // void RegisterBufferUntilWaitOK(void *buffer) {
  //   RegisterWaitFn(
  //     [](void *data) {
  //       char *d = static_cast<char *>(data);
  //       delete[] d;
  //     },
  //     buffer
  //   );
  // }

  template <class T>
  T& HoldDataUntilWaitOK(T&& data) {
    struct Holder {
      T data;
    };

    auto* holder = new Holder{};
    holder->data = std::move(data);  // NOLINT
    data.clear();
    MY_SIGNAL_MFENCE();
    RegisterWaitFn(
        [](void* arg) {
          auto holder = static_cast<Holder*>(arg);
          delete holder;
        },
        (void*)holder  // NOLINT
    );
    return holder->data;
  }

 private:
  std::mutex              m_;
  bool                    ready_ = false;
  std::condition_variable cv_;

  std::vector<std::unique_ptr<ClearFn>>  clear_fn_list_;
  std::vector<std::unique_ptr<ClearFn>>  wait_fn_list_;
  std::vector<std::unique_ptr<MySignal>> sub_signal_list_;
};

class MySignalGuard {
 public:
  ~MySignalGuard();

  inline MySignal* GetSignal() { return &s_; }

 private:
  MySignal s_;
};
}  // namespace my_signal
