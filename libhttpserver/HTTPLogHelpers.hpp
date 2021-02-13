#pragma once
#include <cstdio>
#include <fmt/format.h>
#include <fstream>
#include <functional>
#include <iostream>
#include <memory>

namespace HTTP::LOG{

	struct LogMessage{
		std::string client_ip, date, resource, useragent, log_message;
		~LogMessage() = default;
	};

	class LoggerHelper{
		public:
			virtual void log(const std::string&) const noexcept = 0;
			virtual ~LoggerHelper() = default;
	};

	class ErrorContext final : public LoggerHelper{
		private:
			std::string _err_file_name;
			std::FILE* _error_stream;
		public:
			explicit ErrorContext(const std::string& file_name);
			void log(const std::string& fmt_str) const noexcept override;
			~ErrorContext();
	};

	class AccessContext final : public LoggerHelper{
		private:
			std::string _acc_file_name;
			FILE* _access_stream;
		public:
			explicit AccessContext(const std::string& file_name);
			void log(const std::string& fmt_str) const noexcept override;
			~AccessContext();
	};

	inline void ErrorContext::log(const std::string &fmt_str) const noexcept {
		std::fprintf(_error_stream, "[Error] %s\n", fmt_str.c_str());
		fflush(_error_stream);
	}

	inline void AccessContext::log(const std::string &fmt_str) const noexcept {
		std::fprintf(_access_stream, "[Access] %s\n", fmt_str.c_str());
		fflush(_access_stream);
	}

	inline ErrorContext::ErrorContext(const std::string& file_name){
		_error_stream = std::fopen(file_name.c_str(), "a");
	}

	inline ErrorContext::~ErrorContext(){
		std::fclose(_error_stream);
	}

	inline AccessContext::AccessContext(const std::string& file_name){
		_access_stream = std::fopen(file_name.c_str(), "a");
	}

	inline AccessContext::~AccessContext(){
		std::fclose(_access_stream);
	}

	class LoggerFactory{
		public:
		enum Log : std::uint8_t{
			Error, Access
		};
		static std::unique_ptr<LoggerHelper> MakeLog(const std::string& file_name, Log log_type){
			switch(log_type){
				case Log::Error:
					return std::make_unique<ErrorContext>(file_name);
				case Log::Access:
					return std::make_unique<AccessContext>(file_name);
				default:
					throw "Invalid Log context";
			}
		}
	};

} // end namespace HTTP::LOG
