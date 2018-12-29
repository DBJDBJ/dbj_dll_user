
#include "dbj_dll_call.h"

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

	using namespace ::std;
	using namespace ::std::string_literals;
	using namespace ::std::string_view_literals;

	template<typename F>
	inline void test_dbj_dll_call 
		(	wstring dll_ , wstring fun_ , F test_fun )
	{
		// note: bellow is a C++17 if() syntax
			if (
				// load the dll + get the function
				// third argument if true means 'system dll is required' 
				BeepFP  fp = dbj::win::dll_call<BeepFP>(dll_, fun_, true);
				fp 
			) {
				::wprintf(L"\nTesting function %s '%S', from '%s'\n",
					fun_.data(), typeid(fp).name(), dll_.data());
				test_fun(fp);
			} else {
				throw dbj::nano::terror(fun_ + L" -- Function not found?");
			}
	}; // test_dbj_dll_call
} 

/*
		return ::GetGeoInfoEx(
				(PWSTR)location,
				(GEOTYPE)query,
				(PWSTR)geoData,
				(int)geoDataCount
			);
			exist in kernel32.dll but only above W10 RESTONE 3
*/
static void try_get_geo_infoex()
{
	using GGIEX = 
		int (*)( PWSTR ,GEOTYPE, PWSTR ,int);
	// direct use
	try {
		// if dll is found but function inside
		// it is not found, fp is nullptr
		GGIEX  fp = dbj::win::dll_call<GGIEX>(L"kernel32.dll", L"GetGeoInfoEx", true);

		if (nullptr == fp) return;

		// else use it
	}
	catch (dbj::exception & x) {
		dbj::nano::log("\n\nException while looking for GetGeoInfoEx in kernel32.dll\n\n"
		, x.what() );
	}
}

int wmain(int argc, const wchar_t * argv[], wchar_t * envp) {

	using namespace dbj::nano; 
	dbj::nano::log("\n", argv[0] , "\n" );
	int exit_code = EXIT_SUCCESS;

	try {
		try_get_geo_infoex();
		// direct use
		BeepFP  fp = dbj::win::dll_call<BeepFP>(L"kernel32.dll", L"Beep", true);
		fp(1000, 1000);
		// or
		// using a fancy test unit
		test_dbj_dll_call(L"kernel32.dll"s, L"Beep"s, beeper );
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