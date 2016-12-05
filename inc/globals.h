#ifndef _MY_GLOBALS_H_
#define _MY_GLOBALS_H_

inline void ctor_msg(const char* f)
{
	fprintf(stderr, "*** %s ***\n", f);
}

inline void dtor_msg(const char* f)
{
	fprintf(stderr, "*** %s ***\n", f);
}

#endif
