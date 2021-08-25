# `dbj::win::dll_load`

(c) 2018-2020 by dbj@dbj.org

## WIN32 DLL dynamic loader

### 2.7.0

- `dll_call`

From version `2.6.0` there is even simpler usage idiom through the `dll_call` function:
```cpp
// the Beep function signature
typedef BOOL(*BeepFP) (DWORD, DWORD);

int frequency_ = 1000 ; // Hz
int duration_  = 1000 ; // 1 sec
// just beep once
	auto  rezult = dbj::win::dll_call<BeepFP>(
		"kernel32.dll", 
		"Beep",
		[&](BeepFP beeper) {
			return beeper(frequency_, duration_ );
	}
);
```
Notice how the return value of the dll function is returned.

### Installation

Download and include `dbj_dll_call.h` in your include path. NOTE: `windows.h` is not included by default. Define `DBJ_DLL_CALL_INCLUDES_WINDOWS` to have windows included.

#### Compatibility

There is no obvious reason for which this header should not work with any C++ version you might use. Please [do let us know](mailto:info@dbj.systems) if it does not. 

### Usage

You need three things:

1. the dll name 
2. the function name
3. the function signature

The usage:

```cpp
#include <dbj_dll_call.h>
{
// the Beep function signature
typedef BOOL(*BeepFP) (DWORD, DWORD);

auto loader_ =
 ::dbj::win::dll_load("kernel32.dll", is_system_dll);
BeepFP beeper_ = 
  loader_.get_function<BeepFP>("Beep");
// if any, failures are already logged
// just use it
  if (beeper_)
      beeper_(1000,1000);
// Caution: when loader is going out of scope the dll is unloaded
// after that  calling beeper_ will crash the app
}
```
This is one very comfortable way of calling WIN32 API. Without huge legacy headers, macros, globals, pragmas and the rest.

### Caveat Emptor

As mentioned in the comment above, the dll used, will be properly unloaded when it's holder goes out of scope. In the `test.cpp` please observe the  
deliberate bad example regarding the validity of the function pointer for which the parent dll has been unloaded.

Basically user defined aka non system dll requires due dilligence. Recommended course of action is to use `dbj::win::dll_call`.

### Testing is mandatory

For the test please look into `test.cpp`. That is not extremely trivial test. Please spend some time in that code.

### Logging

Error reporting is usualy done with file logging.
By default `dbj::win::dll_load` logs to `actual_log_function` through this macro:
```cpp
#define DBJCS_LOADER_LOG(...) \
actual_log_function \
(__FILE__, __LINE__, __VA_ARGS__)
```
One is provided, but to replace the `actual_log_function` define `DBJ_DLL_USER_PROVIDED_LOG_FUN` and provide your implementation with the same signature. Before including `dbj_dll_call.h`.

`test.cpp` also contains an `stderr` redirector.  The log file `dbj_lib_load.exe.log` is together with the executable.

### Enjoy

Contact: [info@dbj.systems](mailto:info@dbj.systems), and the mandatory GitHub "Issues" tab.

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
