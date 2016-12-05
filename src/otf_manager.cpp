#include <otf_manager.h>

OTF_Manager::OTF_Manager(const OTF_Manager&)
{
	//TODO: implement me
	// for now: warning suppression
}

OTF_Manager::OTF_Manager(std::shared_ptr<Config> _cfg, uint32_t nfiles, uint32_t buffersize)
{
#ifdef DEBUG
	ctor_msg(__PRETTY_FUNCTION__);
#endif

	// init custom data structures
	udata.cfg = _cfg;
	if (udata.cfg == nullptr)
	{
		throw std::bad_alloc();
	}

	udata.ts = std::make_shared<TraceStats>(udata.cfg);
	udata.tviz = std::make_shared<TraceVisualizer>(udata.cfg, udata.ts);

	// init data structures of OTF library
	manager = OTF_FileManager_open(nfiles);
	if (manager == nullptr)
	{
		throw std::bad_alloc();
	}

	reader = OTF_Reader_open(udata.cfg->otffile.c_str(), manager);
	if (reader == nullptr)
	{
		OTF_FileManager_close(manager);
		throw std::bad_alloc();
	}

	handler_array = OTF_HandlerArray_open();
	if (handler_array == nullptr)
	{
		OTF_Reader_close(reader);
		OTF_FileManager_close(manager);
		throw std::bad_alloc();
	}

	// the template parameter determines the implementation
	// of the handlers to be used
	set_handler_Functions<Sonar<UserData>>();

	OTF_Reader_setBufferSizes(reader, buffersize);
	OTF_Reader_setTimeInterval(reader, 0, std::numeric_limits<uint64_t>::max());

	const uint8_t DISABLE = 0;
	const uint8_t ENABLE  = !DISABLE;

	// enable all processes:
	OTF_Reader_setProcessStatusAll(reader, ENABLE);

	// example: enable only a set of processes:
	//OTF_Reader_setProcessStatusAll(reader, DISABLE); // disable all
	//OTF_Reader_setProcessStatus(reader, 1, ENABLE); // enable proc 1
	//OTF_Reader_setProcessStatus(reader, 2, ENABLE); // enable proc 2
	//...
}

OTF_Manager::~OTF_Manager()
{
	// free OTF lib. data structures
	OTF_HandlerArray_close(handler_array);
	OTF_Reader_close(reader);
	OTF_FileManager_close(manager);

	// print stats on exit
	udata.ts->print();

#ifdef DEBUG
	dtor_msg(__PRETTY_FUNCTION__);
#endif
}

OTF_Manager&
OTF_Manager::operator=(const OTF_Manager& q)
{
	manager = q.manager;
	reader = q.reader;
	handler_array = q.handler_array;
	return *this;
}

void OTF_Manager::read_otf(void)
{
	uint64_t read;
	const std::string otf_read_error_msg = "An error occurred while reading the trace. The files seem to be damaged. Abort.\n(is zlib installed?)";

	read = OTF_Reader_readDefinitions(reader, handler_array);
	std::cout << "Read " << read << " definitions" << std::endl;
	if (read == OTF_READ_ERROR)
	{
		throw std::runtime_error(otf_read_error_msg);
	}

	read = OTF_Reader_readEvents(reader, handler_array);
	std::cout << "Read " << read << " events" << std::endl;
	if (read == OTF_READ_ERROR)
	{
		throw std::runtime_error(otf_read_error_msg);
	}

	read = OTF_Reader_readStatistics(reader, handler_array);
	std::cout << "Read " << read << " statistics" << std::endl;
	if (read == OTF_READ_ERROR)
	{
		throw std::runtime_error(otf_read_error_msg);
	}

	read = OTF_Reader_readSnapshots(reader, handler_array);
	std::cout << "Read " << read << " snapshots" << std::endl;
	if (read == OTF_READ_ERROR)
	{
		throw std::runtime_error(otf_read_error_msg);
	}

	read = OTF_Reader_readMarkers(reader, handler_array);
	std::cout << "Read " << read << " markers" << std::endl;
	if (read == OTF_READ_ERROR)
	{
		throw std::runtime_error(otf_read_error_msg);
	}
}

template<typename T>
void OTF_Manager::set_handler_Functions(void)
{
	using ofp = OTF_FunctionPointer*;
	std::map<int, ofp> handleMap;

	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Wpedantic"
	#pragma GCC diagnostic ignored "-Wunused-parameter"

	// Definitions
	handleMap[OTF_DEFINITIONCOMMENT_RECORD]     = (ofp) &T::handleDefinitionComment;
	handleMap[OTF_DEFTIMERRESOLUTION_RECORD]    = (ofp) &T::handleDefTimerResolution;
	handleMap[OTF_DEFPROCESS_RECORD]            = (ofp) &T::handleDefProcess;
	handleMap[OTF_DEFPROCESSGROUP_RECORD]       = (ofp) &T::handleDefProcessGroup;
	handleMap[OTF_DEFATTRLIST_RECORD]           = (ofp) &T::handleDefAttributeList;
	handleMap[OTF_DEFPROCESSORGROUPATTR_RECORD] = (ofp) &T::handleDefProcessOrGroupAttributes;
	handleMap[OTF_DEFFUNCTION_RECORD]           = (ofp) &T::handleDefFunction;
	handleMap[OTF_DEFFUNCTIONGROUP_RECORD]      = (ofp) &T::handleDefFunctionGroup;
	handleMap[OTF_DEFCOLLOP_RECORD]             = (ofp) &T::handleDefCollectiveOperation;
	handleMap[OTF_DEFCOUNTER_RECORD]            = (ofp) &T::handleDefCounter;
	handleMap[OTF_DEFCOUNTERGROUP_RECORD]       = (ofp) &T::handleDefCounterGroup;
	handleMap[OTF_DEFSCL_RECORD]                = (ofp) &T::handleDefScl;
	handleMap[OTF_DEFCREATOR_RECORD]            = (ofp) &T::handleDefCreator;
	handleMap[OTF_DEFUNIQUEID_RECORD]           = (ofp) &T::handleDefUniqueId;
	handleMap[OTF_DEFVERSION_RECORD]            = (ofp) &T::handleDefVersion;
	handleMap[OTF_DEFFILE_RECORD]               = (ofp) &T::handleDefFile;
	handleMap[OTF_DEFFILEGROUP_RECORD]          = (ofp) &T::handleDefFileGroup;
	handleMap[OTF_DEFKEYVALUE_RECORD]           = (ofp) &T::handleDefKeyValue;
	handleMap[OTF_DEFTIMERANGE_RECORD]          = (ofp) &T::handleDefTimeRange;
	handleMap[OTF_DEFCOUNTERASSIGNMENTS_RECORD] = (ofp) &T::handleDefCounterAssignments;
	handleMap[OTF_DEFPROCESSSUBSTITUTES_RECORD] = (ofp) &T::handleDefProcessSubstitutes;
	handleMap[OTF_DEFAUXSAMPLEPOINT_RECORD]     = (ofp) &T::handleDefAuxSamplePoint;

	// Events
	handleMap[OTF_NOOP_RECORD]                  = (ofp) &T::handleNoOp;
	handleMap[OTF_ENTER_RECORD]                 = (ofp) &T::handleEnter;
	handleMap[OTF_LEAVE_RECORD]                 = (ofp) &T::handleLeave;
	handleMap[OTF_SEND_RECORD]                  = (ofp) &T::handleSendMsg;
	handleMap[OTF_RECEIVE_RECORD]               = (ofp) &T::handleRecvMsg;
	handleMap[OTF_COUNTER_RECORD]               = (ofp) &T::handleCounter;
	handleMap[OTF_COLLOP_RECORD]                = (ofp) &T::handleCollectiveOperation;
	handleMap[OTF_BEGINCOLLOP_RECORD]           = (ofp) &T::handleBeginCollectiveOperation;
	handleMap[OTF_ENDCOLLOP_RECORD]             = (ofp) &T::handleEndCollectiveOperation;
	handleMap[OTF_EVENTCOMMENT_RECORD]          = (ofp) &T::handleEventComment;
	handleMap[OTF_BEGINPROCESS_RECORD]          = (ofp) &T::handleBeginProcess;
	handleMap[OTF_ENDPROCESS_RECORD]            = (ofp) &T::handleEndProcess;
	handleMap[OTF_FILEOPERATION_RECORD]         = (ofp) &T::handleFileOperation;
	handleMap[OTF_BEGINFILEOP_RECORD]           = (ofp) &T::handleBeginFileOperation;
	handleMap[OTF_ENDFILEOP_RECORD]             = (ofp) &T::handleEndFileOperation;
	handleMap[OTF_RMAPUT_RECORD]                = (ofp) &T::handleRMAPut;
	handleMap[OTF_RMAPUTRE_RECORD]              = (ofp) &T::handleRMAPutRemoteEnd;
	handleMap[OTF_RMAGET_RECORD]                = (ofp) &T::handleRMAGet;
	handleMap[OTF_RMAEND_RECORD]                = (ofp) &T::handleRMAEnd;

	// Snapshots
	handleMap[OTF_SNAPSHOTCOMMENT_RECORD]       = (ofp) &T::handleSnapshotComment;
	handleMap[OTF_ENTERSNAPSHOT_RECORD]         = (ofp) &T::handleEnterSnapshot;
	handleMap[OTF_SENDSNAPSHOT_RECORD]          = (ofp) &T::handleSendSnapshot;
	handleMap[OTF_OPENFILESNAPSHOT_RECORD]      = (ofp) &T::handleOpenFileSnapshot;
	handleMap[OTF_BEGINCOLLOPSNAPSHOT_RECORD]   = (ofp) &T::handleBeginCollopSnapshot;
	handleMap[OTF_BEGINFILEOPSNAPSHOT_RECORD]   = (ofp) &T::handleBeginFileOpSnapshot;
	handleMap[OTF_COLLOPCOUNTSNAPSHOT_RECORD]   = (ofp) &T::handleCollopCountSnapshot;
	handleMap[OTF_COUNTERSNAPSHOT_RECORD]       = (ofp) &T::handleCounterSnapshot;

	// Statistics
	handleMap[OTF_SUMMARYCOMMENT_RECORD]        = (ofp) &T::handleSummaryComment;
	handleMap[OTF_FUNCTIONSUMMARY_RECORD]       = (ofp) &T::handleFunctionSummary;
	handleMap[OTF_FUNCTIONGROUPSUMMARY_RECORD]  = (ofp) &T::handleFunctionGroupSummary;
	handleMap[OTF_MESSAGESUMMARY_RECORD]        = (ofp) &T::handleMessageSummary;
	handleMap[OTF_COLLOPSUMMARY_RECORD]         = (ofp) &T::handleCollopSummary;
	handleMap[OTF_FILEOPERATIONSUMMARY_RECORD]  = (ofp) &T::handleFileOperationSummary;
	handleMap[OTF_FILEGROUPOPERATIONSUMMARY_RECORD] = (ofp) &T::handleFileGroupOperationSummary;
	handleMap[OTF_UNKNOWN_RECORD]               = (ofp) &T::handleUnknownRecord;

	// Markers
	handleMap[OTF_DEFMARKER_RECORD]             = (ofp) &T::handleDefMarker;
	handleMap[OTF_MARKER_RECORD]                = (ofp) &T::handleMarker;

	#pragma GCC diagnostic pop

	for (auto &h : handleMap)
	{
		auto fkt_id  = h.first;
		auto fkt_ptr = h.second;
		OTF_HandlerArray_setHandler(handler_array, fkt_ptr, fkt_id);
		OTF_HandlerArray_setFirstHandlerArg(handler_array, &udata, fkt_id);
	}
}
