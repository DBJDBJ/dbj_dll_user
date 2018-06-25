#pragma once
/* Copyright 2017 dbj@dbj.org

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
#include <iostream>
#include <cstdlib>      // for std::exit, EXIT_FAILURE, EXIT_SUCCESS  
#include <exception>    // for std::set_terminate  

namespace dbj {

	namespace win {

		class Libload
		{
			/*
			forbidden operations are private, thus unreachable for callers
			or inheritors
			*/
			Libload() {}
		public:
			Libload(const wchar_t * dllFname, bool system_mod = false)
				: dllHandle_(NULL), dllName_(dllFname), is_system_module(system_mod)
			{
				if (dllName_.empty())
					throw std::exception("dbj::Libload ctor needs dll or exe name as the first argument");

				if (is_system_module) {
					using namespace dbj::str;
					prepend(L"\\", dllName_);
					prepend(dbj_simple_system_dir(), dllName_);
				}
				auto libpath = dllName_.c_str();
				dllHandle_ = ::LoadLibrary(libpath);
				// address of filename of executable module 
				if (NULL == dllHandle_)
					throw std::exception(L"Could not find the DLL named: " + dllName_);
			}
			/*
			FreeLibrary() failure is very rare and might signal some deep error with the machines or OS
			thus we will not ignore it.
			NOTE: C++11 standard behavior is not to throw exceptions from destructor
			*/
			~Libload()
			{
				if (NULL != dllHandle_)
					if (!FreeLibrary(dllHandle_))
					{
						std::wclog 
						<< L"dbj::Libload::FreeLibrary failed. DLL name was " 
						+ dllName_ 
						<< std::endl;
						// https://en.cppreference.com/w/cpp/io/clog
					}
			}

			FARPROC getFunction(const wchar_t * funName)
			{
				FARPROC result = NULL;

				if (NULL != dllHandle_)
					result =
					::GetProcAddressW(
					(HMODULE)dllHandle_,  // handle to DLL module 
						std::wstring{funName}.data()		// name of a function 
					);
				return result;
			}

		protected:
			HINSTANCE	dllHandle_;
			std::wstring	dllName_;
			bool is_system_module = false;
		};
	} // win
} // dbj

