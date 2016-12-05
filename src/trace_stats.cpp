#include <trace_stats.h>

static inline void cutString(std::string& str, uint32_t cutTo)
{
	if (str.size() > cutTo)
	{
		str.resize(cutTo);
		str[cutTo-1] = '.';
		str[cutTo-2] = '.';
		str[cutTo-3] = '.';
		str[cutTo-4] = ' ';
	}
}

static inline double average(std::vector<uint64_t> v)
{
	uint64_t sum = std::accumulate(v.begin(), v.end(), 0.0);
	return static_cast<double>(sum) / static_cast<double>(v.size());
}

static inline double average(std::vector<double> v)
{
	double sum = std::accumulate(v.begin(), v.end(), 0.0);
	return sum / v.size();
}

struct NodeMetrics {
	std::vector<double> verbosity {};
	std::vector<double> msgrate {};
	std::vector<double> mpi_idle_min {};
	std::vector<double> mpi_idle_max {};
	std::vector<double> mpi_idle_avg {};
	std::vector<double> mpi_idle_tot {};
	std::vector<uint64_t> msg_tx {};
	std::vector<uint64_t> msg_rx {};
	std::vector<uint64_t> bytes_tx {};
	std::vector<uint64_t> bytes_rx {};
	std::vector<std::map<std::string, uint64_t>> papi {};
};
NodeMetrics metrics {};

TraceStats::TraceStats(std::shared_ptr<Config> cfg) :
	config(cfg)
{
#ifdef DEBUG
	ctor_msg(__PRETTY_FUNCTION__);
#endif
}

TraceStats::~TraceStats()
{
#ifdef DEBUG
	dtor_msg(__PRETTY_FUNCTION__);
#endif
}

void TraceStats::addOtfCreator(std::string creator)
{
	otfparam.creator = creator;
}

void TraceStats::addOtfTimeRange(uint64_t begin, uint64_t end)
{
	otfparam.time_begin = begin;
	otfparam.time_end   = end;
}

void TraceStats::addOtfResolution(uint64_t res)
{
	otfparam.time_resolution = res;
}

void TraceStats::addProcessGroup(uint32_t id, std::string name, uint32_t numMembers, std::set<uint32_t> members)
{
	process_group_map[id].name       = name;
	process_group_map[id].numMembers = numMembers;
	process_group_map[id].members    = members;
}

void TraceStats::addProcess(uint32_t id, std::string name, uint32_t parent)
{
	process_map[id].name = name;
	process_map[id].parent = parent;
}

void TraceStats::addFunctionGroup(uint32_t id, std::string name)
{
	function_group_map[id] = name;
}

void TraceStats::addFunction(uint32_t id, std::string name, uint32_t group, uint32_t source)
{
	function_map[id].name = name;
	function_map[id].group = group;
	function_map[id].source = source;
}

void TraceStats::addCounterGroup(uint32_t id, std::string name)
{
	counter_group_map[id] = name;
}

void TraceStats::addCounter(uint32_t id, std::string name, std::string unit, uint32_t group)
{
	counter_map[id].group = group;
	counter_map[id].name  = name;
	counter_map[id].unit  = unit;
}

void TraceStats::addCollective(uint32_t id, uint32_t type, std::string name)
{
	coll_map[id].name = name;
	coll_map[id].type = type;
}

void TraceStats::addPapiCounter(uint32_t proc, uint32_t counter, uint64_t value)
{
	const std::string PAPI_OP = getCounterName(counter);

	if (PAPI_OP.substr(0, 4) != "PAPI")
		return;

	papi_counter[proc][PAPI_OP] = value;
}

void TraceStats::addCollectiveEvent(uint32_t proc, uint32_t communicator, uint32_t operation, uint32_t sent, uint32_t recv, uint64_t time)
{
	coll_stats[communicator][operation].calls++;
	coll_stats[communicator][operation].sent += sent;
	coll_stats[communicator][operation].recv += recv;

	node_idle[proc].update(time);
}

void TraceStats::addSendMsg(uint32_t proc, uint32_t len, uint64_t time)
{
	msg_stats[proc].sent.msgs++;
	msg_stats[proc].sent.bytes += len;
	msg_stats[proc].sent.sizemap[len]++;
	msg_stats[proc].sent.updateMinMax(len);

	node_idle[proc].update(time);
}

void TraceStats::addRecvMsg(uint32_t proc, uint32_t len, uint64_t time)
{
	msg_stats[proc].recv.msgs++;
	msg_stats[proc].recv.bytes += len;
	msg_stats[proc].recv.sizemap[len]++;
	msg_stats[proc].recv.updateMinMax(len);

	node_idle[proc].update(time);
}

void TraceStats::addFktEnter(uint32_t proc, uint32_t func, uint64_t time)
{
	uint32_t group = getFunctionGroup(func);

	if (fkt_stats[proc][group][func].enter == std::numeric_limits<uint64_t>::max())
	{
		fkt_stats[proc][group][func].enter = time; // remember function entry
	}
	else
	{
		std::stringstream msg;
		msg <<
			"consecutive ENTER-events\n" <<
			"  p=" << proc << "\n" <<
			"  g=" << group << "\n" <<
			"  f=" << func << " (" << getFunctionName(func) << ")";
		throw std::runtime_error(msg.str());
	}
}

void TraceStats::addFktLeave(uint32_t proc, uint32_t func, uint64_t time)
{
	uint32_t group = getFunctionGroup(func);

	if (fkt_stats[proc][group][func].enter != std::numeric_limits<uint64_t>::max())
	{
		auto entry = fkt_stats[proc][group][func].enter;
		auto leave = time;

		fkt_stats[proc][group][func].time += leave-entry;
		fkt_stats[proc][group][func].calls++;
		fkt_stats[proc][group][func].enter = std::numeric_limits<uint64_t>::max(); // reset entry time for next call-event
	}
	else
	{
		std::stringstream msg;
		msg <<
			"consecutive LEAVE-events\n" <<
			"  p=" << proc << "\n" <<
			"  g=" << group << "\n" <<
			"  f=" << func << " (" << getFunctionName(func) << ")";
		throw std::runtime_error(msg.str());
	}
}

uint64_t TraceStats::getFlops(uint32_t proc)
{
	if (papi_counter[proc]["PAPI_FP_OPS"] > 0)
		return papi_counter[proc]["PAPI_FP_OPS"];

	if (papi_counter[proc]["PAPI_FP_INS"] > 0)
		return papi_counter[proc]["PAPI_FP_INS"];

	std::cout << "Warning: no PAPI_FP_OPS or PAPI_FP_INS recorded" << std::endl;
	return 0;
}

TraceStats::OTF_Trace_Param TraceStats::getOtfParam(void)
{
	return otfparam;
}

double TraceStats::getRelativeTime(uint64_t time)
{
	/* return the percentage of the timestamp in relation to the runtime of the whole application */

	auto t = static_cast<double>(time);
	auto e = static_cast<double>(otfparam.time_end);
	return t/e;
}

double TraceStats::getAbsoluteTime(uint64_t time)
{
	auto t = static_cast<double>(time);
	auto r = static_cast<double>(otfparam.time_resolution);
	return t/r;
}

uint64_t TraceStats::toNanoS(uint64_t time)
{
	auto ns = getAbsoluteTime(time)*1e9;
	return static_cast<uint64_t>(ns);
}

uint32_t TraceStats::getNumProcesses(void)
{
	return process_map.size();
}

uint32_t TraceStats::getNumFunctions(void)
{
	return function_map.size();
}

uint64_t TraceStats::getNumSentGlobal(void)
{
	uint64_t sum {0};

	for (auto x:msg_stats)
		sum += x.second.sent.msgs;

	return sum;
}

uint64_t TraceStats::getNumRecvGlobal(void)
{
	uint64_t sum {0};

	for (auto x:msg_stats)
		sum += x.second.recv.msgs;

	return sum;
}

uint64_t TraceStats::getBytesSentGlobal(void)
{
	uint64_t sum {0};

	for (auto x:msg_stats)
		sum += x.second.sent.bytes;

	return sum;
}

uint64_t TraceStats::getBytesRecvGlobal(void)
{
	uint64_t sum {0};

	for (auto x:msg_stats)
		sum += x.second.recv.bytes;

	return sum;
}

double TraceStats::getApplicationTime(void)
{
	return static_cast<double>(otfparam.time_end) / static_cast<double>(otfparam.time_resolution);
}

std::string TraceStats::getCommunicatorName(uint32_t groupid)
{
	/* in OTF, communicators are represented by process-groups */

	return process_group_map[groupid].name;
}

std::set<uint32_t> TraceStats::getCommunicatorMembers(uint32_t groupid)
{
	/* in OTF, communicators are represented by process-groups */

	return process_group_map[groupid].members;
}

std::string TraceStats::getCollectiveName(uint32_t collid)
{
	return coll_map[collid].name;
}

uint32_t TraceStats::getCollectiveType(uint32_t collid)
{
	return coll_map[collid].type;
}

uint32_t TraceStats::getCommunicatorSize(uint32_t groupid)
{
	return getCommunicatorMembers(groupid).size();
}

std::set<uint32_t> TraceStats::getCollectiveCommunicators(void)
{
	/* returns all the communicators which were used by coll. operations */

	std::set<uint32_t> s;

	for (auto c:coll_stats)
	{
		s.insert(c.first);
	}

	return s;
}

std::map<uint32_t, uint32_t> TraceStats::getCollectiveCommunicatorsMap(void)
{
	/* returns a map which translates OTF comm. IDs to VEF comm. IDs*/

	std::map<uint32_t, uint32_t> m;

	int vefID = 0;
	for (auto otfID:getCollectiveCommunicators())
	{
		m[otfID] = vefID;
		//std::cout
		//<< "otf=" << otfID << " vef=" << vefID
		//<< " (" << getCommunicatorName(otfID) << ")"
		//<< std::endl;
		vefID++;
	}

	return m;
}

uint64_t TraceStats::getNumSent(uint32_t pid)
{
	return msg_stats[pid].sent.msgs;
}

uint64_t TraceStats::getBytesSent(uint32_t pid)
{
	return msg_stats[pid].sent.bytes;
}

uint64_t TraceStats::getNumRecv(uint32_t pid)
{
	return msg_stats[pid].recv.msgs;
}

uint64_t TraceStats::getBytesRecv(uint32_t pid)
{
	return msg_stats[pid].recv.bytes;
}

std::string TraceStats::getCounterName(uint32_t cid)
{
	return counter_map[cid].name;
}

std::string TraceStats::getCounterUnit(uint32_t cid)
{
	return counter_map[cid].unit;
}

std::string TraceStats::getFunctionName(uint32_t fid)
{
	return function_map[fid].name;
}

std::string TraceStats::getGroupName(uint32_t gid)
{
	return function_group_map[gid];
}

uint32_t TraceStats::getFunctionGroup(uint32_t fid)
{
	return function_map[fid].group;
}

uint64_t TraceStats::getFunctionCalls(uint32_t pid, uint32_t fid)
{
	uint32_t gid = getFunctionGroup(fid);
	return fkt_stats[pid][gid][fid].calls;
}

double TraceStats::getFunctionTime(uint32_t pid, uint32_t fid)
{
	uint32_t gid = getFunctionGroup(fid);
	return fkt_stats[pid][gid][fid].time/1e9;
}

std::string
TraceStats::map2table(const std::string title, std::map<uint32_t, std::string>& container)
{
	std::stringstream buf;

	buf << title << ": " << container.size() << '\n';
	buf << "==================================================================" << '\n';
	buf << "ID, Name" << '\n';
	buf << "__________________________________________________________________" << '\n';
	for (auto x:container)
	{
		buf << std::setfill(' ') << std::setw(3) << x.first << " | " << x.second << '\n';
	}
	buf << std::endl;

	return buf.str();
}

std::string
TraceStats::map2table(const std::string title, std::map<uint32_t, ProcessParameters>& container)
{
	std::stringstream buf;

	buf << title << ": " << container.size() << '\n';
	buf << "==================================================================" << '\n';
	buf << "ID, Parent, Name" << '\n';
	buf << "__________________________________________________________________" << '\n';
	for (auto x:container)
	{
		if (!config->verbose)
			cutString(x.second.name, 30);

		buf << std::setfill(' ') << std::setw(3) << x.first << " | "
			<< std::setfill(' ') << std::setw(6) << x.second.parent << " | "
			<< x.second.name << '\n';
	}
	buf << std::endl;

	return buf.str();
}

std::string
TraceStats::map2table(const std::string title, std::map<uint32_t, ProcessGroupParameters>& container)
{
	std::stringstream buf;

	buf << title << ": " << container.size() << '\n';
	buf << "==================================================================" << '\n';
	buf << "ID, Name, Num. of Members, Group Members" << '\n';
	buf << "__________________________________________________________________" << '\n';
	for (auto x:container)
	{
		if (!config->verbose)
			cutString(x.second.name, 30);

		auto id         = x.first;
		auto name       = x.second.name;
		auto numMembers = x.second.numMembers;
		std::string members = "";
		for (const auto m:x.second.members)
			members += std::to_string(m) + ", ";

		buf << id << " | "
			<< std::setfill(' ') << std::setw(20) << name << " | "
			<< std::setfill(' ') << std::setw(3) << numMembers << " | "
			<< members
			<< '\n';
	}
	buf << std::endl;

	return buf.str();
}

std::string
TraceStats::map2table(const std::string title, std::map<uint32_t, FunctionParameters>& container)
{
	std::stringstream buf;

	buf << title << ": " << container.size() << '\n';
	buf << "==================================================================" << '\n';
	buf << "ID, Group, Source, Name" << '\n';
	buf << "__________________________________________________________________" << '\n';
	for (auto x:container)
	{
		if (!config->verbose)
			cutString(x.second.name, 30);

		buf << std::setfill(' ') << std::setw(3) << x.first << " | "
			<< std::setfill(' ') << std::setw(5) << x.second.group << " | "
			<< std::setfill(' ') << std::setw(6) << x.second.source << " | "
			<< x.second.name << '\n';
	}
	buf << std::endl;

	return buf.str();
}

std::string
TraceStats::map2table(const std::string title, std::map<uint32_t, CounterParameters>& container)
{
	std::stringstream buf;

	buf << title << ": " << container.size() << '\n';
	buf << "==================================================================" << '\n';
	buf << "ID, Group, Unit, Name" << '\n';
	buf << "__________________________________________________________________" << '\n';
	for (auto x:container)
	{
		if (!config->verbose)
			cutString(x.second.name, 30);

		buf << std::setfill(' ') << std::setw(3) << x.first << " | "
			<< std::setfill(' ') << std::setw(5) << x.second.group << " | "
			<< std::setfill(' ') << std::setw(4) << x.second.unit << " | "
			<< x.second.name << '\n';
	}
	buf << std::endl;

	return buf.str();
}

std::string
TraceStats::msgs2table(const std::string title, std::map<uint32_t, MessageStatistics>& container)
{
	//const???
	auto _printStats = [](std::string subtitle, MessageStatistics::DirectionStats& ds)
	{
		std::stringstream buf;

		buf << subtitle << '\n';
		if (ds.msgs > 0 && ds.bytes > 0)
		{
			buf << "Details" << '\n';
			for (const auto y:ds.sizemap)
			{
				buf << "  "
					<< std::setfill(' ') << std::setw(9) << y.first << " Bytes: "
					<< std::setfill(' ') << std::setw(5) << y.second << "x "
					<< std::setprecision(3) << "(" << (static_cast<double>(y.second) / static_cast<double>(ds.msgs)*100) << "%)" << '\n';
			}
			buf << '\n';

			buf << "Summary" << '\n';
			buf
				<< "  Total : " << ds.bytes << " Bytes, " << ds.msgs << " Messages" << '\n'
				<< "  Min   : " <<   ds.min << " Bytes" << '\n'
				<< "  Max   : " <<   ds.max << " Bytes" << '\n';
		}
		else
		{
			buf << "  "
				<< "No P2P messages recorded on this node." << '\n';
		}

		return buf.str();
	};

	std::stringstream buf;
	uint32_t proc = 0;

	buf << title << '\n';
	buf << "===================================================" << '\n';
	for (auto x:container)
	{
		buf << "---------------------------------------------------" << '\n';
		buf << "Process " << proc << ":" << '\n';
		buf << "---------------------------------------------------" << '\n';
		proc++;

		buf << _printStats("Send Statistics", x.second.sent) << '\n';
		buf << _printStats("Recv Statistics", x.second.recv) << '\n';

	}

	const auto _iec_prefix = [](uint64_t bytes)
	{
		constexpr uint64_t kilo = 1024;
		constexpr uint64_t mega = kilo*kilo;
		constexpr uint64_t giga = mega*kilo;
		constexpr uint64_t tera = giga*kilo;
		constexpr uint64_t peta = tera*kilo;
		constexpr uint64_t  exa = peta*kilo;

		double b;
		std::string unit;
		if (bytes >= exa)
		{
			b = static_cast<double>(bytes) / static_cast<double>(exa);
			unit = "EiB";
		}
		else if (bytes >= peta)
		{
			b = static_cast<double>(bytes) / static_cast<double>(peta);
			unit = "PiB";
		}
		else if (bytes >= tera)
		{
			b = static_cast<double>(bytes) / static_cast<double>(tera);
			unit = "TiB";
		}
		else if (bytes >= giga)
		{
			b = static_cast<double>(bytes) / static_cast<double>(giga);
			unit = "GiB";
		}
		else if (bytes >= mega)
		{
			b = static_cast<double>(bytes) / static_cast<double>(mega);
			unit = "MiB";
		}
		else if (bytes >= kilo)
		{
			b = static_cast<double>(bytes) / static_cast<double>(kilo);
			unit = "KiB";
		}
		else
		{
			b = static_cast<double>(bytes);
			unit = "B";
		}

		return std::to_string(b) + " " + unit;
	};

	buf << "---------------------------------------------------" << '\n';
	buf << "All Processes" << '\n';
	buf << "---------------------------------------------------" << '\n';
	buf << "  " << getNumSentGlobal() << " Messages sent" << '\n';
	buf << "  " << getNumRecvGlobal() << " Messages received" << '\n';
	buf << "  " << _iec_prefix(getBytesSentGlobal()) << " sent" << '\n';
	buf << "  " << _iec_prefix(getBytesRecvGlobal()) << " received" << '\n';

	return buf.str();
}

std::string
TraceStats::fkts2table(const std::string title, std::map<uint32_t,std::map<uint32_t,std::map<uint32_t, FunctionStatistics>>>& container)
{
	std::stringstream buf;

	buf << title << '\n';
	for (const auto m1:container)
	{
		const auto p = m1.first;
		buf << "---------------------------------------------------" << '\n';
		buf << "Process " << p << '\n';
		buf << "---------------------------------------------------" << '\n';

		for (const auto m2:m1.second)
		{
			const auto g = m2.first;
			std::string gname = getGroupName(g);
			if (gname == "") gname = "unspecified group";
			buf << "/--- " << gname << " ---\\" << '\n';

			for (const auto m3:m2.second)
			{
				const auto f = m3.first;
				std::string fname = getFunctionName(f);

				if (!config->verbose)
					cutString(fname, 25);

				const auto fpercentage = static_cast<double>(m3.second.time) / static_cast<double>(getApplicationTime()) * 100.0;

				buf << "|--> "
					<< std::setw(30) << fname << " : "
					<< std::setw(10) << m3.second.calls << " calls, "
					<< std::setw(10) << std::setprecision(3) << m3.second.time/1e9 << " seconds"
					<< " (" << std::setprecision(4) << fpercentage << "%)"
					<< '\n';
			}
			buf << '\n';
		}
		buf << '\n';
	}

	return buf.str();
}

std::string TraceStats::coll2table(const std::string title, std::map<uint32_t, std::map<uint32_t, CollectiveStatistics>>& container)
{
	std::stringstream buf;

	buf << title << '\n';
	buf << "===================================================" << '\n';
	for (const auto c:container)
	{
		buf << getCommunicatorName(c.first) << '\n';
		buf << "---------------------------------------------------" << '\n';
		for (const auto o:c.second)
		{
			buf << getCollectiveName(o.first) << " (type=" << getCollectiveType(o.first) << ")" << '\n';
			buf << "  calls: " << o.second.calls << '\n';
			buf << "  sent : " << o.second.sent << " Bytes\n";
			buf << "  recv : " << o.second.recv << " Bytes\n";
			buf << '\n';
		}
		buf << '\n';
	}
	buf << '\n';

	return buf.str();
}

void
TraceStats::print(void)
{
	std::stringstream buf;

	buf << "~~~~~~~~~~~~~~~~~~~~~~ Stats ~~~~~~~~~~~~~~~~~~~~~~" << '\n';

	buf << "OTF Stats:" << '\n';
	buf << "  Filename              : " << config->otffile << '\n';
	buf << "  Creator               : " << otfparam.creator << '\n';
	buf << "  Trace begin           : " << otfparam.time_begin << " ticks" << '\n';
	buf << "  Trace end             : " << otfparam.time_end << " ticks" << '\n';
	buf << "  Trace time resolution : " << otfparam.time_resolution << " ticks per second" << '\n';
	buf << "  Trace duration        : " << static_cast<double>(otfparam.time_end) / static_cast<double>(otfparam.time_resolution) << " seconds\n";
	buf << "\n";

	buf << map2table("Process Groups", process_group_map);
	buf << map2table("Processes", process_map);
	buf << "\n";

	buf << map2table("Counter Groups", counter_group_map);
	buf << map2table("Counters", counter_map);
	buf << "\n";

	buf << msgs2table("Message Statistics:", msg_stats);
	buf << "\n";

	buf << map2table("Function Groups", function_group_map);
	buf << map2table("Functions", function_map);
	buf << "\n";

	buf << fkts2table("Function Statistics:", fkt_stats);
	buf << "\n";

	buf << coll2table("Collective Statistics:", coll_stats);
	buf << "\n";

	buf << "PAPI/Performance Counter Stats:\n";
	buf << "===================================================" << '\n';
	std::map<std::string, uint64_t> total;
	for (auto p:papi_counter)
	{
		auto proc = p.first;
		buf << "Process " << proc << ":\n";
		buf << "---------------------------------------------------" << '\n';

		for (auto c:p.second)
		{
			buf.precision(3);
			buf << std::scientific << std::setw(16) << c.first << "   " << static_cast<double>(c.second) << std::endl;

			total[c.first] += c.second;

			metrics.papi.push_back(p.second);
		}
		buf << '\n';
	}

	buf << "All Processes:" << "\n";
	buf << "---------------------------------------------------" << '\n';
	for (auto c:total)
	{
		buf.precision(3);
		buf << std::scientific << std::setw(16) << c.first << "   " << static_cast<double>(c.second) << std::endl;
	}
	buf << '\n';

	buf << "Additional Metrics:" << "\n";
	buf << "===================================================" << '\n';

	buf << "Verbosity:" << '\n';
	buf << "---------------------------------------------------" << '\n';
	std::vector<double> verbosity_all;
	for (auto p:process_map)
	{
		auto proc = p.first;
		auto bytes = getBytesSent(proc) + getBytesRecv(proc);
		auto flops = getFlops(proc);

		buf << "P" << proc << ": ";
		if (flops == 0 || bytes == 0)
		{
			char b[4096];
			sprintf(b, "No flops or bytes recorded (f=%lu, b=%lu)", flops, bytes);
			buf << b << '\n';
			verbosity_all.push_back(0.0);
		}
		else
		{
			auto verbosity_node = (double)bytes / (double)flops;
			buf << verbosity_node << " Bytes/Flop" << '\n';
			verbosity_all.push_back(verbosity_node);

			metrics.verbosity.push_back(verbosity_node);
		}
	}
	buf << "--------------------------" << '\n';
	buf << "Global Average: " << average(verbosity_all) << " Bytes/Flop" << '\n';
	buf << '\n';


	buf << "Messages Rate:" << '\n';
	buf << "---------------------------------------------------" << '\n';
	std::vector<double> messagerate_all;
	for (auto p:process_map)
	{
		auto proc = p.first;
		double n = getNumSent(proc);
		double t = getApplicationTime();

		buf << "P" << proc << ": ";
		if (n > 0)
		{
			auto mrate = n/t;
			buf << mrate << " Msgs/s" << '\n';
			messagerate_all.push_back(mrate);

			metrics.msgrate.push_back(mrate);
			metrics.msg_tx.push_back(getNumSent(proc));
			metrics.msg_rx.push_back(getNumRecv(proc));
			metrics.bytes_tx.push_back(getBytesSent(proc));
			metrics.bytes_rx.push_back(getBytesRecv(proc));
		}
		else
		{
			buf << "No messages recorded." << '\n';
			messagerate_all.push_back(0.0);
		}
	}
	buf << "--------------------------" << '\n';
	buf << "Global Average: " << average(messagerate_all) << " Messages/s" << '\n';
	buf << '\n';


	buf << "MPI Idle Time" << '\n';
	buf << "---------------------------------------------------" << '\n';
	std::map<std::string, std::vector<double>> idle_all;
	buf << "# process min max avg tot percent" << '\n';
	for (auto p:node_idle)
	{
		auto proc = p.first;
		//auto min = static_cast<double>(p.second.getMin()/1e9);
		//auto max = static_cast<double>(p.second.getMax()/1e9);
		//auto avg = static_cast<double>(p.second.getAvg()/1e9);
		//auto tot = static_cast<double>(p.second.getTot()/1e9);
		auto min = toNanoS(p.second.getMin());
		auto max = toNanoS(p.second.getMax());
		auto avg = toNanoS(p.second.getAvg());
		auto tot = toNanoS(p.second.getTot());
		auto percent = p.second.getTot() / getApplicationTime();

		buf << "P" << proc << ": " << min << " " << max << " " << avg << " " << tot << " " << percent << '\n';

		idle_all["min"].push_back(min);
		idle_all["max"].push_back(max);
		idle_all["avg"].push_back(avg);
		idle_all["tot"].push_back(tot);
		idle_all["percent"].push_back(percent);

		metrics.mpi_idle_min.push_back(min);
		metrics.mpi_idle_max.push_back(max);
		metrics.mpi_idle_avg.push_back(avg);
		metrics.mpi_idle_tot.push_back(tot);
		metrics.mpi_idle_tot.push_back(percent);
	}
	buf << "--------------------------" << '\n';
	buf << "Global Average Min     : " << average(idle_all["min"]) << " s idle" << '\n';
	buf << "Global Average Max     : " << average(idle_all["max"]) << " s idle" << '\n';
	buf << "Global Average Average : " << average(idle_all["avg"]) << " s idle" << '\n';
	buf << "Global Average Total   : " << average(idle_all["tot"]) << " s idle" << '\n';
	buf << "Global Average Percent : " << average(idle_all["percent"]) << " % idle" << '\n';
	buf << '\n';


	buf << "Performance:" << '\n';
	buf << "---------------------------------------------------" << '\n';
	std::vector<double> perf_all;
	for (auto p:process_map)
	{
		auto proc = p.first;
		double flops = getFlops(proc);
		double time = getApplicationTime();

		buf << "P" << proc << ": ";
		if (flops > 0)
		{
			auto fps = flops/time;
			buf << fps  << " Flops/s" << '\n';
			perf_all.push_back(fps);
		}
		else
		{
			buf << "No flops recorded." << '\n';
			perf_all.push_back(0.0);
		}
	}
	buf << "--------------------------" << '\n';
	buf << "Global Average: " << average(perf_all) << " Flops/s" << '\n';
	buf << "Global Total  : " << std::accumulate(perf_all.begin(), perf_all.end(), 0.0) << " Flops/s" << '\n';
	buf << '\n';

	buf << "Note: Metrics with respect to time may be inaccurate due to the tracing overhead!" << '\n';
	buf << '\n';

	buf << "~~~~~~~~~~~~~~~~~~~~~~ /Stats ~~~~~~~~~~~~~~~~~~~~~~" << '\n';

	const std::string sep = ",";
	const std::string header =
		"Process" + sep +
		"Verbosity" + sep +
		"MessageRate" + sep +
		"MPI_idle_min" + sep +
		"MPI_idle_max" + sep +
		"MPI_idle_avg" + sep +
		"TX_Messages" + sep +
		"RX_Messages" + sep +
		"TX_Bytes" + sep +
		"RX_Bytes" +
		//"TOT_CYC" + sep +
		//"TOT_INS" + sep +
		//"FP_INS" + sep +
		//"FP_OPS" +
		'\n';

	std::ofstream aggr_nodes(config->resdir + "/" + "aggr_nodes.csv", std::ofstream::out);
	aggr_nodes << header;
	for (uint32_t i=0; i<getNumProcesses(); i++)
	{
		aggr_nodes << i << sep;
		aggr_nodes << metrics.verbosity.at(i) << sep;
		aggr_nodes << metrics.msgrate.at(i) << sep;
		aggr_nodes << metrics.mpi_idle_min.at(i) << sep;
		aggr_nodes << metrics.mpi_idle_max.at(i) << sep;
		aggr_nodes << metrics.mpi_idle_avg.at(i) << sep;
		aggr_nodes << metrics.msg_tx.at(i) << sep;
		aggr_nodes << metrics.msg_rx.at(i) << sep;
		aggr_nodes << metrics.bytes_tx.at(i) << sep;
		aggr_nodes << metrics.bytes_rx.at(i);
		//aggr_nodes << metrics.papi.at(i)["PAPI_TOT_CYC"] << sep;
		//aggr_nodes << metrics.papi.at(i)["PAPI_TOT_INS"] << sep;
		//aggr_nodes << metrics.papi.at(i)["PAPI_FP_INS"] << sep;
		//aggr_nodes << metrics.papi.at(i)["PAPI_FP_OPS"] << sep;
		aggr_nodes << '\n';
	}
	aggr_nodes << std::endl;
	aggr_nodes.close();

	std::ofstream aggr_avg(config->resdir + "/" + "aggr_avg.csv", std::ofstream::out);
	aggr_avg << header;
	{
		aggr_avg << -1 << sep;
		aggr_avg << average(metrics.verbosity) << sep;
		aggr_avg << average(metrics.msgrate) << sep;
		aggr_avg << average(metrics.mpi_idle_min) << sep;
		aggr_avg << average(metrics.mpi_idle_max) << sep;
		aggr_avg << average(metrics.mpi_idle_avg) << sep;
		aggr_avg << average(metrics.msg_tx) << sep;
		aggr_avg << average(metrics.msg_rx) << sep;
		aggr_avg << average(metrics.bytes_tx) << sep;
		aggr_avg << average(metrics.bytes_rx);
		// TODO: PAPI avg
	}
	aggr_avg << std::endl;
	aggr_avg.close();

	// write stats to screen
	if (config->stats_toscreen)
		std::cout << buf.str() << std::endl;

	// write stats to file
	std::string fname = config->resdir + "/tracestats_" + config->tracename + ".txt";
	if (config->stats_toscreen)
		std::cout << "### trace stats file: " << fname << std::endl;
	std::ofstream statsfile;
	statsfile.open (fname, std::ofstream::out);
	statsfile << buf.str() << std::endl;
	statsfile.close();
}

bool
TraceStats::validate(const bool printErrors)
{
	bool error = false;

	if (getNumProcesses() == 0)
	{
		if (printErrors)
			std::cout << "Warning: no processes found." << std::endl;
		error = true;
	}

	if (getNumFunctions() == 0)
	{
		if (printErrors)
			std::cout << "Warning: no functions found." << std::endl;
		error = true;
	}

	// check for equal number of send/receive events globally
	if (getNumSentGlobal() != getNumRecvGlobal())
	{
		if (printErrors)
			std::cout	<< "Warning: sent/recv messages are unequal.\n"
						<< getNumSentGlobal() << " vs " << getNumRecvGlobal()
						<< std::endl;
		error = true;
	}

	// check for equal number of entrys/exits for each function
	for (const auto m1:fkt_stats)
		for (const auto m2:m1.second)
			for (const auto m3:m2.second)
			{
				if (m3.second.enter != std::numeric_limits<uint64_t>::max())
					if (printErrors)
						std::cout	<< "Warning: unbalanced function entry/exit ratio.\n"
									<< "  f_id=" << m2.first << " (" << getFunctionName(m2.first) << ")"
									<< std::endl;
			}

	return error;
}
