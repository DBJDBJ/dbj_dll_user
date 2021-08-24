/* (c) 2019/2020 by dbj.org   -- CC BY-SA 4.0 -- https://creativecommons.org/licenses/by-sa/4.0/ */

#define DBJ_DLL_CALL_INCLUDES_WINDOWS
#include "..\dbj_dll_call.hpp"



/*
----------------------------------------------------------------
NOTE! be sure to declare function pointers from WIN32 DLL's
adorning them with WINBASEAPI and WINAPI macros like here
otherwise x86 builds will very likely fail

BOOL Beep( DWORD dwFreq,  // sound frequency, in hertz
DWORD dwDuration  // sound duration, in milliseconds
);

Parameters dwFreq, specifies the frequency, in hertz, of the sound.
This parameter must be in the range 37 through 32,767 (0x25 through 0x7FFF).
*/
typedef WINBASEAPI BOOL(WINAPI* BeepFP) (DWORD, DWORD);

bool beeper_function(BeepFP /*beepFunction*/);

/*
----------------------------------------------------------------
this is one bad example
*/
template<typename AFT>
static auto not_a_good_idea(char const* dll_, char const* fun_)
{
	auto loader_ = ::dbj::win::dll_load(dll_);
	auto function_fetched = loader_.get_function<AFT>(fun_);
	/*
	1. function_fetched fp might be null here
	2. dll loader destructor will unload the dll before the fp is returned
	   thus calling function_fetched will crash the app
	*/
	return function_fetched;
};

extern "C" void  memory_info(FILE*);

/// -----------------------------------------------------
int main(int argc, const char* argv[], char* envp)
{
	redirector stderr_to_file(std::string(argv[0]) + ".log");

/*
NOTE: freeLibrary on dll  will not release memory immediately
most system dll's will simply stay in the memory
But user made dll's unloading will be very benefitial. Best example is resource only dll's
*/
	memory_info(stderr);

	// provoke error
	// see in the log what has happened
	dbj::win::dll_call<BeepFP>(
		"kernel32.dll",
		"Humpty Dumpty",
		[](BeepFP beeper) {
			/* never called */
		}
	);

	// definitely not a good idea
	// in case of kernel32.dll this works because 
	// that is one dll that is almost always pre loaded
	// for some user made dll this will crash the app
	auto beeper = not_a_good_idea<BeepFP>("kernel32.dll", "Beep");

	// if dll is unloaded pointer to its function will not be NULL
	if (beeper)
		// this call will crash the app
		beeper(1000, 1000);

	// just quickly beep once
	auto  rezult = dbj::win::dll_call<BeepFP>(
		"kernel32.dll",
		"Beep",
		[](BeepFP beeper) {
			return beeper(1000, 1000);
		}
	);

	// try some non trivial beep-ing
	auto R = 3U;
	while (R--) {
		auto  rezult = dbj::win::dll_call<BeepFP>(
			"kernel32.dll",
			"Beep",
			beeper_function
			);
	}
	return EXIT_SUCCESS;
}

/// -----------------------------------------------------
static bool  beeper_function(BeepFP beepFunction)
{
	_ASSERTE(beepFunction);

	bool rezult_{};
	unsigned long frequency = 300, duration = 100; // max freq. is 32765;
	const unsigned int f_step = 50, d_step = 50;

	do {
		rezult_ = beepFunction(frequency, duration);
		if (!rezult_) {
			DBJ_DLL_CALL_LOG(__FUNCSIG__ "beepFunction() FAILED ?");
			break;
		}
		frequency -= f_step;

	} while (frequency > 0);
	return rezult_;
};

// EOF

