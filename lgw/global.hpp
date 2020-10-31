#pragma once

#ifdef LGW_OPTIMIZE
	#ifdef _WIN32
		#define WIN32_LEAN_AND_MEAN
		#include <Windows.h>
		#define LGW_OPTIMIZE_CYCLES Sleep(1);
	#endif
	
	#ifdef __linux__
		#include "unistd.h"
		#define LGW_OPTIMIZE_CYCLES usleep(1);
	#endif
#else
	#define LGW_OPTIMIZE_CYCLES
#endif

#ifdef LGW_LOG
#define lgw_log(t, m) std::cout << "[" << #t << ": " << __FUNCTION__ << "] " << m << "\n";
#else
#define lgw_log(t, m)
#endif

#ifdef LGW_ENABLE_MUTEXES
	#define LGW_LOCKED_LOCAL(code)\
		m_window_access_mtx.lock(); \
			code \
		m_window_access_mtx.unlock();

	#define LGW_LOCKED_CTX(code)\
		ctx_window_access_mtx.lock(); \
			code \
		ctx_window_access_mtx.unlock();
#else
	#define LGW_LOCKED_LOCAL(code) code
	#define LGW_LOCKED_CTX(code) code
#endif
