/* 2016-10-19 */
#ifndef BENCHMARK_ERROR_H_
#define BENCHMARK_ERROR_H_

#include <exception>
#include <string>

namespace benchmark {

class BenchmarkError : public std::exception {
public:
  BenchmarkError() noexcept : msg_("benchmark::") {}
  BenchmarkError(const BenchmarkError& other) noexcept : msg_(other.msg_) {}
  explicit BenchmarkError(const std::string& s) noexcept : 
    msg_("benchmark::" + s) {}
  explicit BenchmarkError(const char* s) noexcept : 
    msg_(std::string("benchmark::") + s) {}
  ~BenchmarkError() noexcept {}
  const char* what() const noexcept override { return msg_.c_str(); }
protected:
  std::string msg_;
};

}  // namespace benchmark

#endif  // BENCHMARK_ERROR_H_
