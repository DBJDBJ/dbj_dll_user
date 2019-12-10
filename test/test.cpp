/* (c) 2019 by dbj.org   -- CC BY-SA 4.0 -- https://creativecommons.org/licenses/by-sa/4.0/ */


#include "..\dbj--simplelog\log.h"

// replace itr with our simple file log
#define DBJ_DLL_CALL_LOG(...) log_error( __VA_ARGS__ )

#include "..\dbj_dll_call.h"

using namespace ::std;
using namespace ::std::string_literals;
using namespace ::std::string_view_literals;

/*
NOTE! be sure to declare function pointers from WIN32 DLL's
adorning them with WINBASEAPI and WINAPI macros like here
otherwise x86 builds will very likely fail

typedef WINBASEAPI BOOL(WINAPI* BeepFP) (DWORD, DWORD);

or in C++ lingo
*/
using  BeepFP = __declspec(dllimport) int (__stdcall* ) (unsigned long, unsigned long);

void beeper(BeepFP /*beepFunction*/);

template<typename F>
	void test_dbj_dll_call
		(string_view /*dll_*/, string_view /*fun_*/, F /*test_fun*/);

	
	int main(int argc, const char* argv[], char* envp)
	{
		if (EXIT_SUCCESS != dbj_simple_log_startup(argv[0]))
			return EXIT_FAILURE;

	// provoke error
	test_dbj_dll_call
	("kernel32.dll"sv, "Humpty Dumpty"sv, beeper);

	auto R = 3U;
	while (R--) {
		test_dbj_dll_call
		("kernel32.dll"sv, "Beep"sv, beeper);
	}
	return EXIT_SUCCESS;
}

/*
NOTE: we could make the test function a template and receive
beeper in where the Beep function usage will be
encapsulated, etc. But here we are just testing the
dynamic dll loading

BOOL Beep( DWORD dwFreq,  // sound frequency, in hertz
DWORD dwDuration  // sound duration, in milliseconds
);

Parameters dwFreq, specifies the frequency, in hertz, of the sound.
This parameter must be in the range 37 through 32,767 (0x25 through 0x7FFF).
*/


// inline auto beeper (BOOL(*beepFunction) (DWORD, DWORD) ) 
static void  beeper(BeepFP beepFunction)
{
	_ASSERTE(beepFunction);

	unsigned long frequency = 300, duration = 100; // max is 32765;
	const unsigned int f_step = 50, d_step = 50;

	do {
		log_info(" ::Beep(%6d, %6d)", frequency, duration);

		if (0 == beepFunction(frequency, duration)) {
			log_error(__FUNCSIG__ "beepFunction() FAILED ?");
			break;
		}
		frequency -= f_step;
	} while (frequency > 0);
};

template<typename F>
static void test_dbj_dll_call
(string_view dll_, string_view fun_, F test_fun)
{
	if (
		// load the dll + get the function
		// third argument if true means 'user dll is required' 
		auto  fp = dbj::win::dll_call<BeepFP>(dll_, fun_)
		; fp // note: C++17 if() syntax
		) {
		log_info("\nCalling the function: %s,\nwith signature %s\nfrom a dll: %s", fun_.data(), typeid(fp).name(), dll_.data());
		test_fun(fp);
	}
	else {
		log_warn("Calling function: %s, from a dll: %s, failed!", fun_.data(), dll_.data());
	}

}; // test_dbj_dll_call



/*---------------------------------------------------------------------------------------------------------*/

