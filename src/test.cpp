#include "../stdafx.h"
/*
*/
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
		std::wcout << L"beepFunction( Frequency:" << frequency << ", Duration:"
			<< duration << L" )" << std::endl;

		if (0 == beepFunction(frequency, duration)) {
			std::wcerr << L" FAILED ?" << std::endl;
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
		dbj::win::Libload dll(dll_, true);

		/* note to win32 scouts: fp type bellow is FARPROC */
		auto  fp = dll.getFunction(fun_);

		if (!fp) {
			throw 
			dbj::nano::exception(
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

	std::wclog << std::endl << argv[0] << std::endl;
	int exit_code = EXIT_SUCCESS;

	try {
		test_dbjLibload();
	}
	catch ( std::exception rex )
	{
		std::clog << L"\nRun time exception: " << rex.what() << std::endl;
		exit_code = EXIT_FAILURE;
	}
	catch (...)
	{
		std::clog << L"\nUnknown exception: " << std::endl;
		exit_code = EXIT_FAILURE;
	}
	return exit_code;
}