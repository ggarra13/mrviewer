/**
 * @file   mrvCPU.h
 * @author gga
 * @date   Mon Jan 14 21:24:10 2008
 * 
 * @brief  CPU detection routines
 * 
 * 
 */


#ifndef mrvCPU_h
#define mrvCPU_h

#include <string>

typedef struct cpucaps_s {
	int cpuType;
	int cpuModel;
	int cpuStepping;
	int hasMMX;
	int hasMMX2;
	int has3DNow;
	int has3DNowExt;
	int hasSSE;
	int hasSSE2;
	int isX86;
	unsigned cl_size; /* size of cache line */
        int hasAltiVec;
	int hasTSC;
} CpuCaps;

extern CpuCaps gCpuCaps;

std::string GetCpuCaps(CpuCaps *caps);

unsigned int cpu_count();

#endif // mrvCPU_h

