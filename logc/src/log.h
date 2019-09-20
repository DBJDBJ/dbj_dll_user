/**
 * Copyright (c) 2017 rxi
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the MIT license. See `log.c` for details.
 */

#ifndef LOG_H
#define LOG_H

#include <stdio.h>
#include <stdarg.h>

#ifndef __cplusplus
#include <stdbool.h>
#endif

#ifdef __cplusplus
namespace dbj::simplelog {
	extern "C" {
#endif // __cplusplus

#define LOG_VERSION "0.1.0"
#define LOG_USE_COLOR

		bool enable_vt_mode();

		// bear in mind in C++ this type full name is
		// dbj::simplelog::log_lock_function_ptr 
		typedef void (*log_lock_function_ptr)(void* udata, int lock);

		enum { LOG_TRACE, LOG_DEBUG, LOG_INFO, LOG_WARN, LOG_ERROR, LOG_FATAL };

		void log_set_udata(void* udata);
		void log_set_lock(log_lock_function_ptr fn);
		void log_set_fp(FILE* fp);
		void log_set_level(int level);
		void log_set_quiet(int enable);

		void log_log(int level, const char* file, int line, const char* fmt, ...);

#ifdef __cplusplus
	} // extern "C" 
} // namespace dbj::simplelog 
#endif // __cplusplus

#ifdef __cplusplus
#include <mutex>

namespace dbj::simplelog {
	namespace mt {
		/*
		C++ MT usage
	   */
		struct log_protector final
		{
			//  process wide mutex instance
			inline static std::mutex log_protector_mutex{};
			// one per instance
			// try to guard the proc wide mutex as soon as constructed
			log_protector() {
				log_protector_mutex.lock();
			}

			~log_protector() {
				// once per instance
				// release the guard on the proc wide mutex instance
				log_protector_mutex.unlock();
			}
		};

		inline log_protector* global_log_protector_ptr = nullptr;

		// typedef void (*log_lock_function_ptr)(void* udata, int lock);
		extern "C" inline void  protector_function(void* udata, int lock)
		{
			log_protector* log_protector_ptr = (log_protector*)udata;

			if (lock)
			{
				_ASSERTE(global_log_protector_ptr == nullptr);
				global_log_protector_ptr = new log_protector{};
			}
			else {
				// unlock
				_ASSERTE(global_log_protector_ptr != nullptr);
				delete global_log_protector_ptr;
				global_log_protector_ptr = nullptr;
			}
		}

		inline void setup() {
			log_set_udata(global_log_protector_ptr);
			log_set_lock(protector_function);
		}

		/*
		 ---------------------------------------------------------------------------------------------------------
		C++ MT usage
		   void main() {
			dbj::simplelog::mt::setup();
		   }
		*/
	} // mt

#ifdef _MSC_VER
#define DBJ_PATH_DELIM '\\'
#else
#define DBJ_PATH_DELIM '/'
#endif

	std::string app_to_log_file_name (
		std::string_view full_exe_path,
		char const * const suffix = ".log.txt"
		)
	{
		auto pos_ = full_exe_path.find_last_of(DBJ_PATH_DELIM);
		if (pos_ == full_exe_path.npos) return {};
		auto basename = full_exe_path.substr(pos_);
		return std::string{ basename.data() }.append(suffix);
	}

#undef DBJ_PATH_DELIM

} // namespace dbj::simplelog;

#endif //  __cplusplus
#ifndef __cplusplus
#define log_trace(...) log_log(LOG_TRACE, __FILE__, __LINE__, __VA_ARGS__)
#define log_debug(...) log_log(LOG_DEBUG, __FILE__, __LINE__, __VA_ARGS__)
#define log_info(...)  log_log(LOG_INFO,  __FILE__, __LINE__, __VA_ARGS__)
#define log_warn(...)  log_log(LOG_WARN,  __FILE__, __LINE__, __VA_ARGS__)
#define log_error(...) log_log(LOG_ERROR, __FILE__, __LINE__, __VA_ARGS__)
#define log_fatal(...) log_log(LOG_FATAL, __FILE__, __LINE__, __VA_ARGS__)
#else
#define log_trace(...) dbj::simplelog::log_log(dbj::simplelog::LOG_TRACE, __FILE__, __LINE__, __VA_ARGS__)
#define log_debug(...) dbj::simplelog::log_log(dbj::simplelog::LOG_DEBUG, __FILE__, __LINE__, __VA_ARGS__)
#define log_info(...)  dbj::simplelog::log_log(dbj::simplelog::LOG_INFO,  __FILE__, __LINE__, __VA_ARGS__)
#define log_warn(...)  dbj::simplelog::log_log(dbj::simplelog::LOG_WARN,  __FILE__, __LINE__, __VA_ARGS__)
#define log_error(...) dbj::simplelog::log_log(dbj::simplelog::LOG_ERROR, __FILE__, __LINE__, __VA_ARGS__)
#define log_fatal(...) dbj::simplelog::log_log(dbj::simplelog::LOG_FATAL, __FILE__, __LINE__, __VA_ARGS__)
#endif // !__cplusplus



#endif
