/* (c) 2019/2020 by dbj.org   -- CC BY-SA 4.0 -- https://creativecommons.org/licenses/by-sa/4.0/ */

// by default dbj_dll_call,h does not include windows

#define NOMINMAX
#define min(x, y) ((x) < (y) ? (x) : (y))
#define max(x, y) ((x) > (y) ? (x) : (y))
#define STRICT
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "..\dbj_dll_call.h"

#include <string>
#include <typeinfo>
#include <io.h>

/// -----------------------------------------------------
/// redirect stderr to file
/// warning: not enough error checking
struct redirector final {
	int fd{};
	fpos_t pos{};
	FILE* stream{};

	redirector(std::string filename)
	{
		fflush(stderr);
		fgetpos(stderr, &pos);
		fd = _dup(_fileno(stderr));
		errno_t err = freopen_s(&stream, filename.c_str(), "w", stderr);

		if (err != 0) {
			perror("error on freopen");
			exit(EXIT_FAILURE); // a bit drastic?
		}

		SYSTEMTIME lt;
		GetLocalTime(&lt);

		fprintf(stderr, "\n\n");
		fprintf(stderr, "\n*****                                                                     *****");
		fprintf(stderr, "\n*****  LOG BEGIN                                                          *****");
		fprintf(stderr, "\n*****                                                                     *****");
		fprintf(stderr, "\n\nLocal time:%4d-%02d-%02d %02d:%02d:%02d\n\n", lt.wYear, lt.wMonth, lt.wDay, lt.wHour, lt.wMinute, lt.wSecond);
	}

	~redirector()
	{
		/// if you do this too soon
		/// stderr might not outpout to file but to 
		/// non existent console
		/// 
#if 0
		fflush(stderr);
		int dup2_rezult_ = _dup2(fd, _fileno(stderr));
		_close(fd);
		clearerr(stderr);
		fsetpos(stderr, &pos);
#endif
	}
}; // redirector

/*
NOTE! be sure to declare function pointers from WIN32 DLL's
adorning them with WINBASEAPI and WINAPI macros like here
otherwise x86 builds will very likely fail
*/
typedef WINBASEAPI BOOL(WINAPI* BeepFP) (DWORD, DWORD);

void beeper_function(BeepFP /*beepFunction*/);

template<typename F>
void test_dbj_dll_call
(char const* /*dll_*/, char const*  /*fun_*/, F /*test_fun*/);

/*
this is one bad example
*/
static auto not_a_good_idea(char const* dll_, char const* fun_)
{
	// load the dll + get the function
	// third argument if true means 'user dll is required' 
	auto  fp = dbj::win::dll_fetch<BeepFP>(dll_, fun_);
	/*
	1. fp might be null here
	2. dll loader destructor will unload the dll before the fp is returned
	*/
	return fp;
};

/// -----------------------------------------------------
int main(int argc, const char* argv[], char* envp)
{
	redirector stderr_to_file(std::string(argv[0]) + ".log");

	// provoke error
	// see what happens
	test_dbj_dll_call
	("kernel32.dll", "Humpty Dumpty", beeper_function);

	// see what will happen if using function
	// from unloaed dll
	auto beeper = not_a_good_idea("kernel32.dll", "Beep");
	// should work,but definitely not a good idea
	// in case of kernel32.dll this works because 
	// that is one dll that is perhaps always loaded
	// with any kind of windows program
	// see test_dbj_dll_call for a safer approach
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
		test_dbj_dll_call
		("kernel32.dll", "Beep", beeper_function);
	}
	return EXIT_SUCCESS;
}

/*
 -----------------------------------------------------
BOOL Beep( DWORD dwFreq,  // sound frequency, in hertz
DWORD dwDuration  // sound duration, in milliseconds
);

Parameters dwFreq, specifies the frequency, in hertz, of the sound.
This parameter must be in the range 37 through 32,767 (0x25 through 0x7FFF).
*/


// inline auto beeper (BOOL(*beepFunction) (DWORD, DWORD) ) 
static void  beeper_function(BeepFP beepFunction)
{
	_ASSERTE(beepFunction);

	unsigned long frequency = 300, duration = 100; // max is 32765;
	const unsigned int f_step = 50, d_step = 50;

	do {
		DBJ_DLL_CALL_LOG(" ::Beep(%6d, %6d)", frequency, duration);

		if (0 == beepFunction(frequency, duration)) {
			DBJ_DLL_CALL_LOG(__FUNCSIG__ "beepFunction() FAILED ?");
			break;
		}
		frequency -= f_step;

	} while (frequency > 0);
};

/*
----------------------------------------------------
We deliberately do not do trivial dll function usage
to show we can carry arround the function pointer
we fetched from a dll
*/
template<typename F>
static void test_dbj_dll_call
(char const* dll_, char const* fun_, F test_fun)
{
	if (
		// load the dll + get the function
		// third argument if true means 'user dll is required' 
		auto  fp = dbj::win::dll_fetch<BeepFP>(dll_, fun_)
		; fp
		// note: C++17 if() syntax
		) {
		DBJ_DLL_CALL_LOG("\nCalling the function: %s,\nwith signature %s\nfrom a dll: %s", fun_, typeid(fp).name(), dll_);

		/*
		we indeed send the fp further down but we do not return it
		thus if will function properly because dll loader destructor
		has not unloaded the dll used before this function is finished
		*/
		test_fun(fp);
	}
	else {
		DBJ_DLL_CALL_LOG("Calling function: %s, from a dll: %s, failed!", fun_, dll_);
	}

}; // test_dbj_dll_call

/*---------------------------------------------------------------------------------------------------------*/

