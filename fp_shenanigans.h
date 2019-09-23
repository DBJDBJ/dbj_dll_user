#pragma once

// This file is a "Hello, world!" in C++ language by GCC for wandbox.
#include <iostream>
#include <string>
#include <array>
#include <functional>

#define DBJ_TYPENAME(T) typeid(T).name() 

namespace dbj::util {
	// https://stackoverflow.com/a/18682805/10870835
	template<typename T>
	class has_type
	{
		typedef struct { char c[1]; } yes;
		typedef struct { char c[2]; } no;

		template<typename U> static constexpr yes test(typename U::type);
		template<typename U> static constexpr no  test(...);

	public:
		static constexpr bool result = sizeof(test<T>(nullptr)) == sizeof(yes);
	};

	template<typename SIG, typename FP >
	constexpr bool signature_fp_match(FP fp_)
	{
		using c_t = common_type< SIG, FP >;
			if (has_type<c_t>::result) return true;
		return false;
	}

	//template<typename SIG, typename  FP >
	///*constexpr*/ bool signature_fp_match( FP fp_ )
	//{
	//	//if this compiles FP confirms to SIG
	//	std::function<SIG> fo_ = fp_ ;
	//	return true;
	//}
} // dbj::util 

// https://wandbox.org/permlink/HDbvC6PP33BMVTbA

namespace faux_fp {
	using namespace std;
	using namespace std::literals;
	// #define DBJ_TX(x) do { std::cout << std::boolalpha << "\n\nExpression: '" << #x << "'\n\tResult: " << (x) << "\n\tIt's type: " << typeid(x).name()<< "\n\n"; } while (0)

	template<typename T, typename ... A>
	using RequiredSignature = bool(T&, A ... a);

	template<typename T, typename ... A>
	using RequiredFP = bool(*)(T&, A ... a);

	bool ok_fun(int& a) { cout << "\n" << "bool ok_fun(int& " << a << ")";  return true; }
	void wrong_fun() { cout << "\n" << "void wrong_fun( )"; }

	int test(int, const char* [])
	{
		int forty_two = 42;

		using required_sig = RequiredSignature<int>;

		if /*constexpr*/ (dbj::util::signature_fp_match< required_sig >(ok_fun)) {
			DBJ_PRINT("\n\n%sdoes  confirm to the required signature:\n%s ",
				DBJ_TYPENAME(decltype(ok_fun)),
				DBJ_TYPENAME(required_sig));
		}

		if /*constexpr*/ (!dbj::util::signature_fp_match< required_sig >(wrong_fun)) {
 		DBJ_PRINT("\n\n%sdoes not confirm to the required signature:\n%s ", 
			DBJ_TYPENAME(decltype(wrong_fun)),
			DBJ_TYPENAME(required_sig));
		}

		// rezult not true
		auto lambada_ok = ! dbj::util::signature_fp_match< required_sig >([](int &) -> bool { return true;  });

		auto lambada = [](int&) -> bool { return true;  };
		// rezult not true
		auto lambada_ok_2 = ! dbj::util::signature_fp_match< required_sig >( lambada );

		return EXIT_SUCCESS;
	}




} // ns