#ifndef _VEF_OTFSOURCE_H_
#define _VEF_OTFSOURCE_H_

#include <globals.h>
#include <config.h>
#include <otf_handler.h>
#include <trace_stats.h>
#include <trace_visualizer.h>

template <typename T>
class Sonar : public OTF_Handler {
public:
	Sonar()
	{
		ctor_msg(__PRETTY_FUNCTION__);
	}
	
	~Sonar()
	{
		dtor_msg(__PRETTY_FUNCTION__);
	}

	//
	// specific handler implementations
	//

	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Wunused-variable"
	#pragma GCC diagnostic ignored "-Wunused-parameter"

	static int handleDefCreator(void* userData, uint32_t stream, const char* creator, OTF_KeyValueList *list)
	{
		((T*)userData)->ts->addOtfCreator(creator);

		if (((T*)userData)->cfg->verbose)
		{
			std::cout << "OTF creator            : " << creator << std::endl;
		}

		return OTF_RETURN_OK;
	}

	static int handleDefTimeRange(void* userData, uint32_t stream, uint64_t minTime, uint64_t maxTime, OTF_KeyValueList* list)
	{
		((T*)userData)->ts->addOtfTimeRange(minTime, maxTime);

		if (((T*)userData)->cfg->verbose)
		{
			auto tps = ((T*)userData)->ts->getOtfParam().time_resolution;
			auto tmin = static_cast<double>(minTime)/static_cast<double>(tps);
			auto tmax = static_cast<double>(maxTime)/static_cast<double>(tps);
			std::cout << "OTF ticks per second   : " << tps << std::endl;
			std::cout << "OTF Time Range (time)  : [" << tmin << " ... " << tmax << "]" << std::endl;
			std::cout << "OTF Time Range (ticks) : [" << minTime << " ... " << maxTime << "]" << std::endl;
		}

		return OTF_RETURN_OK;
	}

	static int handleDefTimerResolution(void* userData, uint32_t stream, uint64_t ticksPerSecond, OTF_KeyValueList *list)
	{
		((T*)userData)->ts->addOtfResolution(ticksPerSecond);
		return OTF_RETURN_OK;
	}

	static int handleDefProcess(void* userData, uint32_t stream, uint32_t process, const char* name, uint32_t parent, OTF_KeyValueList* list)
	{
		((T*)userData)->ts->addProcess(process, name, parent);
		return OTF_RETURN_OK;
	}

	static int handleDefProcessGroup(void* userData, uint32_t stream, uint32_t procGroup, const char* name, uint32_t numberOfProcs, const uint32_t* procs, OTF_KeyValueList *list)
	{
		std::set<uint32_t> group_members;
		for (uint32_t i=0; i<numberOfProcs; i++)
			group_members.insert(procs[i]);

		((T*)userData)->ts->addProcessGroup(procGroup, name, numberOfProcs, group_members);
		return OTF_RETURN_OK;
	}

	static int handleDefFunction(void* userData, uint32_t stream, uint32_t func, const char* name, uint32_t funcGroup, uint32_t source, OTF_KeyValueList *list)
	{
		((T*)userData)->ts->addFunction(func, name, funcGroup, source);
		return OTF_RETURN_OK;
	}

	static int handleDefFunctionGroup(void* userData, uint32_t stream, uint32_t funcGroup, const char* name, OTF_KeyValueList *list)
	{
		((T*)userData)->ts->addFunctionGroup(funcGroup, name);
		return OTF_RETURN_OK;
	}

	static int handleDefCounter(void* userData, uint32_t stream, uint32_t counter, const char* name, uint32_t properties, uint32_t counterGroup, const char* unit, OTF_KeyValueList *list)
	{
		((T*)userData)->ts->addCounter(counter, name, unit, counterGroup);

		return OTF_RETURN_OK;
	}

	static int handleDefCounterGroup(void* userData, uint32_t stream, uint32_t counterGroup, const char* name, OTF_KeyValueList *list)
	{
		((T*)userData)->ts->addCounterGroup(counterGroup, name);

		return OTF_RETURN_OK;
	}

	static int handleDefCollectiveOperation(void* userData, uint32_t stream, uint32_t collOp, const char* name, uint32_t type, OTF_KeyValueList *list)
	{
		((T*)userData)->ts->addCollective(collOp, type, name);

		return OTF_RETURN_OK;
	}

	static int handleDefAuxSamplePoint(void* userData, uint32_t stream, uint64_t time, OTF_AuxSamplePointType type, OTF_KeyValueList* list)
	{
		// re-implementation of this function to suppress the messages from otf_handler.h
		// not needed for functionality
		return OTF_RETURN_OK;
	}

	static int handleBeginProcess(void* userData, uint64_t time, uint32_t process, OTF_KeyValueList *list)
	{
		if (((T*)userData)->cfg->rawotf)
		{
			std::cout
				<< "pb "
				<< "p=" << process << " "
				<< "t=" << ((T*)userData)->ts->getAbsoluteTime(time)*1000 << " ms"
				<< std::endl;
		}

		return OTF_RETURN_OK;
	}

	static int handleEndProcess(void* userData, uint64_t time, uint32_t process, OTF_KeyValueList *list)
	{
		if (((T*)userData)->cfg->rawotf)
		{
			std::cout
				<< "pe "
				<< "p=" << process << " "
				<< "t=" << ((T*)userData)->ts->getAbsoluteTime(time)*1000 << " ms"
				<< std::endl;
		}

		return OTF_RETURN_OK;
	}

	static int handleSendMsg(void* userData, uint64_t time, uint32_t sender, uint32_t receiver, uint32_t group, uint32_t type, uint32_t length, uint32_t source, OTF_KeyValueList *list)
	{
		if (((T*)userData)->cfg->rawotf)
		{
			std::cout
				<< "sm"
				<< " p" << sender << " --> " << length << " B --> p" << receiver << " @ " << ((T*)userData)->ts->getAbsoluteTime(time)
			<< std::endl;
		}

		((T*)userData)->tviz->addSendP2P(sender, time, length);
		((T*)userData)->tviz->addMessageCDF_P2P(sender, length);

		((T*)userData)->ts->addSendMsg(sender, length, time);

		return OTF_RETURN_OK;
	}

	static int handleRecvMsg(void* userData, uint64_t time, uint32_t recvProc, uint32_t sendProc, uint32_t group, uint32_t type, uint32_t length, uint32_t source, OTF_KeyValueList *list)
	{
		if (((T*)userData)->cfg->rawotf)
		{
			std::cout
				<< "rm"
				<< " p" << recvProc << " <-- " << length << " B <-- p" << sendProc << " @ " << ((T*)userData)->ts->getAbsoluteTime(time)
			<< std::endl;
		}

		((T*)userData)->tviz->addRecvP2P(recvProc, time, length);

		((T*)userData)->ts->addRecvMsg(recvProc, length, time);

		return OTF_RETURN_OK;
	}

	static int handleEnter(void* userData, uint64_t time, uint32_t function, uint32_t process, uint32_t source, OTF_KeyValueList *list)
	{
		if (((T*)userData)->cfg->rawotf)
		{
			std::cout
				<< "ef "
				<< "p" << process << " f" << function << " "
				<< "--> " << ((T*)userData)->ts->getFunctionName(function) << " @ " << ((T*)userData)->ts->getAbsoluteTime(time)
			<< std::endl;
		}

		// FIXME: broken for VampirTrace traces
		//if (((T*)userData)->cfg->stats_fkt)
		//	((T*)userData)->ts->addFktEnter(process, function, ((T*)userData)->ts->toNanoS(time));

		return OTF_RETURN_OK;
	}

	static int handleLeave(void* userData, uint64_t time, uint32_t function, uint32_t process, uint32_t source, OTF_KeyValueList *list)
	{
		if (((T*)userData)->cfg->rawotf)
		{
			std::cout
				<< "lf "
				<< "p" << process << " f" << function << " "
				<< "<-- " << ((T*)userData)->ts->getFunctionName(function) << " @ " << ((T*)userData)->ts->getAbsoluteTime(time)
			<< std::endl;
		}

		// FIXME: broken for VampirTrace traces
		//if (((T*)userData)->cfg->stats_fkt)
		//	((T*)userData)->ts->addFktLeave(process, function, ((T*)userData)->ts->toNanoS(time));

		static long x;
		if (((T*)userData)->cfg->progress)
		{
			if (++x >= 500000)
			{
				std::cout
					<< "Progress: "
					<< std::fixed << std::setprecision(2) << ((T*)userData)->ts->getRelativeTime(time) * 100
					<< " %" << '\r';
				std::cout.flush();
				x = 0;
			}
		}

		return OTF_RETURN_OK;
	}

	static int handleCounter(void* userData, uint64_t time, uint32_t process, uint32_t counter, uint64_t value, OTF_KeyValueList *list)
	{
		if (((T*)userData)->cfg->rawotf)
		{
			std::cout
				<< "ct p" << process << " c" << counter << " "
				<< "\"" << ((T*)userData)->ts->getCounterName(counter) << "\" "
				<< "v=" << value << " " << ((T*)userData)->ts->getCounterUnit(counter) << " "
				<< "@ " << ((T*)userData)->ts->getAbsoluteTime(time)
			<< std::endl;
		}

		// PAPI
		((T*)userData)->ts->addPapiCounter(process, counter, value);

		return OTF_RETURN_OK;
	}

	static int handleBeginCollectiveOperation(void* userData, uint64_t time, uint32_t process, uint32_t collOp, uint64_t matchingId, uint32_t procGroup, uint32_t rootProc, uint64_t sent, uint64_t received, uint32_t scltoken, OTF_KeyValueList *list)
	{
		if (((T*)userData)->cfg->rawotf)
		{
			std::cout
				<< "cb "
				<< "m" << matchingId << " "
				<< "p" << process << " "
				<< "root=" << rootProc << " "
				<< "op=" << collOp << " "
				<< "(" << ((T*)userData)->ts->getCollectiveName(collOp) << ") "
				<< "c=" << procGroup << " "
				<< "(" << ((T*)userData)->ts->getCommunicatorName(procGroup) << ") "
				<< "s=" << sent << " "
				<< "r=" << received << " "
				<< "sclt=" << scltoken << " "
				<< "@ " << ((T*)userData)->ts->getAbsoluteTime(time)
			<< std::endl;
		}

		((T*)userData)->ts->addCollectiveEvent(process, procGroup, collOp, sent, received, time);

		((T*)userData)->tviz->addColl(process, time, sent, received);
		((T*)userData)->tviz->addMessageCDF_COLL(process, sent);

		((T*)userData)->ts->addSendMsg(process, sent, time);
		((T*)userData)->ts->addRecvMsg(process, received, time);

		return OTF_RETURN_OK;
	}

	static int handleEndCollectiveOperation(void* userData, uint64_t time, uint32_t process, uint64_t matchingId, OTF_KeyValueList *list)
	{
		if (((T*)userData)->cfg->rawotf)
		{
			std::cout
				<< "ce "
				<< "m" << matchingId << " "
				<< "p" << process << " "
				<< "@ " << ((T*)userData)->ts->getAbsoluteTime(time)
			<< std::endl;
		}

		return OTF_RETURN_OK;
	}

	static int handleRMAPut(void* userData, uint64_t time, uint32_t process, uint32_t origin, uint32_t target, uint32_t communicator, uint32_t tag, uint64_t bytes, uint32_t source, OTF_KeyValueList *list)
	{
		if (((T*)userData)->cfg->rawotf)
		{
			std::cout
				<< "rma put "
				<< "p" << process << " "
				<< "o=" << origin << " "
				<< "t=" << target << " "
				<< "b=" << bytes << " "
				<< "@ " << ((T*)userData)->ts->getAbsoluteTime(time)
			<< std::endl;
		}

		return OTF_RETURN_OK;
	}

	static int handleRMAGet(void* userData, uint64_t time, uint32_t process, uint32_t origin, uint32_t target, uint32_t communicator, uint32_t tag, uint64_t bytes, uint32_t source, OTF_KeyValueList *list)
	{
		if (((T*)userData)->cfg->rawotf)
		{
			std::cout
				<< "rma get "
				<< "p" << process << " "
				<< "o=" << origin << " "
				<< "t=" << target << " "
				<< "b=" << bytes << " "
				<< "@ " << ((T*)userData)->ts->getAbsoluteTime(time)
			<< std::endl;
		}

		return OTF_RETURN_OK;
	}

	static int handleRMAEnd(void* userData, uint64_t time, uint32_t process, uint32_t remote, uint32_t communicator, uint32_t tag, uint32_t source, OTF_KeyValueList *list)
	{
		if (((T*)userData)->cfg->rawotf)
		{
			std::cout
				<< "rma end "
				<< "p" << process << " "
				<< "r=" << remote << " "
				<< "@ " << ((T*)userData)->ts->getAbsoluteTime(time)
			<< std::endl;
		}

		return OTF_RETURN_OK;
	}

	#pragma GCC diagnostic pop
};

#endif
