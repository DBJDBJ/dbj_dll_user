#pragma once
/* (c) 2019 by dbj.org   -- CC BY-SA 4.0 -- https://creativecommons.org/licenses/by-sa/4.0/ */

#define NOMINMAX
#define min(x, y) ((x) < (y) ? (x) : (y))
#define max(x, y) ((x) > (y) ? (x) : (y))
#define STRICT
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

/*
works without including windows with the declarations bellow 
but this is of a limited value, for conole apps and such
windows apps need to include windows.h anyway
*/
//extern "C" {
//
//	typedef __int64 (__stdcall* DBJ_FARPROC)();
//	/*
//	the declarations for dll dynamic loading
//	*/
//	__declspec(dllimport) void* __stdcall LoadLibraryA(
//		const char* /*lpLibFileName*/
//	);
//
//	__declspec(dllimport) int __stdcall FreeLibrary(
//		void* /*hLibModule*/
//	);
//
//	__declspec(dllimport) DBJ_FARPROC __stdcall GetProcAddress(
//		void* /*hModule*/,
//		const char* /*lpProcName*/
//	);
//
//} // "C
//
//#pragma comment(lib, "kernel32.lib")


/// <summary>
/// user can provide log_function(...)
/// note: in here we log only errors
/// </summary>
#ifndef DBJ_DLL_CALL_LOG
#define DBJ_DLL_CALL_LOG(...) (void)::fprintf_s(stderr, __VA_ARGS__ )
#endif DBJ_DLL_CALL_LOG

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

			dll_load() = delete;
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
					DBJ_DLL_CALL_LOG(
						"dll_load(), needs dll or exe name as the first argument"
					);
					return;
				}
				// address of filename of executable module 
				dll_handle_ = ::LoadLibraryA(dll_name_.c_str());

				if (NULL == dll_handle_)
					DBJ_DLL_CALL_LOG(

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
				if (this->valid())
					if (!::FreeLibrary(dll_handle_))
					{
						DBJ_DLL_CALL_LOG(
							"\ndbj::dll_load::FreeLibrary failed. The DLL name is: %s", dll_name_.c_str());
					}
			}

			/*
			AFT = Actual Function Type
			returns null or function pointer to the one requested
			*/
			template< typename AFT>
			AFT get_function(string_view funName)
			{
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
						funName.data()
					);
#ifndef NDEBUG
				if (result == nullptr)
					DBJ_DLL_CALL_LOG(
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

