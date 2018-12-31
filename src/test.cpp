
#include "dbj_dll_call.h"

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

inline auto beeper (BOOL(*beepFunction) (DWORD, DWORD) ) 
{
	_ASSERTE(beepFunction);

	DWORD frequency = 2400, duration = 100 ; // max is 32765;
	const UINT f_step = 50, d_step = 50 ;

	do {
		::wprintf(L"\t{F = %6d , D = %6d}", frequency, duration);

		if (0 == beepFunction(frequency, duration)) {
			dbj::nano::log(__FUNCSIG__ "beepFunction() FAILED ?");
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
			::dbj::nano::log("\nCalling the function: ",	fun_.data(), ",\nwith signature ", typeid(fp).name(), "\nfrom a dll: ", dll_.data());
		test_fun(fp);
	} else {
		throw dbj::nano::terror(
			dbj::nano::prefix(fun_ , " -- Function not found?"sv )
		);
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
		// if dll is not found exception is thrown
		auto  fp = dbj::win::dll_dyna_load<GGIEX>(
			"kernel32.dll", "GetGeoInfoEx"
			);

		// if dll is found but function inside
		// it is not found, fp is nullptr
		if (nullptr == fp) return;

		// else use it
	}
	catch (dbj::exception & x) {
		dbj::nano::log
		("\n\nException while looking for GetGeoInfoEx in kernel32.dll\n\n"
		, x.what() );
	}
}

/*
On VISTA or above there is no reason not to use wmain()
*/
int wmain(int argc, const wchar_t * argv[], wchar_t * envp) 
{
	using namespace ::std;
	using namespace ::std::string_literals;
	using namespace ::std::string_view_literals;

	using ::dbj::nano::log ; 
	log("\n", argv[0] , "\n" );
	int exit_code = EXIT_SUCCESS;

	try {
		// test the direct use
		try_get_geo_infoex();
		// or use a fancy test unit
		testing_testing_123::test_dbj_dll_call
		 ("kernel32.dll"sv, "Beep"sv, testing_testing_123::beeper );
	}
	catch ( const dbj::exception & rex )
	{
		log( "\ndbj exception:\n\t", rex.what() );
		exit_code = EXIT_FAILURE;
	}
	catch ( const std::exception & rex )
	{
		log( "\nstd exception:\n\t", rex.what() );
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