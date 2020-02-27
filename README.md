# `dbj::win::dll_load`

V 1.0.0  (c) 2018-2020 by dbj@dbj.org

## WIN32 DLL dynamic loader

### Installation

Download and include `dbj_dll_call.h` in your include path. NOTE: `windows.h` is not included by default. Define `DBJ_DLL_CALL_INCLUDES_WINDOWS` to have windows included.

#### Compatibility

There is no obvious reason for which this header shoild not work with any C++ version you might use. 

In case of problem please do mail me.

### Usage

Basically there is one funcion that does it all. 
You need three things:

1. the dll name 
2. the function name
3. the function signature

The usage:

```cpp
#include <dbj_dll_call.h>

// the Beep function signature
typedef BOOL(*BeepFP) (DWORD, DWORD);

// load the dll + get the function
// not using the class directly
BeepFP  fp = dbj::win::dll_call<BeepFP>(
  "kernel32.dll", 
  "Beep", 
// third argument if true means 'system dll is required' 
  true);

// just use it
fp(1000,1000);
```

The dll used, will be properly unloaded when it's holder goes out of scope.

### Testing

For the test please look into `test.cpp`.

### Logging

This header is used in windows app. Thus error reporting is usualy done with file logging.
By default `dbj::win::dll_load` logs to `actual_log_function` through this macro:

```cpp
#define DBJ_DLL_CALL_LOG(...) actual_log_function (__FILE__, __LINE__, __VA_ARGS__)
```

To replace the `actual_log_function` define `DBJ_DLL_USER_PROVIDED_LOG_FUN` and provide your implementation with the same signature. Before including `dbj_dll_call.h`.

Enjoy

--------------------------------------------

Copyright 2018/2019/2020 by dbj@dbj.org

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http ://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
