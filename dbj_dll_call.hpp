#pragma once
/* (c) 2019/2020 by dbj.org   -- CC BY-SA 4.0 -- https://creativecommons.org/licenses/by-sa/4.0/ */

#ifndef _WIN32
#error __FILE__ does require WIN32 API 
#endif // _WIN32

// The hope is "any" C++ compiler might compile this header
// thus we will refrain from C++11 and beyond features
// this is completely untested as I have no other compiler 
// but VS 2019 

typedef enum {major = 2, minor = 7, patch = 0 } version ;

#include <assert.h>

/// --------------------------------------------------------------
// by default dbj_dll_call.h does not include windows
// but it needs that
#ifdef DBJ_DLL_CALL_INCLUDES_WINDOWS

#undef NOMINMAX
#define NOMINMAX
#undef min
#define min(x, y) ((x) < (y) ? (x) : (y))
#undef max
#define max(x, y) ((x) > (y) ? (x) : (y))
#undef STRICT
#define STRICT 1
#undef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#endif // DBJ_DLL_CALL_INCLUDES_WINDOWS

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
	assert(filename && lineno && format);
	va_list args;
	char buffer[BUFSIZ] = {0};

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
#endif // DBJ_DLL_CALL_LOG

namespace dbj {
	namespace win {
		// using namespace std;
		/*
		dynamic dll loading and fetching a function from the said dll
		*/
		class dll_load final
		{
			/// to widen the applicability instead of C++17 solution
			/// inline static const size_t dll_name_len = BUFSIZ;
			/// we shall do a macro (gasp!)
#define DBJ_DLL_LOAD_FILE_LEN BUFSIZ

			HINSTANCE		dll_handle_ = nullptr;
			char  		        dll_name_[BUFSIZ]{0};
			bool			is_system_module{ true };

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
                 (void)this->is_system_module; // reserved for future use

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
			void unload () noexcept 
			{
				if (this->valid()) {
					if (!::FreeLibrary(dll_handle_))
					{
						DBJ_DLL_CALL_LOG(
							"\ndbj::dll_load::FreeLibrary failed. The DLL name is: %s", dll_name_);
					}
					this->dll_handle_ = nullptr;
				}
			}

			~dll_load() { unload(); }

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
						"\nFailed to find the address for a function: %s\nThe DLL name is: %s", fun_name_, dll_name_  );
#endif // _DEBUG
				return (AFT)result;
			}
		}; // eof dll_load

		/*
		The do it all function,
		call the callback provided with the 
		pointer of the fetched function.
		if dll load has failed the callback 
		will not be called.
		AFT = Actual Function Type
		CBF = Call Back Function
			void ( * callback ) ( AFT )
		*/
		template< typename AFT, typename CBF >
		inline auto dll_call(
			char const * dll_, // the dll name
			char const * fun_, // the function name
			CBF callback ,
			bool is_system_dll = false ) noexcept
			-> void
		{
			assert( dll_ && fun_ );
			auto loader_ = ::dbj::win::dll_load(dll_, is_system_dll);
			AFT function_fetched = loader_.get_function<AFT>(fun_);
			// if any, failures are already logged
			if (function_fetched)
				callback(function_fetched);
		}
	} // win
} // dbj

