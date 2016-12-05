#include <config.h>

inline static std::string now(void)
{
	/* returns the current time as string; format: YYYY-MM-DD_hh-mm-ss */

	time_t now;
	time(&now);
	struct tm *t = localtime(&now);

	char buf[128];
	sprintf(buf, "%04d-%02d-%02d_%02d-%02d-%02d",
		1900+t->tm_year, 1+t->tm_mon, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec);

	return std::string(buf);
}

static void usage(const char* prog)
{
#ifdef DEBUG
		std::cout
			<< "\nDEBUG BUILD\n"
			<< "Built on " << __DATE__ << ", " << __TIME__ << "\n"
			<< std::endl;
#endif
		std::cout
			<< "Usage: " << prog << " [OPTIONS] trace.otf" << '\n'
			<< '\n'
			<< "Options:" << '\n'
			<< "  -h | --help     - show help" << '\n'
			<< "  -v | --verbose  - be verbose" << '\n'
			<< "  -r | --rawotf   - show unmodified content of OTF-trace" << '\n'
			<< "  -p | --progress - show progress" << '\n'
			<< "  -s | --toscreen - print results to screen" << '\n'
			<< std::endl;
}

Config::Config(const int argc, const char* argv[])
{
#ifdef DEBUG
	_verbose = true;
	_progress = true;
	_stats_toscreen = true;
#endif

	if (argc == 1)
	{
		usage(argv[0]);
		std::exit(0);
	}
	else if (argc == 2)
	{
		if (!strcmp("-h", argv[1]) || !strcmp("--help", argv[1]))
		{
			usage(argv[0]);
			std::exit(0);
		}
		else if (argv[1][0] == '-')
		{
			throw std::invalid_argument("Unknow argument: '" + (std::string)argv[1] + "'");
		}
	}
	else
	{
		for (int i=1; i<argc-1; ++i)
		{
			#ifdef DEBUG
			std::cout << "### Current Arg: " << argv[i] << ", " << argv[i+1] << " next" << std::endl;
			#endif

			if (!strcmp("--help", argv[i]) || !strcmp("-h", argv[i]))
			{
				usage(argv[0]);
				std::exit(0);
			}
			else if (!strcmp("--verbose", argv[i]) || !strcmp("-v", argv[i]))
			{
				std::cout << "Verbose Mode: On" << std::endl;
				_verbose = true;
			}
			else if (!strcmp("--rawotf", argv[i]) || !strcmp("-r", argv[i]))
			{
				_rawotf = true;
			}
			else if (!strcmp("--progress", argv[i]) || !strcmp("-p", argv[i]))
			{
				_progress = true;
			}
			else if (!strcmp("--toscreen", argv[i]) || !strcmp("-s", argv[i]))
			{
				_stats_toscreen = true;
			}
			else
			{
				throw std::invalid_argument("Unknow argument: '" + (std::string)argv[i] + "'");
			}
		}
	}

	// the last argument must be the OTF file
	_otffile = argv[argc-1];
	#ifdef DEBUG
	std::cout << "### Last arg: " << argv[argc-1] << std::endl;
	#endif

	// check for file extension
	if (_otffile.find("otf") == std::string::npos)
		throw std::invalid_argument("File \"" + otffile + "\" seems not to be of type OTF");

	if (verbose)
		std::cout << "OTF-File: " << otffile << std::endl;

	// get name of trace without path and file extension
	const auto &tmp = otffile;
	const int lastSlash = tmp.rfind("/");
	std::copy(tmp.begin()+lastSlash+1, tmp.end()-4, std::back_inserter(_tracename));
	if (verbose)
		std::cout << "### tracename: " << tracename << std::endl;

	// set and create directory to store results in
	_resdir = tracename + "_SonarResults_" + now();
	if (verbose)
		std::cout << "### resdir: " << resdir << std::endl;

	int ret = mkdir(_resdir.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	if (ret)
		throw std::runtime_error("Can not create directory -> " + _resdir + " (" + strerror(errno) + ")");

#ifdef DEBUG
	ctor_msg(__PRETTY_FUNCTION__);
#endif
}

Config::~Config()
{
#ifdef DEBUG
	dtor_msg(__PRETTY_FUNCTION__);
#endif
}
