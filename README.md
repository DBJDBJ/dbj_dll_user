# dbjdll

## WIN32 DLL dynamic loader

### C++17

```cpp
  
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
      // beeping
    	typedef BOOL(*BeepFP) (DWORD, DWORD);
     	BeepFP beepFunction = (BeepFP)fp;
      beepFunction(1000,1000) ;
		// free-ing dll lib loaded 
		// happens here
				
	} // test_dbjLibload
```

Copyright 2018 by dbj@dbj.org

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http ://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
