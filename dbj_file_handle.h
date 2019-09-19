#pragma once
#ifndef _DBJ_FILE_HANDLE_INC_
#define _DBJ_FILE_HANDLE_INC_

#include <io.h>
#include <fcntl.h>
#include <string>
#include <optional>

namespace dbj {

	using namespace std;
	
	// File Handle(r)
	class FH;
	class FH final 
	{
	public:
		enum class error { OPEN_PROBLEM, UNKNOWN_DEVICE };
		using return_type = std::pair< optional<FH>, optional<error> >;

		static char const * err_message( optional<error> & rt ) noexcept
		{
			if (!rt) return "Status is empty";

			if (*rt == error::OPEN_PROBLEM) return "Error while opening the file";
			if (*rt == error::UNKNOWN_DEVICE) return "Unknown device error";
			return "Unknown status";
		}
	private:
		std::string name_;
		int file_descriptor_ = -1;

		FH(const char* filename, int descriptor_)
			: name_(filename), file_descriptor_(descriptor_)
		{
		}
		// static return_type make(const char* filename);
	public:
		
		static return_type make(std::string_view filename)
		{
			return make(filename.data());
		}

		static return_type make( const char* filename) 
		{
			const char* fn = filename;

			_ASSERTE(fn);
			//			fn = "/dev/null";
			int fd = _open(fn, O_WRONLY | O_APPEND | O_CREAT, _S_IREAD | _S_IWRITE);

			if (fd < 1) {
				return { {}, { error::OPEN_PROBLEM } };
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
				return { {}, { error::UNKNOWN_DEVICE } };
				break;
			}

			return { FH( filename , fd ), {} };

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

