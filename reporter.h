#ifndef BENCHMARK_REPORTER_H_
#define BENCHMARK_REPORTER_H_

#include <fstream>
#include <iostream>
#include <string>

#include "benchmark.h"
// #include "experiment.h"
#include "error.h"
#include "timer.h"
#include "time_unit.h"

namespace benchmark {

class Reporter {
public:
	virtual ~Reporter() {}
  virtual void report(const Timer& timer) const = 0;
  virtual void report(const Benchmark& benchmark) const = 0;
  // virtual void report(const Experiment& experiment) = 0;
};

class ConsoleReporter : public Reporter {

};

class FileReporter : public Reporter {
public:
	FileReporter(const std::string& filename) : filename_(filename) {}

	virtual void report(const Timer& timer) const {}
	virtual void report(const Benchmark& benchmark) const {}
	// void report(const Experiment& experiment) {}

protected:
	std::string filename_;
};

class JsonReporter : public FileReporter {
public:
	JsonReporter(const std::string& filename) : FileReporter(filename) {}

	void report(const Timer& timer) const override {
		report_aux(timer);
	}
	void report(const Benchmark& benchmark) const override {
		report_aux(benchmark);
	}
	// void report(const Experiment& experiment) const override {
	// 	std::ofstream file(filename_);
	// 	std::ofstream file(filename_);
	// 	if (!file.is_open()) {
	// 		throw BenchmarkError("JsonReporter::report: Cannot open file.");
	// 	}
	// 	file << "{\n";
	// 			 << "  Experiment: {\n"
	// 			 << "    \"label\": \"" << experiment.label << "\"\n"
	// 			 << "    \"benchmarks\": [\n";
	// 	for (const Benchmark& bm : experiment.benchmarks_) {
	// 		report_aux(bm, file, "  ");
	// 	}
	// 	file << "    ]\n"
	// 	     << "  }\n"
	// 	     << "}\n";
	// 	file.close();
	// }

private:
	template <typename Tp>
	void report_aux(const Tp& x) const {
		std::ofstream file(filename_);
		if (!file.is_open()) {
			throw BenchmarkError("JsonReporter::report: Cannot open file.");
		}
		file << "{\n";
		report_aux(x, file, "");
		file << "}\n";
		file.close();		
	}
	void report_aux(const Timer& timer, std::ofstream& file, 
									const std::string& indent) const {
		file << indent << "  \"Timer\": {\n"
				 << indent << "    \"label\": \"" << timer.label << "\",\n"
				 << indent << "    \"time_unit\": \"ns\",\n"
				 << indent << "    \"duration\": " << timer.duration().count() << ",\n"
				 << indent << "    \"iterations\": " << timer.iterations() << ",\n"
				 << indent << "    \"loop_durations\": [";
		auto durations = timer.durations();
		for (std::size_t i = 0; i < durations.size() - 1; ++i) {
			file << durations[i].count() << ", ";
		}
		file << durations[durations.size() - 1].count() << "]\n"
		     << indent << "  }\n";
	}
	void report_aux(const Benchmark& benchmark, std::ofstream& file, 
									const std::string& indent) const {
		file << indent << "  \"Benchmark\": {\n"
		     << indent << "    \"label\": \"" << benchmark.label << "\",\n"
		     << indent << "    \"item\": [\n";
		auto results = benchmark.result();
		for (auto it = results.cbegin(); it != results.cend(); ++it) {
			file << indent << "      {\n"
					 << indent << "        \"label\": \"" << it->label << "\",\n"
					 << indent << "        \"time_unit\": \"" 
					 << indent << internal::time_unit_symbol(it->time_unit) << "\",\n"
					 << indent << "        \"iterations\": " << it->iterations << ",\n"
					 << indent << "        \"duration\": " << it->duration << ",\n"
					 << indent << "        \"mean\": " << it->mean << ",\n"
					 << indent << "        \"variance\": " << it->variance << ",\n"
					 << indent << "        \"max\": " << it->max << ",\n"
					 << indent << "        \"min\": " << it->min << "\n"
					 << indent << "      }";
			if (it != results.cend() - 1) {
				file << ",";
			}
			file << "\n";
		}
		file << indent << "    ]\n"
				 << indent << "  }\n";;
	}
};


}  // namespace benchmark

#endif  // BENCHMARK_REPORTER_H_
