/*

Experiment EX_sort("Sorting Algorithm Comparison");
EX_sort.add(BM_sort_small);
EX_sort.add(BM_sort_middle);
EX_sort.add(BM_sort_large);
EX_sort.add({BM_sort_sorted, BM_sort_reversed, BM_sort_same, BM_sort_sparse});
EX_sort.run();
EX_sort.report(json_reporter);

*/

#ifndef BENCHMARK_EXPERIMENT_H_
#define BENCHMARK_EXPERIMENT_H_

#include <string>
#include "benchmark.h"

namespace benchmark {

class Experiment {
public:
  const std::string label;

  Experiment(const std::string& ex_label) : 
    label(ex_label), benchmarks_(), reporter_() { initialize(); }
  virtual ~Experiment() { finialize(); }

  virtual void add(const Benchmark& benchmark) {
    benchmarks_.push_back(&benchmark);
  }
  virtual void run() {
  	for (Benchmark* bm : benchmarks_) {
  		bm->run();
  	}
  }

protected:
  virtual void initialize() {}
  virtual void finialize() {}

  std::vector<Benchmark*> benchmarks_;
};

}  // namespace benchmark

#endif  // BENCHMARK_EXPERIMENT_H_