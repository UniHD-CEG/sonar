#ifndef _CONFIG_H_
#define _CONFIG_H_

#include <iostream>
#include <string>
#include <stdexcept>

#include <ctime>
#include <cstring>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <globals.h>

class Config {
private:
	std::string _otffile        {};
	bool        _verbose        { false };
	bool        _rawotf         { false };
	bool        _progress       { false };
	bool        _stats_toscreen { false };

	std::string _tracename      {""};
	std::string _resdir         {""};

public:
	// read-only interface to member variables
	const decltype(_otffile)		&otffile        = _otffile;
	const decltype(_verbose)		&verbose        = _verbose;
	const decltype(_rawotf)			&rawotf         = _rawotf;
	const decltype(_progress)		&progress       = _progress;
	const decltype(_stats_toscreen)	&stats_toscreen = _stats_toscreen;

	const decltype(_tracename)		&tracename      = _tracename;
	const decltype(_resdir)		    &resdir         = _resdir;

	Config(const int argc, const char* argv[]);
	~Config();
};

#endif
