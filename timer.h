/* 2016-10-21 */
#ifndef BENCHMARK_TIMER_H_
#define BENCHMARK_TIMER_H_

#include <chrono>
#include <cstddef>
#include <string>
#include <vector>

#include "error.h"
#include "time_unit.h"

namespace benchmark {
namespace internal {
template <bool IsSteady = std::chrono::high_resolution_clock::is_steady>
struct SteadyClock {
  typedef std::chrono::high_resolution_clock type;
};
template <>
struct SteadyClock<false> {
  typedef std::chrono::steady_clock type;
};
}  // namespace internal

class Timer {
public:
  typedef internal::SteadyClock<>::type clock_type;
  typedef std::chrono::nanoseconds duration_type;
  static constexpr const TimeUnit time_unit = TimeUnit::ns;

  const std::string label;

  Timer(std::size_t iter = 1) : 
    label("timer"),
    is_started_(false),
    is_stopped_(false),
    is_running_(false), 
    iterations_(iter),
    num_iterated_(0),
    start_time_(),
    duration_(0),
    old_duration_(0),
    loop_durations_() {
    if (iterations_ == 0) iterations_ = 1;
  }
  Timer(const std::string& timer_label, std::size_t iter = 1) :
    label(timer_label),
    is_started_(false),
    is_stopped_(false),
    is_running_(false), 
    iterations_(iter),
    num_iterated_(0),
    start_time_(),
    duration_(0),
    old_duration_(0),
    loop_durations_() {
    if (iterations_ == 0) iterations_ = 1;
  }

  /* Pre-condition: is_started_ == false and 
   *                is_stopped_ == false and
   *                is_running_ == false
   */
  void start() {
    if (!(!is_started_ && !is_stopped_ && !is_running_)) {
      throw BenchmarkError("Timer::start: Invalid pre-condition.");
    }
    is_started_ = true;
    is_running_ = true;
    start_time_ = clock_type::now();
  }

  /* Pre-condition: is_started_ == true and 
   *                is_stopped_ == false
   */
  void stop() {
    clock_type::time_point stop_time_point = clock_type::now();
    if (!(is_started_ && !is_stopped_)) {
      throw BenchmarkError("Timer::stop: Invalid pre-condition.");
    }
    if (is_running_) {
      duration_ += std::chrono::duration_cast<duration_type>
                   (stop_time_point - start_time_);
      is_running_ = false;
    }
    is_stopped_ = true;
  }

  /* Pre-condition: is_started_ == true and 
   *                is_stopped_ == false and
   *                is_running_ == true
   */
  void pause() {
    clock_type::time_point pause_time_point = clock_type::now();
    if (!(is_started_ && !is_stopped_ && is_running_)) {
      throw BenchmarkError("Timer::pause: Invalid pre-condition.");
    }
    duration_ += std::chrono::duration_cast<duration_type>
                 (pause_time_point - start_time_);
    is_running_ = false;
  }

  /* Pre-condition: is_started_ == true and 
   *                is_stopped_ == false and
   *                is_running_ == false
   */
  void resume() {
    if (!(is_started_ && !is_stopped_ && !is_running_)) {
      throw BenchmarkError("Timer::resume: Invalid pre-condition.");
    }
    is_running_ = true;
    start_time_ = clock_type::now();
  }

  bool looping() {
    bool was_running = is_running_;
    if (is_started_ && !is_stopped_ && is_running_) pause();
    if (num_iterated_ != 0) {
      loop_durations_.push_back(duration_ - old_duration_);
      old_duration_ = duration_;
    }
    if (is_stopped_) return false;
    if (num_iterated_ >= iterations_) {
      stop();
      return false;
    }
    if (num_iterated_ == 0) {
      start();
      pause();
      was_running = true;
      old_duration_ = duration_;
    }    
    ++num_iterated_;
    if (was_running) resume();
    return true;
  }

  void reset(std::size_t iter) {
    is_started_ = false;
    is_stopped_ = false;
    is_running_ = false;
    iterations_ = iter;
    num_iterated_ = 0;
    start_time_ = clock_type::time_point();
    duration_ = duration_type(0);
    old_duration_ = duration_type(0),
    loop_durations_.clear();
  }
  void reset() { reset(iterations_); }

  /* Pre-condition: is_running_ == false
   */
  duration_type duration() const {
    if (!(!is_running_)) {
      throw BenchmarkError("Timer::duration: "
            "Cannot get duration while the timer is still running.");
    }
    return duration_;
  }
  inline std::size_t iterations() const { return iterations_; }
  inline std::size_t iter_index() const {
    return num_iterated_ == 0 ? 0 : num_iterated_ - 1;
  }

  /* Pre-condition: is_running_ = false
   */
  std::vector<duration_type> durations() const {
    if (!(!is_running_)) {
      throw BenchmarkError("Timer::loop_durations: "
            "Cannot get loop durations while the timer is still running.");
    }
    if (iterations_ == 1) return std::vector<duration_type>{duration_};
    return loop_durations_;
  }

private:
  bool is_started_;
  bool is_stopped_;
  bool is_running_;

  std::size_t iterations_;
  std::size_t num_iterated_;

  clock_type::time_point start_time_;
  duration_type duration_;
  duration_type old_duration_;  // duration_ in the last looping statement
  std::vector<duration_type> loop_durations_;
};

}  // namespace benchmark

#endif  // BENCHMARK_TIMER_H_
