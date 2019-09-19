#pragma once
/* (c) 2019 by dbj.org   -- CC BY-SA 4.0 -- https://creativecommons.org/licenses/by-sa/4.0/ */

#define NOMINMAX
#define min(x, y) ((x) < (y) ? (x) : (y))
#define max(x, y) ((x) > (y) ? (x) : (y))
#define STRICT
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

// #include "..//dbj--nanolib/dbj++nanolib.h"
#include "../logc/src/log.h"

namespace dbj {

	namespace win {

		using namespace ::std;

		/*
		dynamic dll loading and fetching a function from the said dll
		*/
		class dll_load final
		{
			HINSTANCE			dll_handle_ = nullptr;
			std::string		    dll_name_{};
			bool				is_system_module{ true };

			dll_load() {}
		public:

			constexpr bool valid() const noexcept {
				return dll_handle_ != nullptr;
			}

			explicit dll_load(
				string_view dll_file_name_,
				bool system_mod = true
			)
				: dll_handle_(NULL),
				dll_name_(dll_file_name_.data()),
				is_system_module(system_mod)
			{
				if (dll_name_.empty()) {
					log_error(
						"dll_load(), needs dll or exe name as the first argument"
					);
					return;
				}
				// address of filename of executable module 
				dll_handle_ = ::LoadLibraryA(dll_name_.c_str());

				if (NULL == dll_handle_)
					log_error(

						" Could not find the DLL by name: %s", dll_name_.c_str()
					);
			}
			/*
			FreeLibrary() failure is very rare and might signal
			some deep error with the machines or OS
			thus we will not ignore it.
			*/
			~dll_load()
			{
				if (NULL != dll_handle_)
					if (!::FreeLibrary(dll_handle_))
					{

						std::error_code ec(::GetLastError(), std::system_category());

						log_error(
							"\ndbj::dll_load::FreeLibrary failed. The DLL name is: "
							"%s"
							"\nlast win32 error is: %s", dll_name_.c_str(), ec.message().c_str());
					}
			}

			/*
			AFT = Actual Function Type
			returns null or function pointer to the one requested
			*/
			template< typename AFT>
			AFT get_function(string_view funName)
			{
				if (nullptr == dll_handle_)
				{
					log_error(
						"instance is not in a valid state!");
					return nullptr;
				}
				// GetProcAddress
				// has no unicode equivalent
				FARPROC result =
					::GetProcAddress(
						// handle to DLL module 
					(HMODULE)dll_handle_,
						// name of a function 
						funName.data()
					);
#ifndef NDEBUG
				if (result == nullptr)
					log_error(
						"Failed to find the address for a function: %s ", funName.data());
#endif // !NDEBUG
				return (AFT)result;
			}
		};

		/*
		The 'do it all function',
		AFT = Actual Function Type
		*/
		template< typename AFT>
		inline AFT dll_call(
			string_view dll_, string_view fun_,
			bool is_system_dll = true,
			AFT = 0)
		{
			return
				::dbj::win::dll_load(dll_, is_system_dll)
				.get_function<AFT>(fun_);
		}
	} // win
} // dbj

