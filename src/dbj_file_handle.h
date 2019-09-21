/* (c) 2019 by dbj.org   -- CC BY-SA 4.0 -- https://creativecommons.org/licenses/by-sa/4.0/ */
#pragma once
#ifndef _DBJ_FILE_HANDLE_INC_
#define _DBJ_FILE_HANDLE_INC_

#include <io.h>
#include <fcntl.h>
#include <string>
#include <optional>

#include "../../dbj--nanolib/dbj++status.h"
#include "../../dbj--nanolib/dbj++tu.h"

#ifdef TESTING_DBJ_RETVALS
namespace tempo_test {

	inline void now() {
		using posix = dbj::nanolib::posix_retval_service;
		DBJ_TX(DBJ_STATUS(posix, std::errc::already_connected));
		DBJ_TX(DBJ_STATUS(posix, "Wowza!"));

		DBJ_TX(DBJ_RETVAL_ERR(posix, std::nullptr_t, std::errc::already_connected));
		DBJ_TX(DBJ_RETVAL_OK(4 + 2 * 3));
		DBJ_TX(DBJ_RETVAL_FULL(posix, std::string("string"), "OK"));
	}
}
#endif // TESTING_DBJ_RETVALS

namespace dbj {

	using namespace std;

	// File Handle(r)
	class FH final
	{
		std::string name_;
		int file_descriptor_ = -1;

		FH(const char* filename, int descriptor_)
			: name_(filename), file_descriptor_(descriptor_)
		{
			/* only make can reach here */
		}

	public:
		/*
		reference_wraper<FH> makes possible two things

		1. using yet not fully defined class
		2. using type which is not copyable and/or moveable
		*/
		using return_type = typename dbj::nanolib::return_type< reference_wrapper<FH> >;
		/*
		return type is { { FH }, { json string } }
		*/
		[[nodiscard]] static decltype(auto) make(const char* filename)
		{
			using posix = dbj::nanolib::posix_retval_service;

			const char* fn = filename;

			_ASSERTE(fn);
			//			fn = "/dev/null";
			int fd = -1;
			errno_t rez = _sopen_s(&fd, fn, O_WRONLY | O_APPEND | O_CREAT, _SH_DENYNO, _S_IREAD | _S_IWRITE);

			if (rez != 0) {
				/*
errno value	Condition
EACCES	The given path is a directory, or the file is read-only, but an open-for-writing operation was attempted.
EEXIST	_O_CREAT and _O_EXCL flags were specified, but filename already exists.
EINVAL	Invalid oflag, shflag, or pmode argument, or pfh or filename was a null pointer.
EMFILE	No more file descriptors available.
ENOENT	File or path not found.
				*/
				return DBJ_RETVAL_ERR(posix, FH, std::errc(rez));
			}

			struct stat sb;
			fstat(fd, &sb);

			switch (sb.st_mode & S_IFMT) {
			case S_IFCHR:  //printf("character device\n");        break;
#ifndef _MSC_VER
			case S_IFIFO:  //printf("FIFO/pipe\n");               break;
			case S_IFLNK:  //printf("symlink\n");                 break;
#endif // !_MSC_VER
			case S_IFREG:  //printf("regular file\n");            break;
				break;
			default:
					return DBJ_RETVAL_ERR(posix, FH, std::errc::no_such_device );
				break;
			}

			return DBJ_RETVAL_OK(FH(filename, fd));

		};

		[[nodiscard]] static decltype(auto) make(std::string_view filename)
		{
			// 1.copy FH instance
			// 2.move FH instance
			return dbj::nanolib::return_type<FH>(make(filename.data()));
		}

		const char* name() const noexcept { return name_.c_str(); };

		FILE* file_ptr(const char* options_ = "w") const noexcept
		{
			_ASSERTE(file_descriptor_ > -1);
			static FILE* file_ = _fdopen(file_descriptor_, options_);
			_ASSERTE(file_ != nullptr);
			_ASSERTE(0 == ferror( file_));
			// fprintf(file, "TRA LA LA LA LA!");
			return file_;
		}
	}; // FH
} // dbj
#endif // !_DBJ_FILE_HANDLE_INC_

