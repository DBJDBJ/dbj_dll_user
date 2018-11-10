
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
inline 
auto beeper = []( auto fp) {
	typedef BOOL(*BeepFP) (DWORD, DWORD);

	BeepFP beepFunction = (BeepFP)fp;
	_ASSERTE(beepFunction);

	DWORD frequency = 1000; // max is 32765;
	DWORD duration = 1000;
	const UINT step = 50;

	do {
		dbj::nano::log( L"\nbeepFunction( Frequency:" , frequency , ", Duration:"
			, duration , L" )" );

		if (0 == beepFunction(frequency, duration)) {
			dbj::nano::log( L" FAILED ?" );
			break;
		}
		frequency -= step;
	} while (frequency > step);
};

	inline void test_dbjLibload(
		const wchar_t * dll_ = L"kernel32.dll",
		const wchar_t * fun_ = L"Beep")
	{
		/* second argument if true means 'system dll is required' */
		auto dll = dbj::win::libload(dll_, true);
		/* note to win32 scouts: fp type bellow is FARPROC */
		auto  fp = dll.getFunction(fun_);
		if (!fp) {
			throw 
			dbj::nano::terror(
			  dbj::nano::prefix(
				fun_ , L" -- Function not found?"
			));
		}
		beeper(fp);
		// free-ing dll lib loaded 
		// happens here
				
	} // test_dbjLibload
} 

int wmain(int argc, const wchar_t * argv[], wchar_t * envp) {

	using namespace dbj::nano; 
	log("\n", appname( argv[0] ) , "\n" );
	int exit_code = EXIT_SUCCESS;

	try {
		test_dbjLibload();
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