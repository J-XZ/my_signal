#include "my_signal.h"
#include <cassert>
#include <memory>
#include <mutex>

namespace my_signal {

void MySignal::RegisgerClearFn(void (*fn)(void *arg), void *arg) {
  clear_fn_list_.emplace_back(new ClearFn{fn, arg});
}

MySignal::ClearFn::~ClearFn() { fn(arg); }

MySignalGuard::~MySignalGuard() { s_.Wait(); }

void MySignal::RegisterWaitFn(void (*fn)(void *arg), void *arg) {
  wait_fn_list_.emplace_back(new ClearFn{fn, arg});
}

MySignal *MySignal::RegisterSubSignal() {
  sub_signal_list_.emplace_back(std::make_unique<MySignal>());
  return sub_signal_list_.back().get();
}

void MySignal::Wait() {  // NOLINT
  for (auto &sub_signal : sub_signal_list_) {
    sub_signal->Wait();
  }
  sub_signal_list_.clear();

  std::unique_lock<std::mutex> lock(m_);
  cv_.wait(lock, [this] {
    if (ready_) {
      ready_ = false;
      return true;
    }
    return false;
  });

  wait_fn_list_.clear();
}

void MySignal::Notify() {
  {
    std::lock_guard<std::mutex> lock(m_);
    assert(!ready_);
    ready_ = true;
  }
  cv_.notify_all();
}

}  // namespace my_signal