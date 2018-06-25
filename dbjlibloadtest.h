#pragma once
/*
*/
#include "dbjlibload.h"
namespace dbj {
	namespace win {
		namespace test {
			/*
			Please remeber -- std::exception is thrown on any error
			*/
			DBJINLINE void test_dbjLibload(void * arg_ = 0)
			{
				/* second argument if true means 'system dll is required' */
				dbj::win::Libload dll(L"kernel32.dll", true);

				/* note to win32 dwellers: fp type bellow is FARPROC */
				auto  fp = dll.getFunction(L"Beep");

				if (!fp) {
					std::wcerr << L"Beep() -- Function not found?" << std::endl; return;
				}
				/*
				BOOL Beep( DWORD dwFreq,  // sound frequency, in hertz
							DWORD dwDuration  // sound duration, in milliseconds
					);

				Parameters dwFreq, specifies the frequency, in hertz, of the sound.
				This parameter must be in the range 37 through 32,767 (0x25 through 0x7FFF).
				*/
				typedef BOOL(*BeepFP) (DWORD, DWORD);

				BeepFP beepFunction = (BeepFP)fp;
				assert(beepFunction);

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

			} // test_dbjLibload
		} // test
	} // win 
} // dbj