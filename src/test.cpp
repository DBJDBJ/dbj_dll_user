
#include "dbjlibload.h"

namespace {
			/*
			Please remeber -- std runtime error  is thrown on any error
	
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

inline auto beeper (BOOL(*beepFunction) (DWORD, DWORD) ) 
{
	_ASSERTE(beepFunction);

	DWORD frequency = 2400, duration = 100 ; // max is 32765;
	const UINT f_step = 50, d_step = 50 ;

	do {
		::wprintf(L"\t{F = %6d , D = %6d}", frequency, duration);

		if (0 == beepFunction(frequency, duration)) {
			dbj::nano::log(L" FAILED ?");
			break;
		}
		frequency -= f_step;
	} while (frequency > 0);
};

	using namespace ::std::string_view_literals;

	inline auto test_dbjLibload 
		= [&] (	auto dll_ , auto fun_ , auto test_fun ) -> void
	{
		/* second argument if true means 'system dll is required' */
		auto dll = dbj::win::libload(dll_, true);
			BeepFP  fp = dll.getFunction<BeepFP>(fun_);
		if (!fp) {
			throw 
			dbj::nano::terror(
			  dbj::nano::prefix(
				fun_ , L" -- Function not found?"
			));
		}
		::wprintf(L"\nTesting function %s '%S', from '%s'\n", fun_.data(), typeid(fp).name(), dll_.data() );
		test_fun(fp);
		// free-ing dll lib loaded 
		// happens here
				
	}; // test_dbjLibload
} 

int wmain(int argc, const wchar_t * argv[], wchar_t * envp) {

	using namespace dbj::nano; 
	log("\n", appname( argv[0] ) , "\n" );
	int exit_code = EXIT_SUCCESS;

	try {
		test_dbjLibload(L"kernel32.dll"sv, L"Beep"sv, beeper );
	}
	catch ( const std::exception & rex )
	{
		log( L"\nRun time exception: ", rex.what() );
		exit_code = EXIT_FAILURE;
	}
	catch (...)
	{
		log(L"\nUnknown exception: ");
		exit_code = EXIT_FAILURE;
	}

	log("\n");
	return exit_code;
}