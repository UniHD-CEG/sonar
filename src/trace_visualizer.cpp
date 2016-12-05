#include <trace_visualizer.h>

TraceVisualizer::TraceVisualizer(std::shared_ptr<Config> cfg, std::shared_ptr<TraceStats> ts) :
	config(cfg),
	stats(ts)
{
	// check for gnuplot utility
	if (std::system("gnuplot --version > /dev/null"))
		std::cout << "Warning: gnuplot not found, no graphs will be printed!" << std::endl;
	else
		gnuplot_present = true;

#ifdef DEBUG
	ctor_msg(__PRETTY_FUNCTION__);
#endif
}

TraceVisualizer::~TraceVisualizer()
{
	makeInjPlot(config->resdir);
	makeCdfPlot(config->resdir);

#ifdef DEBUG
	dtor_msg(__PRETTY_FUNCTION__);
#endif
}

void TraceVisualizer::addSendP2P(uint32_t proc, uint64_t time, uint64_t bytes)
{
	auto time_abs = stats->getAbsoluteTime(time);
	auto time_rel = stats->getRelativeTime(time);
	injections[proc][P2P_SEND].push_back({time_abs, time_rel, bytes});
}

void TraceVisualizer::addRecvP2P(uint32_t proc, uint64_t time, uint64_t bytes)
{
	auto time_abs = stats->getAbsoluteTime(time);
	auto time_rel = stats->getRelativeTime(time);
	injections[proc][P2P_RECV].push_back({time_abs, time_rel, bytes});
}

void TraceVisualizer::addColl(uint32_t proc, uint64_t time, uint64_t sent, uint64_t recv)
{
	auto time_abs = stats->getAbsoluteTime(time);
	auto time_rel = stats->getRelativeTime(time);
	injections[proc][COLL_SEND].push_back({time_abs, time_rel, sent});
	injections[proc][COLL_RECV].push_back({time_abs, time_rel, recv});
}

void TraceVisualizer::addMessageCDF_P2P(uint32_t proc, uint64_t msg_len)
{
	messages_cdf[proc][P2P][msg_len]++;
}

void TraceVisualizer::addMessageCDF_COLL(uint32_t proc, uint64_t msg_len)
{
	messages_cdf[proc][COLL][msg_len]++;
}

void TraceVisualizer::makeInjPlot(std::string dirname)
{
	std::map<Direction, std::string> type;
	type[P2P_SEND] = "P2P Send";
	type[P2P_RECV] = "P2P Recv";
	type[COLL_SEND] = "Coll. Send";
	type[COLL_RECV] = "Coll. Recv";

	const auto& sep = gnuplot_seperator;

	for (auto x:injections)
	{
		auto proc = x.first;
		std::stringstream filename;
		filename << dirname << "/" << gnuplot_inj_filename_prefix << std::setw(procEnumFill) << std::setfill('0') << proc << ".csv";
		std::ofstream out(filename.str(), std::ofstream::out);

		// set dummy values for gnuplot script
		if (x.second[COLL_SEND].empty())
			x.second[COLL_SEND].push_back({0.0, 0.0, 0});

		if (x.second[COLL_RECV].empty())
			x.second[COLL_RECV].push_back({0.0, 0.0, 0});

		if (x.second[P2P_SEND].empty())
			x.second[P2P_SEND].push_back({0.0, 0.0, 0});

		if (x.second[P2P_RECV].empty())
			x.second[P2P_RECV].push_back({0.0, 0.0, 0});

		for (auto y:x.second)
		{
			// section header
			auto dir = y.first;
			out << "\"" << type[dir] << "\"" << '\n';
			out << "# trace=" << config->tracename << ", node=" << proc << '\n';
			out << "# time_absolute" << sep << "time_relative" << sep << "bytes" << "\n";

			// data
			for (auto z:y.second)
			{
				out << z.time_absolute << sep << z.time_relative << sep << z.bytes << "\n";
			}

			// next data section
			out << "\n\n";
		}

		out.close();
		filename.str(std::string()); // clear stringstream
	}

	std::string gnuplot_scriptfile = "plot_inj.gnuplot";
	if (config->verbose)
		std::cout << "Writing Gnuplot Script ... " << std::flush;
	std::ofstream gnuplot(dirname + "/" + gnuplot_scriptfile);
	gnuplot
		<< "#" << config->tracename << '\n'
		<< "set terminal pngcairo size 800,600 enhanced font 'Arial-Bold,16'" << '\n'
		<< "#set terminal postscript eps enhanced color font 'Arial-Bold,16'" << '\n'
		<< "datafiles = system('ls " << gnuplot_inj_filename_prefix << "*.csv')" << '\n'
		<< "set datafile separator \"" << gnuplot_seperator << "\"" << '\n'
		<< '\n'
		<< "set xrange [0:]" << '\n'
		<< "set autoscale y" << '\n'
		<< "set logscale y" << '\n'
		<< '\n'
		<< "set format y '%1.1e'; set ytics(1e1,1e2,1e3,1e4,1e5,1e6,1e7,1e8,1e9)" << '\n'
		<< '\n'
		<< "set xlabel 'Application Runtime [seconds]'" << '\n'
		<< "set ylabel 'Message Size [Bytes]'" << '\n'
		<< '\n'
		<< "set key horiz out bot center" << '\n'
		<< "set key font ',14' spacing 1.0 samplen 1" << '\n'
		<< '\n'
		<< "node = 1" << '\n'
		<< "do for [file in datafiles] {" << '\n'
		<< "\t#set title sprintf('" << config->tracename << ": Node %d', node) noenhanced" << '\n'
		<< "\tnode = node + 1" << '\n'
		<< "\tset output sprintf('%s.png', file)" << '\n'
		<< "\t#set output sprintf('%s.eps', file)" << '\n'
		<< "\tplot \\" << '\n'
		<< "\t\tfile i 0 u 1:3 w points t columnheader(1), \\" << '\n'
		<< "\t\tfile i 1 u 1:3 w points t columnheader(1), \\" << '\n'
		<< "\t\tfile i 2 u 1:3 w points t columnheader(1), \\" << '\n'
		<< "\t\tfile i 3 u 1:3 w points t columnheader(1)" << '\n'
		<< "}" << '\n'
		<< std::endl;

	if (config->verbose)
		std::cout << " done. " << std::endl;

	if (gnuplot_present)
	{
		if (config->verbose)
			std::cout << "Invoking Gnuplot ... " << std::flush;

		std::string gnuplot_command = "(cd " + dirname + " && " + "gnuplot " + gnuplot_scriptfile + ")";
#ifdef DEBUG
		std::cout << "\n### gnuplot cmd: " << gnuplot_command << std::endl;
#endif
		int ret = std::system(gnuplot_command.c_str());
		if (ret != 0)
			std::cout << "Error: gnuplot returned with non-zero (" << ret << ")" << std::endl;
		else if (config->verbose)
			std::cout << " done. " << std::endl;
	}
}

void TraceVisualizer::makeCdfPlot(std::string dirname)
{
	std::map<MsgType, std::string> type;
	type[P2P] = "P2P";
	type[COLL] = "Collectives";

	const auto& sep = gnuplot_seperator;

	for (auto x:messages_cdf)
	{
		auto proc = x.first;
		std::stringstream filename;
		filename << dirname << "/" << gnuplot_cdf_filename_prefix << std::setw(procEnumFill) << std::setfill('0') << proc << ".csv";
		std::ofstream out(filename.str(), std::ofstream::out);

		// set dummy values for gnuplot script
		if (x.second[COLL].empty())
			x.second[COLL][1] = 1;

		if (x.second[P2P].empty())
			x.second[P2P][1] = 1;

		for (auto y:x.second)
		{
			// section header
			auto dir = y.first;
			out << "\"" << type[dir] << "\"" << '\n';
			out << "# trace=" << config->tracename << ", node=" << proc << '\n';
			out << "# size" << sep << "occurences" << sep << "percentage" << "\n";

			// data
			uint64_t occu_sum = 0;
			for (auto l:y.second)
				occu_sum += l.second;

			double last = 0.0;
			for (auto l:y.second)
			{
				auto size = l.first;
				auto occu = l.second;
				auto perc = static_cast<double>(occu) / static_cast<double>(occu_sum) + last;
				last = perc;
				out << size << sep << occu << sep << perc << "\n";
			}

			// next data section
			out << "\n\n";
		}

		out.close();
		filename.str(std::string()); // clear stringstream
	}

	std::string gnuplot_scriptfile = "plot_cdf.gnuplot";
	if (config->verbose)
		std::cout << "Writing Gnuplot Script ... " << std::flush;
	std::ofstream gnuplot(dirname + "/" + gnuplot_scriptfile);
	gnuplot
		<< "#" << config->tracename << '\n'
		<< "set terminal pngcairo size 800,600 enhanced font 'Arial-Bold,16'" << '\n'
		<< "#set terminal postscript eps enhanced color font 'Arial-Bold,16'" << '\n'
		<< "datafiles = system('ls " << gnuplot_cdf_filename_prefix << "*.csv')" << '\n'
		<< "set datafile separator \"" << gnuplot_seperator << "\"" << '\n'
		<< '\n'
		<< "set logscale x" << '\n'
		<< "set yrange [0:1.02]" << '\n'
		<< '\n'
		<< "set xlabel 'Bytes'" << '\n'
		<< "set ylabel 'Probability'" << '\n'
		<< '\n'
		<< "set grid" << '\n'
		<< "set key horiz out bot center" << '\n'
		<< "set key font ',14' spacing 1.0 samplen 1" << '\n'
		<< '\n'
		<< "node = 1" << '\n'
		<< "do for [file in datafiles] {" << '\n'
		<< "\t#set title sprintf('" << config->tracename << ": Node %d', node) noenhanced" << '\n'
		<< "\tnode = node + 1" << '\n'
		<< "\tset output sprintf('%s.png', file)" << '\n'
		<< "\t#set output sprintf('%s.eps', file)" << '\n'
		<< "\tplot \\" << '\n'
		<< "\t\tfile i 0 u 1:3 w linespoints t columnheader(1), \\" << '\n'
		<< "\t\tfile i 1 u 1:3 w linespoints t columnheader(1)" << '\n'
		<< "}" << '\n'
		<< std::endl;

	if (config->verbose)
		std::cout << " done. " << std::endl;

	if (gnuplot_present)
	{
		if (config->verbose)
			std::cout << "Invoking Gnuplot ... " << std::flush;

		std::string gnuplot_command = "(cd " + dirname + " && " + "gnuplot " + gnuplot_scriptfile + ")";
#ifdef DEBUG
		std::cout << "\n### gnuplot cmd: " << gnuplot_command << std::endl;
#endif
		int ret = std::system(gnuplot_command.c_str());
		if (ret != 0)
			std::cout << "Error: gnuplot returned with non-zero (" << ret << ")" << std::endl;
		else if (config->verbose)
			std::cout << " done. " << std::endl;
	}
}
