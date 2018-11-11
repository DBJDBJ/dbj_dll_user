# dbjdll

## C++17 WIN32 DLL dynamic loader

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
dll will be properly unloaded when it's holder goes out of scope.
For the fancy test unit please look into test.cpp .
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
