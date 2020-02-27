/* (c) 2019 by dbj.org   -- CC BY-SA 4.0 -- https://creativecommons.org/licenses/by-sa/4.0/ */
#pragma once
#ifndef _DBJ_FILE_HANDLE_INC_
#define _DBJ_FILE_HANDLE_INC_

#ifdef __cplusplus

#include <string>
#include <io.h>
#include <fcntl.h>

namespace dbj {
	namespace simplelog {

		using namespace std;

		// File Handle(r)
		class FH final
		{
			string name_;
			int file_descriptor_ = -1;

			/*
			assure file descriptor given file name
on error returns one of errno values

Condition -- Message
EACCES	The given path is a directory, or the file is read-only, but an open-for-writing operation was attempted.
EEXIST	_O_CREAT and _O_EXCL flags were specified, but filename already exists.
EINVAL	Invalid oflag, shflag, or pmode argument, or pfh or filename was a null pointer.
EMFILE	No more file descriptors available.
ENOENT	File or path not found.
ENODEV	No such device
*/
			[[nodiscard]] static errno_t  assure(const char* filename, int & fd)
			{
				const char* fn = filename;

				_ASSERTE(fn);
				errno_t rez = _sopen_s( &fd, fn, O_WRONLY | O_APPEND | O_CREAT, _SH_DENYNO, _S_IREAD | _S_IWRITE);

				if (rez != 0) {
					return rez;
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
					return ENODEV;
					break;
				}
					return errno_t(0);
			};

		public:
			FH( errno_t & err_ , const char* filename) noexcept
				: name_(filename) 
			{
				err_ = assure(name_.c_str(), file_descriptor_);
			}

			const char* name() const noexcept { return name_.c_str(); };

			FILE* file_ptr(const char* options_ = "w") const noexcept
			{
				_ASSERTE( this->file_descriptor_ > -1);
				static FILE* file_ = _fdopen( this->file_descriptor_, options_);
				_ASSERTE(file_ != nullptr);
				_ASSERTE(0 == ferror(file_));
				return file_;
			}
		}; // FH
	} // simplelog
} // dbj

#endif // __cplusplus

#endif // !_DBJ_FILE_HANDLE_INC_

