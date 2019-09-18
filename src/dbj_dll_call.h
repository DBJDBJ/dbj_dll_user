#pragma once
/* (c) 2019 by dbj.org   -- CC BY-SA 4.0 -- https://creativecommons.org/licenses/by-sa/4.0/ */

#include "..//dbj--nanolib/dbj++nanolib.h"
#include <exception>
#include <iostream>

namespace dbj {

	namespace win {

		using namespace ::std;

		// https://en.cppreference.com/w/cpp/io/clog
		template< typename ... Args>
		inline void log(Args&& ... rest) {
			if constexpr ((sizeof ... (rest)) > 0) {
				(std::clog << ... << rest);
			}
		}

/*
dynamic dll loading and fetching a function from the said dll
*/
class dll_load final
{
	HINSTANCE			dll_handle_{};
	std::string		    dll_name_{};
	bool				is_system_module{ true };

	dll_load() {}
public:
	explicit dll_load (
		string_view dll_file_name_, 
		bool system_mod = true
	)
		: dll_handle_(NULL), 
			dll_name_(dll_file_name_.data()), 
			is_system_module(system_mod)
	{
		if (dll_name_.empty()) throw 
			std::runtime_error (
			"dll_load(), needs dll or exe name as the first argument"
			);

		// address of filename of executable module 
		dll_handle_ = ::LoadLibraryA(dll_name_.c_str());

		if (NULL == dll_handle_) throw std::runtime_error (
				" Could not find the DLL named: " + dll_name_
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

				log(
				L"\ndbj::dll_load::FreeLibrary failed. DLL name is: " 
				, dll_name_
				, "\nlast win32 error is:\t", ec.message().c_str() );
			}
	}

	/*		
	AFT = Actual Function Type			
	returns null or function pointer to the one requested
	*/
	template< typename AFT>
	AFT get_function(string_view funName)
	{
		_ASSERTE(NULL != dll_handle_);
		// GetProcAddress
		// has no unicode equivalent
		FARPROC result =
			::GetProcAddress(
			// handle to DLL module 
			(HMODULE)dll_handle_,  
			// name of a function 
			funName.data()
			);
		return (AFT)result;
	}
};

		/*  
		The 'do it all function',
		AFT = Actual Function Type			
		*/
		template< typename AFT>
		inline AFT dll_dyna_load(
			string_view dll_, string_view fun_, 
			bool is_system_dll = true,
			AFT = 0)
		{
			return 
				::dbj::win::dll_load(dll_, is_system_dll)
				.get_function<AFT>(fun_) ;
		}
	} // win
} // dbj

