/* 2016-10-20 */
#ifndef BENCHMARK_TIME_UNIT_H_
#define BENCHMARK_TIME_UNIT_H_

#include <string>

namespace benchmark {

enum TimeUnit { ns, us, ms, s, unknown };
typedef double TimeValue;

namespace internal {

std::string time_unit_symbol(TimeUnit unit) {
  switch (unit) {
    case ns: return "ns"; break;
    case us: return "us"; break;
    case ms: return "ms"; break;
    case s:  return "s";  break;
    default: return "unknown";
  }
}

TimeUnit to_time_unit(const std::string& symbol) {
  if (symbol == "ns") return TimeUnit::ns;
  else if (symbol == "us") return TimeUnit::us;
  else if (symbol == "ms") return TimeUnit::ms;
  else if (symbol == "s") return TimeUnit::s;
  else return TimeUnit::unknown;
}

TimeValue multiplier[5][5] = {{   1, 1e-3, 1e-6, 1e-9, 1},
                              { 1e3,    1, 1e-3, 1e-6, 1},
                              { 1e6,  1e3,    1, 1e-3, 1},
                              { 1e9,  1e6,  1e3,    1, 1},
                              {   1,    1,    1,    1, 1}};

template <typename TimeValue1>
TimeValue convert_time(TimeUnit unit_1, TimeUnit unit_2, TimeValue1 t) {
  return multiplier[unit_1][unit_2] * t;
}

}  // namespace internal

};  // namespace benchmark

#endif  // BENCHMARK_TIME_UNIT_H_
