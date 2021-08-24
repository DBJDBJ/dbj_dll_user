#pragma once
/* (c) 2019 - 2021 by dbj.org   -- https://dbj.org/license_dbj
		
dynamic dll loading and fetching a function from the said dll

#define DBJCAPI_DLL_CALLER_IMPLEMENTATION  in exactly one place 
*/

#include "../dbj_capi/ccommon.h"

// ----------------------------------------------------------------
DBJ_EXTERN_C_BEGIN

typedef enum dbjcapi_dll_call_semver
{
	major = 1,
	minor = 0,
	patch = 0
} dbjcapi_dll_call_semver;

/// --------------------------------------------------------------
#ifdef DBJCAPI_DLL_CALLER_IMPLEMENTATION
/// --------------------------------------------------------------

#include "../dbj_capi/cdebug.h"
#include "../dbj_capi/default_log.h"

#include <stdarg.h>
#include <stdio.h>
/// --------------------------------------------------------------
/// user can provide the actual log function
/// the required signature is
///
/// extern "C" void (*user_log_FP) (const char* file, long line, const char* , ...);
#ifndef DBJ_DLL_CALL_LOG
#define DBJ_DLL_CALL_LOG(...) dbj_default_log_function(__FILE__, __LINE__, __VA_ARGS__)
#endif // DBJ_DLL_CALL_LOG

#undef DBJCAPI_DLL_LOAD_FILE_LEN
#define DBJCAPI_DLL_LOAD_FILE_LEN 1024

typedef struct dbjcapi_dll_call_state
{
	// critical section will be in here
	HINSTANCE dll_handle_;
	char dll_name_[DBJCAPI_DLL_LOAD_FILE_LEN];
} dbjcapi_dll_call_state;

static inline dbjcapi_dll_call_state *dbjcapi_dll_call_state_(void)
{
	static dbjcapi_dll_call_state instance = {0, {0}};
	return &instance;
}

static inline bool dbjcapi_dll_loaded(void)
{
	dbjcapi_dll_call_state *state = dbjcapi_dll_call_state_();
	// if this is NOT NULL we know DLL by this name is loaded
	// until it is not dbjcapi_dll_unloaded no other DLL can be used and its function called
	return state->dll_handle_ != 0;
}

static inline int dbjcapi_assign_dll_name(char const name_[static 1])
{
	dbjcapi_dll_call_state *state = dbjcapi_dll_call_state_();

	// major logic check here
#ifdef _DEBUG
	if (dbjcapi_dll_loaded())
	{
		if (state->dll_name_[0] != 0)
			DBG_PRINT("DLL %s, is already loaded", state->dll_name_);
		else
		{
			DBG_PRINT("DLL name must exist if DBJCAPI DLL LOADER state is LOADED?");
			DBJ_FAST_FAIL;
		}
	}
#endif // _DEBUG

	return strncpy_s(state->dll_name_, DBJCAPI_DLL_LOAD_FILE_LEN, name_, strlen(name_));
}

/*
We manage just a single DLL load --> call -->dbjcapi_dll_unload
*/
static inline void dbjcapi_dll_load(
	const char dll_file_name_[static 1])
{
	// will do fast fail on logic error
	// ie if dll is already loaded
	dbjcapi_assign_dll_name(dll_file_name_);

	dbjcapi_dll_call_state *state = dbjcapi_dll_call_state_();

	state->dll_handle_ = LoadLibraryA(state->dll_name_);

	if (NULL == state->dll_handle_)
	{
		DBJ_DLL_CALL_LOG(" Could not find the DLL by name: %s", state->dll_name_);
		// reset the file name
		state->dll_name_[0] = '\0';
	}
}
/*
FreeLibrary() failure is very rare and might signal
some deep error with the machines or OS
thus we will not ignore it.
*/
static inline void dbjcapi_dll_unload()
{
	if (dbjcapi_dll_loaded())
	{
		dbjcapi_dll_call_state *state = dbjcapi_dll_call_state_();
		if (!FreeLibrary(state->dll_handle_))
		{
			DBJ_DLL_CALL_LOG(
				"\ndbjdll_loadFreeLibrary failed. The DLL name is: %s\n", state->dll_name_);
			DBJ_FAST_FAIL;
		}
		// leave it in unloaded state
		state->dll_handle_ = 0;
		state->dll_name_[0] = '\0';
	}
}

// we will use the destructor to free the dll if any is left in memory
__attribute__((destructor)) static inline void dbjcapi_dll_loader_destructor(void) { dbjcapi_dll_unload(); }

/*
RFP = Required Function FP of the function fromm the DLL

bellow returns null or function pointer to the one requested
void * "saves the day here" thus the function is generic
the user knows the RFP and will cast to it
*/
static inline void *dbjcapi_dll_get_function(char const fun_name_[static 1])
{
	dbjcapi_dll_call_state *state = dbjcapi_dll_call_state_();
	if (!dbjcapi_dll_loaded())
	{
		DBJ_FAST_FAIL;
		return 0;
	}

	// Funny fact: GetProcAddress has no unicode equivalent
	FARPROC result =
		GetProcAddress(
			// handle to DLL module
			(HMODULE)state->dll_handle_,
			// name of a function
			fun_name_);
	if (result == 0)
	{
		DBJ_DLL_CALL_LOG(
			"\nFailed to find the address for a function: %s\nThe DLL name is: %s", fun_name_, state->dll_name_);
		DBJ_FAST_FAIL;
	}
	return result;
}

#endif // DBJCAPI_DLL_CALLER_IMPLEMENTATION
/*
RFP = Required Function FP of the function fromm the DLL

The do it all function,
call the callback provided with the 
pointer of the fetched function.
if dll load has failed the callback 
will not be called.
RFP = Actual Function Type
CBF = Call Back Function
	void ( * callback ) ( RFP )

Example: from dbj.dll exported is a function int get42(void) ; let's call it

int (*FP42) (void) ;
void cb ( FP42 get42_from_dll ) { assert( 42 == get42_from_dll() );  }

DBJCAPI_DLL_CALL( "dbj.dll", "get42", FP42 ) ;

	// if any, failures are already logged

*/
#define DBJCAPI_DLL_CALL(dll_name_, fun_name_, RFP, callback_)       \
	do                                                               \
	{                                                                \
		dbjcapi_dll_load(dll_name_);                                 \
		RFP *function_ = (RFP *)dbjcapi_dll_get_function(fun_name_); \
		if (function_)                                               \
			callback_(*function_);                                   \
		dbjcapi_dll_unload();                                        \
	} while (0)

// ----------------------------------------------------------------------------
DBJ_EXTERN_C_END
