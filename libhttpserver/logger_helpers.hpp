#include <fmt/core.h>
#include <iostream>

class LoggerHelper{
	public:
		virtual void log(const std::string&, const std::string&) = 0;
};

// TODO(harsath): complete....
class ErrorContext : public LoggerHelper{
	private:
		std::string _err_file_name;
	public:
		virtual void log(const std::string& fmt_str, const std::string& file_name) override final {

		}
};

class AccessContext : public LoggerHelper{
	private:
		std::string _acc_file_name;
	public:
		virtual void log(const std::string& fmt_str, const std::string& file_name) override final {
			
		}
};
