# dbj--simplelog
### simple resilient fast local log

By default all the log goes to 
the `stderr`. Local file is not used and there is no resilience in presence of threads.

On startup one can use the setup function.

```cpp
int main(int argc, const char* argv[], char* envp)
{
using dbj::simplelog::SETUP;
if (!dbj::simplelog::setup(
	SETUP::LOG_FROM_APP_PATH | SETUP::VT100_CON | SETUP::FILE_LINE_OFF ,
	argv[0])
)
 return EXIT_FAILURE;

log_trace(" Starting Application: %s", argv[0]);
log_trace(" Local log file: %s", dbj::simplelog::current_log_file_path());

```
Setup options

| Setup tag  | the effect  |
|---|---|
 MT | set the Multi Threaded protection
VT100_CON  | switch on the VT100 console mode, if no coloured output 
LOG_FROM_APP_PATH  | If app full path is given  use it to obtain log gile name. Make sure you use absoulte paths.
FILE_LINE_OFF | Exclude file and line, from time stamp 
SILENT | No console output. Beware, if this is set and no file path is given you will have a completely quiet logging. 


---

Tested on Windows 10. Using C++17 and C. 

Depends on [dbj--nanolib](https://github.com/dbj-systems/dbj--nanolib)

How to use: add as a git submodule. Include it's only C file in your project. Use.

-------

(c) 2019 by dbj.org   -- CC BY-SA 4.0 -- https://creativecommons.org/licenses/by-sa/4.0/ 

Based on `logc` lib by `rxi`. See the sub folder `logc`.

