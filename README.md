# dbjdll

## WIN32 DLL dynamic loader

### C++17

```cpp
       // beep signature
    	typedef BOOL(*BeepFP) (DWORD, DWORD);

inline auto test_dbjLibload 
 = [&] ( auto dll_ , auto fun_  ) -> void
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
  fp(1000,1000) ;
} // test_dbjLibload
// calling with wstring_view literals_
test_dbjLibload(L"kernel32.dll"sv, L"Beep"sv );
```
<hr/>
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
