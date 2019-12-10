# `dbj::win::dll_load`

### C++17 WIN32 DLL dynamic loader

### Usage

Basically there is one funcion that does it all. For it to be used you need 
three things

1. the dll name 
2. the function name
3. the function signature

```cpp
// the Beep function signature
typedef BOOL(*BeepFP) (DWORD, DWORD);

// load the dll + get the function
BeepFP  fp = dbj::win::funload<BeepFP>(
  L"kernel32.dll", 
  L"Beep", 
// third argument if true means 'system dll is required' 
  true);

// just use it
fp(1000,1000);
```

This is used in windows app, mostly. Thus error reporting is done with file logging.
By default `dbj::win::dll_load` logs to `stderr`
```cpp
#ifndef DBJ_DLL_CALL_LOG
#define DBJ_DLL_CALL_LOG(...) (void)::fprintf_s(stderr, __VA_ARGS__ )
#endif DBJ_DLL_CALL_LOG
```
In Windows app's that goes to "nowhere". 
Top of the test.cpp, shows how users can use thier own logging instead of
`DBJ_DLL_CALL_LOG`.
```cpp
// include our own logger
#include "..\dbj--simplelog\log.h"
// replace dbj dll call logger, with user provided simple file log
#define DBJ_DLL_CALL_LOG(...) log_error( __VA_ARGS__ )
```

dll will be properly unloaded when it's holder goes out of scope.
For the fancy test unit please look into `test.cpp`.

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
