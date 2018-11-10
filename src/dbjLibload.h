#pragma once
/* Copyright 2017-2018 dbj@dbj.org

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http ://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/
#include <windows.h> // HINSTANCE
#include <string>
#include <exception>
#include <iostream>
#include <string>
#include <string_view>

namespace dbj {

	// nano lib
	namespace nano {

		using namespace ::std;
		using namespace ::std::string_view_literals;

			/*
			Transform any std string or string view
			into any of the 4 the std string types,
			Apache 2.0 (c) 2018 by DBJ.ORG
			*/
			template<typename T, typename F>
		inline T
			transform_to(F str) noexcept
		{
			if (str.empty())
				return {};
			return { std::begin(str), std::end(str) };
		};

		inline exception terror ( wstring_view msg_ )
		{
			return std::runtime_error(
				transform_to<string>(msg_)
			);
		}

		inline std::wstring prefix(std::wstring_view pre, std::wstring_view post)
		{
			return wstring(pre.data()).append(post.data());
		}

		// https://en.cppreference.com/w/cpp/io/clog
		template< typename ... Args>
		inline void log( Args && ... rest) {
			if ((sizeof ... (rest)) > 0)
				( std::wclog << ... << rest );
		}

		constexpr inline const wchar_t path_delimiter = L'\\';

		inline std::wstring appname( std::wstring_view path )
		{
			auto const pos = path.find_last_of( path_delimiter );
			if (std::wstring::npos != pos) {
				return std::wstring{ path.substr(pos + 1) };
			}
			return { path.begin(), path.end() };
		}
	} // nano

	namespace win {

		using namespace ::std;

		// return instance of std::system_error
		inline auto error() 
			->  system_error
		{
			struct error_ final {
				error_() { }
				~error_() { ::SetLastError(0); }
				int operator() () const noexcept {
					return (int)::GetLastError();
				}
			};
			error_ last; 
			return std::system_error( error_code( last(), _System_error_category() ));
		}

		class Libload final
		{
			Libload() {}
		public:
			explicit Libload (
				wstring_view dllFname, 
				bool system_mod = false
			)
				: dllHandle_(NULL), dllName_(dllFname.data()), is_system_module(system_mod)
			{
				_ASSERTE( dllFname.data() != nullptr );
				if (dllName_.empty())
					throw 
					nano::terror (
						__FUNCSIG__
						L" needs dll or exe name as the first argument"
					);

				if (! is_system_module) {
					dllName_ = nano::prefix( L".\\",dllName_);
				}
				// address of filename of executable module 
				dllHandle_ = ::LoadLibraryW(dllName_.c_str());
				if (NULL == dllHandle_) throw nano::terror(
						nano::prefix( 
							L" Could not find the DLL named: ",  
						    dllName_
						)
					);
			}
			/*
			FreeLibrary() failure is very rare and might signal 
			some deep error with the machines or OS
			thus we will not ignore it.
			*/
			~Libload()
			{
				if (NULL != dllHandle_)
					if (!FreeLibrary(dllHandle_))
					{
						system_error sys_e_ = error();
						nano::log(
						L"\ndbj::Libload::FreeLibrary failed. DLL name was " 
						, dllName_
						, "\nlast win error was:", sys_e_.what() );
					}
			}

			/*
			AFT = Actual Function Type
			*/
			template< typename AFT>
			AFT getFunction(wstring_view funName)
			{
				FARPROC result = NULL;
				// GetProcAddress
				// has no unicode equivalent
				_ASSERTE(NULL != dllHandle_);
					result =
					::GetProcAddress(
					(HMODULE)dllHandle_,  // handle to DLL module 
						nano::transform_to<string>( funName ).c_str()
						// name of a function 
					);
				return (AFT)result;
			}

		protected:
			HINSTANCE			dllHandle_;
			std::wstring		dllName_;
			bool				is_system_module = false;
		};

		/* second argument if true means 'system dll is required' */
		inline auto libload( wstring_view dll_, bool is_system_dll = false )
		{
			return dbj::win::Libload(dll_, is_system_dll);
		}
	} // win
} // dbj

