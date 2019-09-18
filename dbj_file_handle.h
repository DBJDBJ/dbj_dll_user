#pragma once
#ifndef _DBJ_FILE_HANDLE_INC_
#define _DBJ_FILE_HANDLE_INC_

#include <io.h>
#include <fcntl.h>
#include <string>

namespace dbj {
	
	// File Handle(r)
	class FH final 
	{
		std::string name_;
		int file_descriptor_ = -1;

	public:
		FH(const char* filename) {
			const char* fn = filename;

			_ASSERTE(fn);
			//			fn = "/dev/null";
			int fd = _open(fn, O_WRONLY | O_APPEND | O_CREAT, _S_IREAD | _S_IWRITE);

			if (fd < 1) {
				fprintf(stderr, "\n\nFatal error, error message: ");
				perror(fn);
				std::exit(1);
			}

			struct stat sb;
			fstat(fd, &sb);
			char err[255] = { 0 };

			switch (sb.st_mode & S_IFMT) {
			case S_IFCHR:  //printf("character device\n");        break;
#ifndef _MSC_VER
			case S_IFIFO:  //printf("FIFO/pipe\n");               break;
			case S_IFLNK:  //printf("symlink\n");                 break;
#endif // !_MSC_VER
			case S_IFREG:  //printf("regular file\n");            break;
				break;
			default:
				sprintf(err, "%s invalid type", fn);
				perror(err);
				exit(2);
				break;
			}
			name_ = std::string(fn);
			this->file_descriptor_ = fd;

		};

		const char* name() const noexcept { return name_.c_str(); };

		FILE* file_ptr(const char* options_ = "w") const noexcept
		{
			_ASSERTE(file_descriptor_ > -1);
			static FILE* file_ = _fdopen(file_descriptor_, options_);
			_ASSERTE(file_ != nullptr);
			// fprintf(file, "TRA LA LA LA LA!");
			return file_;
		}
	}; // FH
} // dbj
#endif // !_DBJ_FILE_HANDLE_INC_

