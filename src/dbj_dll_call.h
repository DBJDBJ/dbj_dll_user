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
#include <windows.h> // HINSTANCE
#include <crtdbg.h> 
#include <string>
#include <exception>
#include <iostream>
#include <string>
#include <string_view>

namespace dbj {

	using ::std::string;
	using ::std::wstring;

	namespace nano 
	{
		template<typename T, typename F>
		inline T
			transform_to(F str) noexcept
		{
			if (str.empty()) return {};
			return { std::begin(str), std::end(str) };
		};
		// no native pointers please
		template<typename T, typename F>
		inline T transform_to(F * ) = delete;

	} // nano

	/*
	one should not do anything inside std space or inherit
	from any std abstractions, thus
	we deliberately do not inherit from std::exception
	dbj::exception is the same interface and functionality
	as std::exception
	*/
#pragma warning(push)
#pragma warning(disable: 4577) // 'noexcept' used with no exception handling mode specified
	class exception
	{
		std::string data_{};
	public:

		exception() noexcept : data_() {	}

		explicit exception(char const* const message_ ) noexcept
			: data_(message_)
		{
			_STL_VERIFY(nullptr != message_, __FUNCSIG__ " constructor argument can not be a null");
		}

		exception(wchar_t const* const message_, int) noexcept
			: data_( ::dbj::nano::transform_to<string>(wstring(message_)) )
		{
			_STL_VERIFY(nullptr != message_, __FUNCSIG__ " constructor argument can not be a null");
		}

		exception(char const* const message_, int) noexcept
			: data_(message_)
		{
			_STL_VERIFY(nullptr != message_, __FUNCSIG__ " constructor argument can not be a null");
		}

		exception(exception const& other_ ) noexcept
			: data_( other_.data_ )
		{
		}

		exception& operator=(exception const& other_) noexcept
		{
			if (this == &other_) {
				return *this;
			}
				data_ = other_.data_;
			return *this;
		}

		virtual ~exception() noexcept
		{
			data_.clear();
		}

		virtual char const* what() const
		{
			return data_.size() > 1 ? data_.c_str() : "Unknown dbj exception";
		}
	};

	// dbj runtime_error
	class runtime_error
		: public ::dbj::exception
	{	// base of all runtime-error exceptions
	public:
		typedef ::dbj::exception parent_ ;

		explicit runtime_error(const string& message_)
			: parent_(message_.c_str())
		{	// construct from message string
		}

		explicit runtime_error(const char *message_)
			: parent_(message_)
		{	// construct from message string
		}
	};
#pragma warning(pop)

	// nano lib
	namespace nano {

		using namespace ::std;
		using namespace ::std::string_view_literals;


		template< typename C >
		inline ::dbj::exception terror ( basic_string_view<C> msg_ ) {
			return ::dbj::runtime_error(
				transform_to<string>(msg_)
			);
		}

		template< typename C >
		inline ::dbj::exception terror(basic_string<C> msg_) {
			return terror(basic_string_view<C>( msg_.c_str())  );
		}

		template< typename C, size_t N >
		inline ::dbj::exception terror(const C (&msg_)[N] ) {
			return terror(basic_string_view<C>(msg_));
		}


		template< typename C >
		inline std::basic_string<C> prefix(
			std::basic_string_view<C> pre, 
			std::basic_string_view<C> post
		)
		{
			return std::basic_string<C>(pre.data()).append(post.data());
		}

		// https://en.cppreference.com/w/cpp/io/clog
		template< typename ... Args>
		inline void log( Args && ... rest) {
			if constexpr ((sizeof ... (rest)) > 0) {
				(std::wclog << ... << rest);
			}
		}
	} // nano

	namespace win {

		using namespace ::std;

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
			dll_load() {}
		public:
			explicit dll_load (
				wstring_view dll_file_name_, 
				bool system_mod = false
			)
				: dll_handle_(NULL), dll_name_(dll_file_name_.data()), is_system_module(system_mod)
			{
				if (dll_name_.empty()) throw 
					nano::terror (
						(__FUNCSIG__
						" needs dll or exe name as the first argument")
					);

				if (! is_system_module) {
					dll_name_ =  L".\\" + dll_name_ ;
				}
				// address of filename of executable module 
				dll_handle_ = ::LoadLibraryW(dll_name_.c_str());
				if (NULL == dll_handle_) throw nano::terror(
					 L" Could not find the DLL named: " + dll_name_
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
					if (!FreeLibrary(dll_handle_))
					{
						system_error sys_e_ = error_instance();
						nano::log(
						L"\ndbj::dll_load::FreeLibrary failed. DLL name was " 
						, dll_name_
						, "\nlast win32 error is:", sys_e_.what() );
					}
			}

			/*		
			AFT = Actual Function Type			
			returns null or function pointer to the one requested
			*/
			template< typename AFT>
			AFT get_function(wstring_view funName)
			{
				FARPROC result = NULL;
				// GetProcAddress
				// has no unicode equivalent
				_ASSERTE(NULL != dll_handle_);
					result =
					::GetProcAddress(
					(HMODULE)dll_handle_,  // handle to DLL module 
						nano::transform_to<string>( funName ).c_str()
						// name of a function 
					);
				return (AFT)result;
			}

		protected:
			HINSTANCE			dll_handle_;
			std::wstring		dll_name_;
			bool				is_system_module = false;
		};

		/* second argument if true means 'system dll is required' */
		inline auto dll_dyna_load( wstring_view dll_, bool is_system_dll = false )
		{
			return ::dbj::win::dll_load(dll_, is_system_dll);
		}

		/*  or do it all function,	AFT = Actual Function Type			*/
		template< typename AFT>
		inline AFT dll_call(
			wstring_view dll_, wstring_view fun_, 
			bool is_system_dll = false)
		{
			return 
				::dbj::win::dll_load(dll_, is_system_dll).get_function<AFT>(fun_) ;
		}

	} // win
} // dbj

