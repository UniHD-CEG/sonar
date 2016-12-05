#ifndef OTF_HANDLER_H_
#define OTF_HANDLER_H_

#include <globals.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

class OTF_Handler {
public:
	OTF_Handler()
	{
		ctor_msg(__PRETTY_FUNCTION__);
	}
	virtual ~OTF_Handler()
	{
		dtor_msg(__PRETTY_FUNCTION__);
	}

	//
	// Functions to print the handler arguments
	//

	template<typename T, typename... Tail>
	static void printHandleArgs(T head, Tail... tail)
	{
		printHandleArgs_next(head);
		printHandleArgs(tail...);
	}

	// Argument 0..N-1
	template<typename T>
	static void printHandleArgs_next(T x)
	{
		std::cerr << x << ", ";
	}

	// Argument N
	static void printHandleArgs()
	{
		std::cerr << std::endl;
	}

	//
	// Definition Handlers
	//

	static int handleDefinitionComment(void* userData, uint32_t stream, const char* comment, OTF_KeyValueList *list)
	{
		#ifdef DEBUG
		printHandleArgs(__FUNCTION__, userData, stream, comment, list->key_count);
		#endif

		return OTF_RETURN_OK;
	}

	static int handleDefTimerResolution(void* userData, uint32_t stream, uint64_t ticksPerSecond, OTF_KeyValueList *list)
	{
		#ifdef DEBUG
		printHandleArgs(__FUNCTION__, userData, stream, ticksPerSecond, list->key_count);
		#endif

		return OTF_RETURN_OK;
	}

	static int handleDefProcess(void* userData, uint32_t stream, uint32_t process, const char* name, uint32_t parent, OTF_KeyValueList* list)
	{
		#ifdef DEBUG
		printHandleArgs(__FUNCTION__, userData, stream, process, name, parent, list->key_count);
		#endif

		return OTF_RETURN_OK;
	}

	static int handleDefProcessGroup(void* userData, uint32_t stream, uint32_t procGroup, const char* name, uint32_t numberOfProcs, const uint32_t* procs, OTF_KeyValueList *list)
	{
		#ifdef DEBUG
		printHandleArgs(__FUNCTION__, userData, stream, procGroup, name, numberOfProcs, procs, list->key_count);
		#endif

		return OTF_RETURN_OK;
	}

	static int handleDefAttributeList(void* userData, uint32_t stream, uint32_t attr_token, uint32_t num, OTF_ATTR_TYPE* array, OTF_KeyValueList *list)
	{
		#ifdef DEBUG
		printHandleArgs(__FUNCTION__, userData, stream, attr_token, num, array, list->key_count);
		#endif

		return OTF_RETURN_OK;
	}

	static int handleDefProcessOrGroupAttributes(void* userData, uint32_t stream, uint32_t proc_token, uint32_t attr_token, OTF_KeyValueList *list)
	{
		#ifdef DEBUG
		printHandleArgs(__FUNCTION__, userData, stream, proc_token, attr_token, list->key_count);
		#endif

		return OTF_RETURN_OK;
	}

	static int handleDefFunction(void* userData, uint32_t stream, uint32_t func, const char* name, uint32_t funcGroup, uint32_t source, OTF_KeyValueList *list)
	{
		#ifdef DEBUG
		printHandleArgs(__FUNCTION__, userData, stream, func, name, funcGroup, source, list->key_count);
		#endif

		return OTF_RETURN_OK;
	}

	static int handleDefFunctionGroup(void* userData, uint32_t stream, uint32_t funcGroup, const char* name, OTF_KeyValueList *list)
	{
		#ifdef DEBUG
		printHandleArgs(__FUNCTION__, userData, stream, funcGroup, name, list->key_count);
		#endif

		return OTF_RETURN_OK;
	}

	static int handleDefCollectiveOperation(void* userData, uint32_t stream, uint32_t collOp, const char* name, uint32_t type, OTF_KeyValueList *list)
	{
		#ifdef DEBUG
		printHandleArgs(__FUNCTION__, userData, stream, collOp, name, type, list->key_count);
		#endif

		return OTF_RETURN_OK;
	}

	static int handleDefCounter(void* userData, uint32_t stream, uint32_t counter, const char* name, uint32_t properties, uint32_t counterGroup, const char* unit, OTF_KeyValueList *list)
	{
		#ifdef DEBUG
		printHandleArgs(__FUNCTION__, userData, stream, counter, name, properties, counterGroup, unit, list->key_count);
		#endif

		return OTF_RETURN_OK;
	}

	static int handleDefCounterGroup(void* userData, uint32_t stream, uint32_t counterGroup, const char* name, OTF_KeyValueList *list)
	{
		#ifdef DEBUG
		printHandleArgs(__FUNCTION__, userData, stream, counterGroup, name, list->key_count);
		#endif

		return OTF_RETURN_OK;
	}

	static int handleDefScl(void* userData, uint32_t stream, uint32_t source, uint32_t sourceFile, uint32_t line, OTF_KeyValueList *list)
	{
		#ifdef DEBUG
		printHandleArgs(__FUNCTION__, userData, stream, source, sourceFile, line, list->key_count);
		#endif

		return OTF_RETURN_OK;
	}

	static int handleDefSclFile(void* userData, uint32_t stream, uint32_t sourceFile, const char* name, OTF_KeyValueList *list)
	{
		#ifdef DEBUG
		printHandleArgs(__FUNCTION__, userData, stream, sourceFile, name, list->key_count);
		#endif

		return OTF_RETURN_OK;
	}

	static int handleDefCreator(void* userData, uint32_t stream, const char* creator, OTF_KeyValueList *list)
	{
		#ifdef DEBUG
		printHandleArgs(__FUNCTION__, userData, stream, creator, list->key_count);
		#endif

		return OTF_RETURN_OK;
	}

	static int handleDefUniqueId(void* userData, uint32_t stream, uint64_t uid, OTF_KeyValueList *list)
	{
		#ifdef DEBUG
		printHandleArgs(__FUNCTION__, userData, stream, uid, list->key_count);
		#endif

		return OTF_RETURN_OK;
	}

	static int handleDefVersion(void* userData, uint32_t stream, uint8_t major, uint8_t minor, uint8_t sub, const char* string, OTF_KeyValueList *list)
	{
		#ifdef DEBUG
		printHandleArgs(__FUNCTION__, userData, stream, (uint32_t)major, (uint32_t)minor, (uint32_t)sub, string, list->key_count);
		#endif

		return OTF_RETURN_OK;
	}

	static int handleDefFile( void* userData, uint32_t stream, uint32_t token, const char *name, uint32_t group, OTF_KeyValueList *list )
	{
		#ifdef DEBUG
		printHandleArgs(__FUNCTION__, userData, stream, token, name, group, list->key_count);
		#endif

		return OTF_RETURN_OK;
	}

	static int handleDefFileGroup(void* userData, uint32_t stream, uint32_t token, const char *name, OTF_KeyValueList *list)
	{
		#ifdef DEBUG
		printHandleArgs(__FUNCTION__, userData, stream, token, name, list->key_count);
		#endif

		return OTF_RETURN_OK;
	}

	static int handleDefKeyValue(void* userData, uint32_t stream, uint32_t key, OTF_Type type, const char *name, const char *description, OTF_KeyValueList *list)
	{
		#ifdef DEBUG
		printHandleArgs(__FUNCTION__, userData, stream, key, type, name, description, list->key_count);
		#endif

		return OTF_RETURN_OK;
	}

	static int handleDefTimeRange(void* userData, uint32_t stream, uint64_t minTime, uint64_t maxTime, OTF_KeyValueList* list)
	{
		#ifdef DEBUG
		printHandleArgs(__FUNCTION__, userData, stream, minTime, maxTime, list->key_count);
		#endif

		return OTF_RETURN_OK;
	}

	static int handleDefCounterAssignments(void* userData, uint32_t stream, uint32_t counter, uint32_t number_of_members, const uint32_t* procs_or_groups, OTF_KeyValueList* list)
	{
		#ifdef DEBUG
		printHandleArgs(__FUNCTION__, userData, stream, counter, number_of_members, procs_or_groups, list->key_count);
		#endif

		return OTF_RETURN_OK;
	}

	static int handleDefProcessSubstitutes(void* userData, uint32_t stream, uint32_t representative, uint32_t numberOfProcs, const uint32_t* procs, OTF_KeyValueList* list)
	{
		#ifdef DEBUG
		printHandleArgs(__FUNCTION__, userData, stream, representative, numberOfProcs, procs, list->key_count);
		#endif

		return OTF_RETURN_OK;
	}

	static int handleDefAuxSamplePoint(void* userData, uint32_t stream, uint64_t time, OTF_AuxSamplePointType type, OTF_KeyValueList* list)
	{
		#ifdef DEBUG
		printHandleArgs(__FUNCTION__, userData, stream, time, type, list->key_count);
		#endif

		return OTF_RETURN_OK;
	}

	//
	// Event Handlers
	//

	static int handleNoOp(void* userData, uint64_t time, uint32_t process, OTF_KeyValueList *list)
	{
		#ifdef DEBUG
		printHandleArgs(__FUNCTION__, userData, time, process, list->key_count);
		#endif

		return OTF_RETURN_OK;
	}

	static int handleEnter(void* userData, uint64_t time, uint32_t function, uint32_t process, uint32_t source, OTF_KeyValueList *list)
	{
		#ifdef DEBUG
		printHandleArgs(__FUNCTION__, userData, time, function, process, source, list->key_count);
		#endif

		return OTF_RETURN_OK;
	}

	static int handleLeave(void* userData, uint64_t time, uint32_t function, uint32_t process, uint32_t source, OTF_KeyValueList *list)
	{
		#ifdef DEBUG
		printHandleArgs(__FUNCTION__, userData, time, function, process, source, list->key_count);
		#endif

		return OTF_RETURN_OK;
	}

	static int handleSendMsg(void* userData, uint64_t time, uint32_t sender, uint32_t receiver, uint32_t group, uint32_t type, uint32_t length, uint32_t source, OTF_KeyValueList *list)
	{
		#ifdef DEBUG
		printHandleArgs(__FUNCTION__, userData, time, sender, receiver, group, type, length, source, list->key_count);
		#endif

		return OTF_RETURN_OK;
	}

	static int handleRecvMsg(void* userData, uint64_t time, uint32_t recvProc, uint32_t sendProc, uint32_t group, uint32_t type, uint32_t length, uint32_t source, OTF_KeyValueList *list)
	{
		#ifdef DEBUG
		printHandleArgs(__FUNCTION__, userData, time, recvProc, sendProc, group, type, length, source, list->key_count);
		#endif

		return OTF_RETURN_OK;
	}

	static int handleCounter(void* userData, uint64_t time, uint32_t process, uint32_t counter, uint64_t value, OTF_KeyValueList *list)
	{
		#ifdef DEBUG
		printHandleArgs(__FUNCTION__, userData, time, process, counter, value, list->key_count);
		#endif

		return OTF_RETURN_OK;
	}

	static int handleCollectiveOperation(void* userData, uint64_t time, uint32_t process, uint32_t collective, uint32_t procGroup, uint32_t rootProc, uint32_t sent, uint32_t received, uint64_t duration, uint32_t source, OTF_KeyValueList *list)
	{
		#ifdef DEBUG
		printHandleArgs(__FUNCTION__, userData, time, process, collective, procGroup, rootProc, sent, received, duration, source, list->key_count);
		#endif

		return OTF_RETURN_OK;
	}

	static int handleBeginCollectiveOperation(void* userData, uint64_t time, uint32_t process, uint32_t collOp, uint64_t matchingId, uint32_t procGroup, uint32_t rootProc, uint64_t sent, uint64_t received, uint32_t scltoken, OTF_KeyValueList *list)
	{
		#ifdef DEBUG
		printHandleArgs(__FUNCTION__, userData, time, process, collOp, matchingId, procGroup, rootProc, sent, received, scltoken, list->key_count);
		#endif

		return OTF_RETURN_OK;
	}

	static int handleEndCollectiveOperation(void* userData, uint64_t time, uint32_t process, uint64_t matchingId, OTF_KeyValueList *list)
	{
		#ifdef DEBUG
		printHandleArgs(__FUNCTION__, userData, time, process, matchingId, list->key_count);
		#endif

		return OTF_RETURN_OK;
	}

	static int handleEventComment(void* userData, uint64_t time, uint32_t process, const char* comment, OTF_KeyValueList *list)
	{
		#ifdef DEBUG
		printHandleArgs(__FUNCTION__, userData, time, process, comment, list->key_count);
		#endif

		return OTF_RETURN_OK;
	}

	static int handleBeginProcess(void* userData, uint64_t time, uint32_t process, OTF_KeyValueList *list)
	{
		#ifdef DEBUG
		printHandleArgs(__FUNCTION__, userData, time, process, list->key_count);
		#endif

		return OTF_RETURN_OK;
	}

	static int handleEndProcess(void* userData, uint64_t time, uint32_t process, OTF_KeyValueList *list)
	{
		#ifdef DEBUG
		printHandleArgs(__FUNCTION__, userData, time, process, list->key_count);
		#endif

		return OTF_RETURN_OK;
	}

	static int handleFileOperation(void* userData, uint64_t time, uint32_t fileid, uint32_t process, uint64_t handleid, uint32_t operation, uint64_t bytes, uint64_t duration, uint32_t source, OTF_KeyValueList *list)
	{
		#ifdef DEBUG
		printHandleArgs(__FUNCTION__, userData, time, fileid, process, handleid, operation, bytes, duration, source, list->key_count);
		#endif

		return OTF_RETURN_OK;
	}

	static int handleBeginFileOperation(void* userData, uint64_t time, uint32_t process, uint64_t matchingId, uint32_t scltoken, OTF_KeyValueList *list)
	{
		#ifdef DEBUG
		printHandleArgs(__FUNCTION__, userData, time, process, matchingId, scltoken, list->key_count);
		#endif

		return OTF_RETURN_OK;
	}

	static int handleEndFileOperation(void* userData, uint64_t time, uint32_t process, uint32_t fileid, uint64_t matchingId, uint64_t handleId, uint32_t operation, uint64_t bytes, uint32_t scltoken, OTF_KeyValueList *list)
	{
		#ifdef DEBUG
		printHandleArgs(__FUNCTION__, userData, time, process, fileid, matchingId, handleId, operation, bytes, scltoken, list->key_count);
		#endif

		return OTF_RETURN_OK;
	}

	static int handleRMAPut(void* userData, uint64_t time, uint32_t process, uint32_t origin, uint32_t target, uint32_t communicator, uint32_t tag, uint64_t bytes, uint32_t source, OTF_KeyValueList *list)
	{
		#ifdef DEBUG
		printHandleArgs(__FUNCTION__, userData, time, process, origin, target, communicator, tag, bytes, source, list->key_count);
		#endif

		return OTF_RETURN_OK;
	}

	static int handleRMAPutRemoteEnd(void* userData, uint64_t time, uint32_t process, uint32_t origin, uint32_t target, uint32_t communicator, uint32_t tag, uint64_t bytes, uint32_t source, OTF_KeyValueList *list)
	{
		#ifdef DEBUG
		printHandleArgs(__FUNCTION__, userData, time, process, origin, target, communicator, tag, bytes, source, list->key_count);
		#endif

		return OTF_RETURN_OK;
	}

	static int handleRMAGet(void* userData, uint64_t time, uint32_t process, uint32_t origin, uint32_t target, uint32_t communicator, uint32_t tag, uint64_t bytes, uint32_t source, OTF_KeyValueList *list)
	{
		#ifdef DEBUG
		printHandleArgs(__FUNCTION__, userData, time, process, origin, target, communicator, tag, bytes, source, list->key_count);
		#endif

		return OTF_RETURN_OK;
	}

	static int handleRMAEnd(void* userData, uint64_t time, uint32_t process, uint32_t remote, uint32_t communicator, uint32_t tag, uint32_t source, OTF_KeyValueList *list)
	{
		#ifdef DEBUG
		printHandleArgs(__FUNCTION__, userData, time, process, remote, communicator, tag, source, list->key_count);
		#endif

		return OTF_RETURN_OK;
	}

	//
	// Snapshots
	//

	static int handleSnapshotComment(void* userData, uint64_t time, uint32_t process, const char* comment, OTF_KeyValueList *list)
	{
		#ifdef DEBUG
		printHandleArgs(__FUNCTION__, userData, time, process, comment, list->key_count);
		#endif

		return OTF_RETURN_OK;
	}

	static int handleEnterSnapshot(void *userData, uint64_t time, uint64_t originaltime, uint32_t function, uint32_t process, uint32_t source, OTF_KeyValueList *list)
	{
		#ifdef DEBUG
		printHandleArgs(__FUNCTION__, userData, time, originaltime, function, process, source, list->key_count);
		#endif

		return OTF_RETURN_OK;
	}

	static int handleSendSnapshot(void *userData, uint64_t time, uint64_t originaltime, uint32_t sender, uint32_t receiver, uint32_t procGroup, uint32_t tag, uint32_t length, uint32_t source, OTF_KeyValueList *list )
	{
		#ifdef DEBUG
		printHandleArgs(__FUNCTION__, userData, time, originaltime, sender, receiver, procGroup, tag, length, source, list->key_count);
		#endif

		return OTF_RETURN_OK;
	}

	static int handleOpenFileSnapshot(void* userData, uint64_t time, uint64_t originaltime, uint32_t fileid, uint32_t process, uint64_t handleid, uint32_t source, OTF_KeyValueList *list)
	{
		#ifdef DEBUG
		printHandleArgs(__FUNCTION__, userData, time, originaltime, fileid, process, handleid, source, list->key_count);
		#endif

		return OTF_RETURN_OK;
	}

	static int handleBeginCollopSnapshot(void* userData, uint64_t time, uint64_t originaltime, uint32_t process, uint32_t collOp, uint64_t matchingId, uint32_t procGroup, uint32_t rootProc, uint64_t sent, uint64_t received, uint32_t scltoken, OTF_KeyValueList *list)
	{
		#ifdef DEBUG
		printHandleArgs(__FUNCTION__, userData, time, originaltime, process, collOp, matchingId, procGroup, rootProc, sent, received, scltoken, list->key_count);
		#endif

		return OTF_RETURN_OK;
	}

	static int handleBeginFileOpSnapshot(void* userData, uint64_t time, uint64_t originaltime, uint32_t process, uint64_t matchingId, uint32_t scltoken, OTF_KeyValueList *list)
	{
		#ifdef DEBUG
		printHandleArgs(__FUNCTION__, userData, time, originaltime, process, matchingId, scltoken, list->key_count);
		#endif

		return OTF_RETURN_OK;
	}

	static int handleCollopCountSnapshot(void* userData, uint64_t time, uint32_t process, uint32_t communicator, uint64_t count, OTF_KeyValueList *list)
	{
		#ifdef DEBUG
		printHandleArgs(__FUNCTION__, userData, time, process, communicator, count, list->key_count);
		#endif

		return OTF_RETURN_OK;
	}

	static int handleCounterSnapshot(void* userData, uint64_t time, uint64_t originaltime, uint32_t process, uint32_t counter, uint64_t value, OTF_KeyValueList *list)
	{
		#ifdef DEBUG
		printHandleArgs(__FUNCTION__, userData, time, originaltime, process, counter, value, list->key_count);
		#endif

		return OTF_RETURN_OK;
	}

	//
	// Statistics
	//

	static int handleSummaryComment(void* userData, uint64_t time, uint32_t process, const char* comment, OTF_KeyValueList *list)
	{
		#ifdef DEBUG
		printHandleArgs(__FUNCTION__, userData, time, process, comment, list->key_count);
		#endif

		return OTF_RETURN_OK;
	}

	static int handleFunctionSummary(void* userData, uint64_t time, uint32_t function, uint32_t process, uint64_t invocations, uint64_t exclTime, uint64_t inclTime, OTF_KeyValueList *list)
	{
		#ifdef DEBUG
		printHandleArgs(__FUNCTION__, userData, time, function, process, invocations, exclTime, inclTime, list->key_count);
		#endif

		return OTF_RETURN_OK;
	}

	static int handleFunctionGroupSummary(void* userData, uint64_t time, uint32_t funcGroup, uint32_t process, uint64_t invocations, uint64_t exclTime, uint64_t inclTime, OTF_KeyValueList *list)
	{
		#ifdef DEBUG
		printHandleArgs(__FUNCTION__, userData, time, funcGroup, process, invocations, exclTime, inclTime, list->key_count);
		#endif

		return OTF_RETURN_OK;
	}

	static int handleMessageSummary(void* userData, uint64_t time, uint32_t process, uint32_t peer, uint32_t comm, uint32_t type, uint64_t sentNumber, uint64_t receivedNumber, uint64_t sentBytes, uint64_t receivedBytes, OTF_KeyValueList *list)
	{
		#ifdef DEBUG
		printHandleArgs(__FUNCTION__, userData, time, process, peer, comm, type, sentNumber, receivedNumber, sentBytes, receivedBytes, list->key_count);
		#endif

		return OTF_RETURN_OK;
	}

	static int handleCollopSummary(void *userData, uint64_t time, uint32_t process, uint32_t comm, uint32_t collective, uint64_t sentNumber, uint64_t receivedNumber, uint64_t sentBytes, uint64_t receivedBytes, OTF_KeyValueList *list)
	{
		#ifdef DEBUG
		printHandleArgs(__FUNCTION__, userData, time, process, comm, collective, sentNumber, receivedNumber, sentBytes, receivedBytes, list->key_count);
		#endif

		return OTF_RETURN_OK;
	}

	static int handleFileOperationSummary(void* userData, uint64_t time, uint32_t fileid, uint32_t process, uint64_t nopen, uint64_t nclose, uint64_t nread, uint64_t nwrite, uint64_t nseek, uint64_t bytesread, uint64_t byteswrite, OTF_KeyValueList *list)
	{
		#ifdef DEBUG
		printHandleArgs(__FUNCTION__, userData, time, fileid, process, nopen, nclose, nread, nwrite, nseek, bytesread, byteswrite, list->key_count);
		#endif

		return OTF_RETURN_OK;
	}

	static int handleFileGroupOperationSummary(void* userData, uint64_t time, uint32_t groupid, uint32_t process, uint64_t nopen, uint64_t nclose, uint64_t nread, uint64_t nwrite, uint64_t nseek, uint64_t bytesread, uint64_t byteswrite, OTF_KeyValueList *list)
	{
		#ifdef DEBUG
		printHandleArgs(__FUNCTION__, userData, time, groupid, process, nopen, nclose, nread, nwrite, nseek, bytesread, byteswrite, list->key_count);
		#endif

		return OTF_RETURN_OK;
	}

	static int handleUnknownRecord(void *userData, uint64_t time, uint32_t process, const char *record)
	{
		#ifdef DEBUG
		printHandleArgs(__FUNCTION__, userData, time, process, record);
		#endif

		return OTF_RETURN_OK;
	}

	//
	// Markers
	//

	static int handleDefMarker(void *userData, uint32_t stream, uint32_t token, const char* name, uint32_t type, OTF_KeyValueList *list)
	{
		#ifdef DEBUG
		printHandleArgs(__FUNCTION__, userData, stream, token, name, type, list->key_count);
		#endif

		return OTF_RETURN_OK;
	}

	static int handleMarker(void *userData, uint64_t time, uint32_t process, uint32_t token, const char* text, OTF_KeyValueList *list)
	{
		#ifdef DEBUG
		printHandleArgs(__FUNCTION__, userData, time, process, token, text, list->key_count);
		#endif

		return OTF_RETURN_OK;
	}

};

#pragma GCC diagnostic pop

#endif
