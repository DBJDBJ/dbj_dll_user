
#include "dbj_dll_call.h"
#include "..//dbj_file_handle.h"

#include "..//logc/src/log.h"
#include "..//..//dbj_cpplog/src/filelog.h"

namespace testing_testing_123  
{
	using namespace ::std;
	using namespace ::std::string_literals;
	using namespace ::std::string_view_literals;
/*
NOTE: we could make the test function a template and receive
beeper in where the Bepp function usage will be
encapsulated, etc. But here we are just testing the
dynamic dll loadin

BOOL Beep( DWORD dwFreq,  // sound frequency, in hertz
DWORD dwDuration  // sound duration, in milliseconds
);

Parameters dwFreq, specifies the frequency, in hertz, of the sound.
This parameter must be in the range 37 through 32,767 (0x25 through 0x7FFF).
*/
	
typedef BOOL(*BeepFP) (DWORD, DWORD);

// inline auto beeper (BOOL(*beepFunction) (DWORD, DWORD) ) 
inline auto beeper (BeepFP beepFunction )
{
	_ASSERTE(beepFunction);

	DWORD frequency = 300, duration = 100 ; // max is 32765;
	const UINT f_step = 50, d_step = 50 ;

	do {
		::wprintf(L"\t{F = %6d , D = %6d}", frequency, duration);

		if (0 == beepFunction(frequency, duration)) {
			dbj::win::log(__FUNCSIG__ "beepFunction() FAILED ?");
			break;
		}
		frequency -= f_step;
	} while (frequency > 0);
	};

template<typename F>
inline void test_dbj_dll_call 
	(	string_view dll_ , string_view fun_ , F test_fun )
{
	if (
		// load the dll + get the function
		// third argument if true means 'user dll is required' 
		auto  fp = dbj::win::dll_dyna_load<BeepFP>(dll_, fun_)
		;fp // note: C++17 if() syntax
	) {
			::dbj::win::log("\nCalling the function: ",	fun_.data(), ",\nwith signature ", typeid(fp).name(), "\nfrom a dll: ", dll_.data());
		test_fun(fp);
	} else {
		throw std::runtime_error(
			std::string(fun_.data()).append( " -- Function not found?").c_str()
		);
	}
}; // test_dbj_dll_call

} 

int main(int argc, const char * argv[], char * envp) 
{
	using namespace ::std;
	using namespace ::std::string_literals;
	using namespace ::std::string_view_literals;

	dbj::FH f_help("d:\\yetanotherlog.txt");

	log_set_fp( f_help.file_ptr() );

	log_trace("Log  TRACE");
	log_debug("Log  DEBUG");
	log_info("Log  INFO");
	log_warn("Log  WARN");
	log_error("Log  ERROR");
	log_fatal("Log  FATAL");


	using ::dbj::win::log ; 
	
	log_info("\n", argv[0] , "\n" );

	int exit_code = EXIT_SUCCESS;

	try {
		// or use a fancy test unit
		DBJ_REPEAT(3) {
			testing_testing_123::test_dbj_dll_call
			("kernel32.dll"sv, "Beep"sv, testing_testing_123::beeper);
		}
	}
	catch ( const std::exception & rex )
	{
		log_error( "\ndbj exception:\n\t", rex.what() );
		exit_code = EXIT_FAILURE;
	}
	catch ( ... )
	{
		log_error( "\nUnknown exception:\n\t" );
		exit_code = EXIT_FAILURE;
	}

	return exit_code;
}