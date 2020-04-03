#pragma once
/* (c) 2019/2020 by dbj.org   -- CC BY-SA 4.0 -- https://creativecommons.org/licenses/by-sa/4.0/ */

#ifndef _WIN32
#error __FILE__ does reuqire WIN32 API 
#endif // _WIN32

#include <assert.h>

/// --------------------------------------------------------------
/// user can provide the actual log function
/// the required signature is
///
/// extern "C" void actual_log_function (const char* , long , const char* , ...);
///
#ifndef DBJ_DLL_USER_PROVIDED_LOG_FUN
///  Our own log function, delberately not C++
/// the simplest way to use this from windows app
/// might be to redirect stderr to a file
/// https://stackoverflow.com/questions/14543443/in-c-how-do-you-redirect-stdin-stdout-stderr-to-files-when-making-an-execvp-or

#include <stdarg.h>
#include <stdio.h>

extern "C" inline void actual_log_function
(const char* filename,	long lineno,	const char* format, 	...)
{
	assert(filename);	assert(lineno);	assert(format);
	va_list args;
	char buffer[BUFSIZ];

	va_start(args, format);
	vsnprintf(buffer, sizeof buffer, format, args);
	va_end(args);

	fprintf(stderr, "\n%s(%lu) : %s", filename, lineno, buffer );
	fflush(stderr);
}
#endif // DBJ_DLL_USER_PROVIDED_LOG_FUN
/// note: in here we log only errors
/// 
#ifndef DBJ_DLL_CALL_LOG
#define DBJ_DLL_CALL_LOG(...) actual_log_function (__FILE__, __LINE__, __VA_ARGS__)
#endif DBJ_DLL_CALL_LOG

/// --------------------------------------------------------------
// by default dbj_dll_call,h does not include windows
#ifdef DBJ_DLL_CALL_INCLUDES_WINDOWS

#undef NOMINMAX
#define NOMINMAX
#undef min
#define min(x, y) ((x) < (y) ? (x) : (y))
#undef max
#define max(x, y) ((x) > (y) ? (x) : (y))
#undef STRICT
#define STRICT
#unedf WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#endif // DBJ_DLL_CALL_INCLUDES_WINDOWS



namespace dbj {

	namespace win {

		using namespace ::std;

		/*
		dynamic dll loading and fetching a function from the said dll
		*/
		class dll_load final
		{
			/// to widen the applicability instead of C++17 solution
			/// inline static const size_t dll_name_len = BUFSIZ;
			/// we shall do a macro (gasp!)
#define DBJ_DLL_LOAD_FILE_LEN BUFSIZ

			HINSTANCE			dll_handle_ = nullptr;
			char  		        dll_name_[BUFSIZ]{0};
			bool				is_system_module{ true };

			dll_load() = delete; // no default ctor

			void assign_dll_name( char const *  name_ )	{
				assert( name_ );
				strncpy_s(dll_name_, name_, DBJ_DLL_LOAD_FILE_LEN);
			}

		public:

			/// we do not do exceptions
			const bool valid() const noexcept {
				return dll_handle_ != nullptr;
			}

			explicit dll_load(
				const char  * dll_file_name_,
				bool system_mod = true
			)
				: dll_handle_(NULL),
				is_system_module(system_mod)
			{
				if (!dll_file_name_) {
					DBJ_DLL_CALL_LOG(
						"dll_load(), needs dll or exe name as the first argument"
					);
					return;
				}

				assign_dll_name(dll_file_name_);

				// address of filename of executable module 
				dll_handle_ = ::LoadLibraryA(dll_name_);

				if (NULL == dll_handle_)
					DBJ_DLL_CALL_LOG(
						" Could not find the DLL by name: %s", dll_name_
					);
			}
			/*
			FreeLibrary() failure is very rare and might signal
			some deep error with the machines or OS
			thus we will not ignore it.
			*/
			~dll_load()
			{
				if (this->valid())
					if (!::FreeLibrary(dll_handle_))
					{
						DBJ_DLL_CALL_LOG(
						"\ndbj::dll_load::FreeLibrary failed. The DLL name is: %s", dll_name_ );
					}
			}

			/*
			AFT = Actual Function Type
			returns null or function pointer to the one requested
			*/
			template< typename AFT>
			AFT get_function(char const * fun_name_ )
			{
				assert(fun_name_);

				if (!this->valid())
				{
					DBJ_DLL_CALL_LOG(
						"instance is not in a valid state!");
					return nullptr;
				}
				// GetProcAddress
				// has no unicode equivalent
				FARPROC result =
					::GetProcAddress(
						// handle to DLL module 
					( HMODULE )dll_handle_,
						// name of a function 
						fun_name_
					);
#ifdef _DEBUG
				if (result == nullptr)
					DBJ_DLL_CALL_LOG(
						"Failed to find the address for a function: %s ", fun_name_ );
#endif // _DEBUG
				return (AFT)result;
			}
		};

		/*
		The 'do it all function',
		AFT = Actual Function Type
		*/
		template< typename AFT>
		inline AFT dll_call(
			char const * dll_, char const * fun_,
			bool is_system_dll = true,
			AFT = 0)
		{
			return
				::dbj::win::dll_load(dll_, is_system_dll)
				.get_function<AFT>(fun_);
		}
	} // win
} // dbj

