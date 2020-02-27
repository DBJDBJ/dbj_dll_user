/* (c) 2019/2020 by dbj.org   -- CC BY-SA 4.0 -- https://creativecommons.org/licenses/by-sa/4.0/ */


#include "..\dbj_dll_call.h"

#include <string>
#include <typeinfo>
#include <io.h>

/// -----------------------------------------------------
/// redirect stderr to file
/// warning: no error checking whatsoever
struct redirector final {
	 int fd{};
	 fpos_t pos{};
	 FILE* stream{};

	 redirector( std::string filename )
	{
		fflush(stderr);
		fgetpos(stderr, &pos);
		fd = _dup(_fileno(stderr));
		errno_t err = freopen_s(&stream, filename.c_str(), "w", stderr);

		if (err != 0) {
			perror("error on freopen");
			exit( EXIT_FAILURE); // a bit drastic?
		}
	}

	~redirector()
	{
		fflush(stdout);
		int dup2_rezult_ = _dup2(fd, _fileno(stderr));
		_close(fd);
		clearerr(stderr);
		fsetpos(stderr, &pos);
	}
}; // redirector

/*
NOTE! be sure to declare function pointers from WIN32 DLL's
adorning them with WINBASEAPI and WINAPI macros like here
otherwise x86 builds will very likely fail
*/
typedef WINBASEAPI BOOL(WINAPI* BeepFP) (DWORD, DWORD);

void beeper(BeepFP /*beepFunction*/);

template<typename F>
	void test_dbj_dll_call
		(char const * /*dll_*/, char const*  /*fun_*/, F /*test_fun*/);

	
	int main(int argc, const char* argv[], char* envp)
	{
		redirector stderr_to_file( std::string(argv[0]) + ".log" );
	// provoke error
	test_dbj_dll_call
	("kernel32.dll", "Humpty Dumpty", beeper);

	auto R = 3U;
	while (R--) {
		test_dbj_dll_call
		("kernel32.dll", "Beep", beeper);
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
		DBJ_DLL_CALL_LOG(" ::Beep(%6d, %6d)", frequency, duration);

		if (0 == beepFunction(frequency, duration)) {
			DBJ_DLL_CALL_LOG(__FUNCSIG__ "beepFunction() FAILED ?");
			break;
		}
		frequency -= f_step;

	} while (frequency > 0);
};

template<typename F>
static void test_dbj_dll_call
(char const * dll_, char const* fun_, F test_fun)
{
	if (
		// load the dll + get the function
		// third argument if true means 'user dll is required' 
		auto  fp = dbj::win::dll_call<BeepFP>(dll_ , fun_)
		; fp // note: C++17 if() syntax
		) {
		DBJ_DLL_CALL_LOG("\nCalling the function: %s,\nwith signature %s\nfrom a dll: %s", fun_, typeid(fp).name(), dll_);
		test_fun(fp);
	}
	else {
		DBJ_DLL_CALL_LOG("Calling function: %s, from a dll: %s, failed!", fun_, dll_);
	}

}; // test_dbj_dll_call



/*---------------------------------------------------------------------------------------------------------*/

