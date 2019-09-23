/* (c) 2019 by dbj.org   -- CC BY-SA 4.0 -- https://creativecommons.org/licenses/by-sa/4.0/ */
#include "dbj_dll_call.h"
#include "dbj_file_handle.h"
#include "..//binary_tree.h"

#include "..//fp_shenanigans.h"

#include "..//logc/src/log.h"

#define DBJ_FILE_LINE  __FILE__ "(" _CRT_STRINGIZE(__LINE__) ")"

using namespace ::std;
using namespace ::std::string_literals;
using namespace ::std::string_view_literals;

namespace 
{

	/*
	NOTE: we could make the test function a template and receive
	beeper in where the Beep function usage will be
	encapsulated, etc. But here we are just testing the
	dynamic dll loadin

	BOOL Beep( DWORD dwFreq,  // sound frequency, in hertz
	DWORD dwDuration  // sound duration, in milliseconds
	);

	Parameters dwFreq, specifies the frequency, in hertz, of the sound.
	This parameter must be in the range 37 through 32,767 (0x25 through 0x7FFF).
	*/
	/*
	!!! be sure to declare function pointers from WIN32 DLL's
	adorning them with WINBASEAPI and WINAPI macros like here
	otherwise x86 builds will very likely fail
	*/
	typedef WINBASEAPI BOOL(WINAPI* BeepFP) (DWORD, DWORD);

	// inline auto beeper (BOOL(*beepFunction) (DWORD, DWORD) ) 
	inline auto beeper(BeepFP beepFunction)
	{
		_ASSERTE(beepFunction);

		DWORD frequency = 300, duration = 100; // max is 32765;
		const UINT f_step = 50, d_step = 50;

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
	inline void test_dbj_dll_call
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

}

/*---------------------------------------------------------------------------------------------------------*/
int main(int argc, const char* argv[], char* envp)
{
	faux_fp::test(argc, argv);
#ifdef TESTING_DBJ_BTREE
	dbj_tree_research::simple_tree_test();
#endif
#ifdef TESTING_DBJ_RETVALS
	tempo_test::now();
#endif
	// locate log file in the same folder with the exe made
	auto lfn = dbj::simplelog::app_to_log_file_name(argv[0]);

	auto [file_handle, status] = dbj::FH::make(lfn);

	if (!file_handle) {
		perror(lfn.c_str());
		return EXIT_FAILURE;
	}

	dbj::simplelog::mt::setup();
	dbj::simplelog::enable_vt_mode();
	dbj::simplelog::log_set_fp(file_handle->file_ptr());

	log_trace(" %s", "=================================================================================");
	log_trace("Starting Application: %s", argv[0]);
	log_trace(" %s", "=================================================================================");

#ifdef LOG_TESTING
	log_trace("Log  TRACE");
	log_debug("Log  DEBUG");
	log_info("Log  INFO");
	log_warn("Log  WARN");
	log_error("Log  ERROR");
	log_fatal("Log  FATAL");
#endif

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

