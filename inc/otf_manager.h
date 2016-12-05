#ifndef OTF_MANAGER_H_
#define OTF_MANAGER_H_

#include <map>
#include <string>
#include <limits>
#include <memory>

#include <otf.h>

#include <globals.h>
#include <config.h>
#include <otf_handler.h>
#include <otf_handler_sonar.h>
#include <trace_stats.h>
#include <trace_visualizer.h>

//RAII Class
class OTF_Manager {
	private:
		//dont use intelligent pointers, as they will be given back to the library
		OTF_FileManager  *manager       {nullptr};
		OTF_Reader       *reader        {nullptr};
		OTF_HandlerArray *handler_array {nullptr};

		struct UserData {
			std::shared_ptr<Config>          cfg  {};
			std::shared_ptr<TraceStats>      ts   {};
			std::shared_ptr<TraceVisualizer> tviz {};
		} udata {};

	public:
		OTF_Manager(const OTF_Manager&);
		OTF_Manager(std::shared_ptr<Config> _cfg, uint32_t nfiles=100, uint32_t buffersize=4*1024);
		~OTF_Manager();

		OTF_Manager& operator=(const OTF_Manager& q);

		void read_otf(void);
		template<typename T> void set_handler_Functions(void);
};

#endif
