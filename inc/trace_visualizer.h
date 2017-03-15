#ifndef _TRACE_VIZ_H_
#define _TRACE_VIZ_H_

#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <set>
#include <map>
#include <string>
#include <utility>
#include <thread>
#include <memory>
#include <mutex>
#include <condition_variable>
#include <atomic>

#include <globals.h>
#include <config.h>
#include <make_unique.h>
#include <trace_stats.h>

class TraceVisualizer {
private:

	const uint32_t procEnumFill {4}; // process enumeration in filenames: p1 --> p0001

	const char gnuplot_seperator {','};
	const std::string gnuplot_inj_filename_prefix {"inj-p"};
	const std::string gnuplot_cdf_filename_prefix {"cdf-p"};
	const std::string gnuplot_iahist_filename_prefix {"iahist-p"};
	bool gnuplot_present {false};
	bool rscript_present {false};

	std::shared_ptr<Config>     config;
	std::shared_ptr<TraceStats> stats;

public:
	TraceVisualizer(std::shared_ptr<Config> cfg, std::shared_ptr<TraceStats> ts);
	~TraceVisualizer();

// message injection diagrams
private:
	struct InjData {
		double time_absolute; // seconds
		double time_relative; // percent
		uint64_t bytes;
	};
	enum Direction {P2P_SEND, P2P_RECV, COLL_SEND, COLL_RECV};
	std::map<uint32_t, std::map<Direction, std::vector<InjData>>> injections {};

// inactivity periods
private:
	std::map<uint32_t, uint64_t> lastActivity {};
	std::map<uint32_t, std::vector<uint64_t>> inactivity_periods {};
public:
	void updateInactPeriods(uint32_t proc, uint64_t time);
	void makeInactivityHistogram(std::string dirname);

public:
	void addSendP2P(uint32_t proc, uint64_t time, uint64_t bytes);
	void addRecvP2P(uint32_t proc, uint64_t time, uint64_t bytes);
	void addColl(uint32_t proc, uint64_t time, uint64_t sent, uint64_t recv);
	void makeInjPlot(std::string dirname);

// message CDF diagrams
private:
	enum MsgType {P2P, COLL};
	std::map<uint32_t, std::map<MsgType, std::map<uint64_t, uint64_t>>> messages_cdf {};
	std::map<MsgType, std::map<uint64_t, uint64_t>> messages_cdf_allnodes {};
public:
	void addMessageCDF_P2P(uint32_t proc, uint64_t msg_len);
	void addMessageCDF_COLL(uint32_t proc, uint64_t msg_len);
	void makeCdfPlot(std::string dirname);
};

#endif
