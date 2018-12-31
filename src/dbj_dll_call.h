#pragma once
/* Copyright 2017-2018 dbj@dbj.org

Licensed under the GPL License, Version 3.0 (the "License");
you may not use this file except in compliance with the License.

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/
#include "dbj_nano_lib.h"

namespace dbj {

	namespace win {

		using namespace ::std;

		/*
		make get last error into automatic get/set to 0

		::dbj::win::last_error last ;
		auto last_err = last() ;

		or simply

		auto last_err = (::dbj::win::last_error)() ;
		*/
		struct last_error  final {
			last_error() { }
			~last_error() { ::SetLastError(0); }
			int operator() () const noexcept {
				return (int)::GetLastError();
			}
		};
		// return instance of std::system_error
		// which for MSVC STL delivers win32 last error message
		// by calling what() on it
		//
		// auto last_err_msg = error_instance().what() ;
		//
		inline auto error_instance ( ) 
			->  system_error
		{
			last_error last;
#ifdef _MSC_VER
			return std::system_error( error_code( last(), _System_error_category() ));
#else
			return std::system_error(error_code(last(), system_category()));
#endif
		}

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
					nano::terror (
					"dll_load(), needs dll or exe name as the first argument"sv
					);

				// address of filename of executable module 
				dll_handle_ = ::LoadLibraryA(dll_name_.c_str());

				if (NULL == dll_handle_) throw nano::terror(
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
						system_error sys_e_ = error_instance();
						nano::log(
						L"\ndbj::dll_load::FreeLibrary failed. DLL name is: " 
						, dll_name_
						, "\nlast win32 error is:\t", sys_e_.what() );
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
					(HMODULE)dll_handle_,  // handle to DLL module 
						funName.data()
						// name of a function 
					);
				return (AFT)result;
			}


		};
#if 0
		/* second argument if true means 'system dll is required' */
		inline auto dll_dyna_load
		( string_view dll_, bool is_system_dll = true )
		{
			return ::dbj::win::dll_load(dll_, is_system_dll);
		}
#endif
		/*  or do it all function,	AFT = Actual Function Type			*/
		template< typename AFT>
		inline AFT dll_dyna_load(
			string_view dll_, string_view fun_, 
			bool is_system_dll = true)
		{
			return 
				::dbj::win::dll_load(dll_, is_system_dll)
				.get_function<AFT>(fun_) ;
		}
	} // win
} // dbj

