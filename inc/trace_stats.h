#ifndef _TRACE_STATS_H_
#define _TRACE_STATS_H_

#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <atomic>
#include <set>
#include <map>
#include <vector>
#include <memory>
#include <numeric>
#include <limits>

#include <globals.h>
#include <config.h>

class TraceStats {
private:

	std::shared_ptr<Config> config;

	// OTF Trace specific parameters
	struct OTF_Trace_Param {
		std::string creator      {};
		uint64_t time_begin      {0};
		uint64_t time_end        {0};
		uint64_t time_resolution {0}; // ticks per second
	} otfparam {};

	// Proc Defs
	struct ProcessParameters {
		std::string name   {};
		uint32_t    parent {};
	};
	std::map<uint32_t, ProcessParameters> process_map {};

	// Proc Group Defs
	struct ProcessGroupParameters {
		std::string name           {"noname"};
		uint64_t numMembers        {0};
		std::set<uint32_t> members {};
	};
	std::map<uint32_t, ProcessGroupParameters> process_group_map {};

	// Func Defs
	struct FunctionParameters {
		uint32_t    group  {};
		std::string name   {};
		uint32_t    source {};
	};
	std::map<uint32_t, std::string> function_group_map {};
	std::map<uint32_t, FunctionParameters> function_map {};

	// Counter Defs
	struct CounterParameters {
		uint32_t    group {};
		std::string name  {};
		std::string unit  {};
	};
	std::map<uint32_t, std::string> counter_group_map {};
	std::map<uint32_t, CounterParameters> counter_map {};

	// Message Statistics
	struct MessageStatistics {
		struct DirectionStats {
			uint64_t msgs  {0};
			uint64_t bytes {0};
			uint64_t min   {std::numeric_limits<uint64_t>::max()};
			uint64_t max   {std::numeric_limits<uint64_t>::min()};
			std::map<uint64_t, uint64_t> sizemap {};
			void updateMinMax(uint64_t len)
			{
				min = len < min ? len : min;
				max = len > max ? len : max;
			}
		};
		DirectionStats sent {};
		DirectionStats recv {};
	};
	std::map<uint32_t, MessageStatistics> msg_stats {};

	// Node idle (phases with no messages issued)
	struct MessageGaps {
		uint64_t min {std::numeric_limits<uint64_t>::max()};
		uint64_t max {std::numeric_limits<uint64_t>::min()};
		std::vector<uint64_t> avg {0};
		uint64_t total {0};

		uint64_t last {0};

		void update(uint64_t time)
		{
			// calculate gap to last event
			auto gap = time - last;
			last = time;

			// update min/max/avg/total
			min = gap < min ? gap : min;
			max = gap > max ? gap : max;

			avg.push_back(gap);
			total += gap;
		}

		double getMin(void) { return min; }

		double getMax(void) { return max; }

		double getAvg(void)
		{
			double sum = 0.0;
			for (auto x:avg)
				sum += x;

			return (sum / avg.size());
		}

		double getTot(void)
		{
			return total;
		}
	};
	std::map<uint32_t, MessageGaps> node_idle {};

	// Function Statistics
	struct FunctionStatistics {
		uint64_t enter {std::numeric_limits<uint64_t>::max()};
		uint64_t time  {0};
		uint64_t calls {0};
	};
	std::map<uint32_t, // process
	std::map<uint32_t, // group
	std::map<uint32_t, // function
	FunctionStatistics
	>>> fkt_stats {};

	// Collective Defs
	struct CollectiveParameters {
		uint32_t type    {};
		std::string name {};
	};
	std::map<uint32_t, CollectiveParameters> coll_map {};

	// Collective Statistics
	struct CollectiveStatistics {
		uint64_t sent  {0};
		uint64_t recv  {0};
		uint64_t calls {0};
	};

	std::map<uint32_t, // communicator
	std::map<uint32_t, // op
	CollectiveStatistics
	>> coll_stats {};

	// Performance Counter
	std::map<uint32_t, std::map<std::string, uint64_t>> papi_counter {};

public:
	TraceStats(std::shared_ptr<Config> cfg);
	~TraceStats();

	// Definitions
	void addOtfCreator(std::string creator);
	void addOtfTimeRange(uint64_t begin, uint64_t end);
	void addOtfResolution(uint64_t res);

	void addProcessGroup(uint32_t id, std::string name, uint32_t numMembers, std::set<uint32_t> members);
	void addProcess(uint32_t id, std::string name, uint32_t parent);
	void addFunctionGroup(uint32_t id, std::string name);
	void addFunction(uint32_t id, std::string name, uint32_t group, uint32_t source);
	void addCounterGroup(uint32_t id, std::string name);
	void addCounter(uint32_t id, std::string name, std::string unit, uint32_t group);
	void addCollective(uint32_t id, uint32_t type, std::string name);
	void addPapiCounter(uint32_t proc, uint32_t counter, uint64_t value);

	// Events
	void addCollectiveEvent(uint32_t proc, uint32_t communicator, uint32_t operation, uint32_t sent, uint32_t recv, uint64_t time);
	void addSendMsg(uint32_t proc, uint32_t len, uint64_t time);
	void addRecvMsg(uint32_t proc, uint32_t len, uint64_t time);
	void addFktEnter(uint32_t proc, uint32_t func, uint64_t time);
	void addFktLeave(uint32_t proc, uint32_t func, uint64_t time);

	// Global stuff
	OTF_Trace_Param getOtfParam(void);

	double   getRelativeTime(uint64_t time);
	double   getAbsoluteTime(uint64_t time);
	uint64_t toNanoS(uint64_t time);

	uint32_t getNumProcesses(void);
	uint32_t getNumFunctions(void);
	uint64_t getNumSentGlobal(void);
	uint64_t getNumRecvGlobal(void);
	uint64_t getBytesSentGlobal(void);
	uint64_t getBytesRecvGlobal(void);
	double   getApplicationTime(void);

	std::string        getCommunicatorName(uint32_t groupid);
	std::set<uint32_t> getCommunicatorMembers(uint32_t groupid);
	
	std::string getCollectiveName(uint32_t collid);
	uint32_t    getCollectiveType(uint32_t collid);

	uint32_t                     getCommunicatorSize(uint32_t groupid);
	std::set<uint32_t>           getCollectiveCommunicators(void);
	std::map<uint32_t, uint32_t> getCollectiveCommunicatorsMap(void);

	// Node specific
	uint64_t    getNumSent(uint32_t pid);
	uint64_t    getBytesSent(uint32_t pid);
	uint64_t    getNumRecv(uint32_t pid);
	uint64_t    getBytesRecv(uint32_t pid);
	std::string getCounterName(uint32_t cid);
	std::string getCounterUnit(uint32_t cid);
	std::string getFunctionName(uint32_t fid);
	std::string getGroupName(uint32_t gid);
	uint32_t    getFunctionGroup(uint32_t fid);
	uint64_t    getFunctionCalls(uint32_t pid, uint32_t fid);
	double      getFunctionTime(uint32_t pid, uint32_t fid);
	uint64_t    getFlops(uint32_t proc);

private:
	std::string map2table(const std::string title, std::map<uint32_t, std::string>& container);
	std::string map2table(const std::string title, std::map<uint32_t, ProcessParameters>& container);
	std::string map2table(const std::string title, std::map<uint32_t, ProcessGroupParameters>& container);
	std::string map2table(const std::string title, std::map<uint32_t, FunctionParameters>& container);
	std::string map2table(const std::string title, std::map<uint32_t, CounterParameters>& container);
	std::string msgs2table(const std::string title, std::map<uint32_t, MessageStatistics>& container);
	std::string fkts2table(const std::string title, std::map<uint32_t,std::map<uint32_t,std::map<uint32_t, FunctionStatistics>>>& container);
	std::string coll2table(const std::string title, std::map<uint32_t, std::map<uint32_t, CollectiveStatistics>>& container);

public:
	void print(void);
	bool validate(const bool printErrors);
};

#endif
