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
		inline T transform_to(F *) = delete;

	} // nano

/*
one should not do anything inside std space or inherit
from any std abstractions, thus
we deliberately do not inherit from std::exception
dbj::exception is the same interface and functionality
as std::exception
Also. We do not want to be "mixed" with things from the std space.
*/
#pragma warning(push)
#pragma warning(disable: 4577) // 'noexcept' used with no exception handling mode specified
	class exception
	{
		std::string data_{};
	public:

		exception() noexcept : data_() {	}

		explicit exception(char const* const message_) noexcept
			: data_(message_)
		{
			_STL_VERIFY(nullptr != message_, __FUNCSIG__ " constructor argument can not be a null");
		}

		exception(wchar_t const* const message_, int) noexcept
			: data_(::dbj::nano::transform_to<string>(wstring(message_)))
		{
			_STL_VERIFY(nullptr != message_, __FUNCSIG__ " constructor argument can not be a null");
		}

		template< typename C >
		exception (std::basic_string_view<C> msg_) 
			: data_(::dbj::nano::transform_to<string>(msg_))
		{
			_STL_VERIFY(msg_.size() > 0 , __FUNCSIG__ " constructor argument can not be a null");
		}

		template< typename C >
		exception (std::basic_string<C> msg_) 
			: data_(::dbj::nano::transform_to<string>(msg_))
		{
			_STL_VERIFY(msg_.size() > 0 , __FUNCSIG__ " constructor argument can not be a null");
		}

		exception(char const* const message_, int) noexcept
			: data_(message_)
		{
			_STL_VERIFY(nullptr != message_, __FUNCSIG__ " constructor argument can not be a null");
		}

		exception(exception const& other_) noexcept
			: data_(other_.data_)
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

#pragma warning(pop)

	// nano lib
	namespace nano {

		using namespace ::std;
		using namespace ::std::string_view_literals;


		template< typename C >
		inline ::dbj::exception terror(basic_string_view<C> msg_) {
			return ::dbj::exception(
				transform_to<string>(msg_)
			);
		}

		template< typename C >
		inline ::dbj::exception terror(basic_string<C> msg_) {
			return terror(basic_string_view<C>(msg_.c_str()));
		}

		template< typename C >
		inline std::basic_string<C> prefix(
			const std::basic_string_view<C> & pre,
			const std::basic_string_view<C> & post
		)
		{
			return std::basic_string<C>(pre.data()).append(post.data());
		}

		std::wostream & operator << (std::wostream & wo, const std::string & s_) 
		{
			return wo << s_.c_str();
		}

		// https://en.cppreference.com/w/cpp/io/clog
		template< typename ... Args>
		inline void log(Args && ... rest) {
			if constexpr ((sizeof ... (rest)) > 0) {
				(std::wclog << ... << rest);
			}
		}
	} // nano
} // dbj