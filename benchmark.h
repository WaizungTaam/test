/* 2016-10-21 */
#ifndef BENCHMARK_BENCHMARK_H_
#define BENCHMARK_BENCHMARK_H_

#include <algorithm>
#include <cstddef>
#include <functional>
#include <numeric>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

#include "error.h"
#include "timer.h"
#include "time_unit.h"

namespace benchmark {

class Benchmark {
public:
  struct Result {
    std::string label;
    TimeUnit time_unit;
    std::size_t iterations;
    TimeValue duration;
    TimeValue mean;
    TimeValue variance;
    TimeValue max;
    TimeValue min;
  };

  const std::string label;

  Benchmark(const std::string& bm_label = "Benchmark") :
    label(bm_label), results_() {}

  virtual ~Benchmark() {}

  void add(const std::string& label, const std::string& unit_symbol,
           const Timer& timer) {
    Result res;
    res.label = label;
    res.time_unit = internal::to_time_unit(unit_symbol);
    res.iterations = timer.iterations();
    results_.push_back(res);
    timers_.push_back(timer);
  }
  void add(const std::vector<std::string>& labels,
           const std::vector<std::string>& unit_symbols,
           const std::vector<Timer>& timers) {
    if (labels.size() != unit_symbols.size() ||
        unit_symbols.size() != timers.size()) {
      throw BenchmarkError("Benchmark::measure: Inconsistent sizes.");
    }
    for (std::size_t i = 0; i < labels.size(); ++i) {
      add(labels[i], unit_symbols[i], timers[i]);
    }
  }

  virtual const Result& run(std::size_t index) {
    if (index >= results_.size()) {
      throw BenchmarkError("Benchmark::run: Index out of range.");
    }
    auto durations = timers_[index].durations();
    TimeUnit timer_time_unit = timers_[index].time_unit, 
             res_time_unit = results_[index].time_unit;
    auto to_value = [timer_time_unit, res_time_unit](
      const typename Timer::duration_type& d) {
      return internal::convert_time(timer_time_unit, res_time_unit, d.count());
    };
    results_[index].duration = to_value(sum(durations));
    results_[index].mean = to_value(mean(durations));
    results_[index].variance = to_value(variance(durations));
    results_[index].max = to_value(max(durations));
    results_[index].min = to_value(min(durations));
    return results_[index];    
  }
  virtual const std::vector<Result>& run() {
    for (std::size_t i = 0; i < results_.size(); ++i) {
      run(i);
    }
    return results_;
  }

  const Result& result(std::size_t index) const {
    if (index > results_.size()) {
      throw BenchmarkError("Benchmark::result: Index out of range.");
    }
    return results_.at(index);
  }
  inline const std::vector<Result>& result() const {
    return results_;
  }

protected:
  typedef typename Timer::duration_type duration_type;

  duration_type sum(const std::vector<duration_type>& d) {
    return std::accumulate(d.begin(), d.end(), duration_type(0));
  }
  duration_type mean(const std::vector<duration_type>& d) {
    return sum(d) / d.size();
  }
  duration_type variance(const std::vector<duration_type>& d) {
    duration_type mean_d = mean(d);
    duration_type res(0);
    for (const duration_type& t : d) {
      res += duration_type((t - mean_d).count() * (t - mean_d).count());
    }
    return res / d.size();
  }
  duration_type max(const std::vector<duration_type>& d) {
    return *std::max_element(d.begin(), d.end());
  }
  duration_type min(const std::vector<duration_type>& d) {
    return *std::min_element(d.begin(), d.end());
  }

  std::vector<Result> results_;
  std::vector<Timer> timers_;
};


template <typename... Parameters>
class FunctionBenchmark : public Benchmark {
public:
  FunctionBenchmark(const std::string& bm_label = "FunctionBenchmark") : 
    Benchmark(bm_label), functions_(), arguments_() {}

  template <typename Func, typename... Args>
  void add(const std::string& label, const std::string& unit_symbol,
           std::size_t iterations, Func func, Args&&... args) {
    functions_.push_back(std::function<void(Timer&, Parameters...)>(func));
    arguments_.push_back(std::tuple<Parameters...>(args...));
    Timer timer(iterations);
    Benchmark::add(label, unit_symbol, timer);
  }
  template <typename Func, typename... Args>
  void add(const std::vector<std::string>& labels, 
           const std::vector<std::string>& unit_symbols,
           const std::vector<std::size_t>& iterations,
           const std::vector<Func>& funcs,
           const std::vector<std::tuple<Args...>>& args) {
    if (labels.size() != unit_symbols.size() ||
        unit_symbols.size() != iterations.size() ||
        iterations.size() || funcs.size() ||
        funcs.size() || args.size()) {
      throw BenchmarkError(
        "FunctionBenchmark::add: Inconsistent sizes.");
    }
    for (std::size_t i = 0; i < labels.size(); ++i) {
      add(labels[i], unit_symbols[i], iterations[i], funcs[i], args[i]);
    }
  }

  const Result& run(std::size_t index) override {
    if (index > results_.size()) {
      throw BenchmarkError("FunctionBenchmark::run: Index of of range.");
    }
    invoke(functions_[index], timers_[index], arguments_[index]);
    return Benchmark::run(index);
  }
  const std::vector<Result>& run() override {
    for (std::size_t i = 0; i < results_.size(); ++i) {
      run(i);
    }
    return results_;
  }

private:
  template <typename Func, typename Tuple, bool Finsihed, int Total, int... N>
  struct InvokeImpl {
    static void invoke(Func func, Timer& timer, Tuple&& t) {
      InvokeImpl<Func, Tuple, Total == 1 + sizeof...(N), Total, N..., 
                sizeof...(N)>::invoke(func, timer, std::forward<Tuple>(t));
    }
  };
  template <typename Func, typename Tuple, int Total, int... N>
  struct InvokeImpl<Func, Tuple, true, Total, N...> {
    static void invoke(Func func, Timer& timer, Tuple&& t) {
      func(timer, std::get<N>(std::forward<Tuple>(t))...);
    }
  };
  template <typename Func, typename Tuple>
  void invoke(Func func, Timer& timer, Tuple&& t) {
    typedef typename std::decay<Tuple>::type DTuple;
    InvokeImpl<Func, Tuple, 0 == std::tuple_size<DTuple>::value,
      std::tuple_size<DTuple>::value>::invoke(
        func, timer, std::forward<Tuple>(t));
  }

  std::vector<std::function<void(Timer&, Parameters...)>> functions_;
  std::vector<std::tuple<Parameters...>> arguments_;
};

}  // namespace benchmark

#endif  // BENCHMARK_BENCHMARK_H_
