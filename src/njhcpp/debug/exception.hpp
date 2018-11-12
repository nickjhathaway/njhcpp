#pragma once

#include <stdexcept>
#include <sstream>

namespace njh {
/**@brief A namespace to add error message and other debugging handling
 *
 */
namespace err {

/**@brief class to inline using the << operator to print a quick message
 *
 */
class F {
// from http://stackoverflow.com/a/12262626
public:
	F() {
	}
	~F() {
	}
	/**@brief Templated << so anything with a << operator operator can be called
	 *
	 * @param value The value to add to the message
	 * @return Return a ref to the class so this can be chained
	 */
	template<typename Type>
	F & operator <<(const Type & value) {
		stream_ << value;
		return *this;
	}
	/**@brief print the message
	 *
	 * @return A string of the message
	 */
	std::string str() const {
		return stream_.str();
	}
	/**@brief convert to std::string
	 *
	 */
	operator std::string() const {
		return stream_.str();
	}

	enum ConvertToString {
		to_str
	};
	/**@b output a std::string of message
	 *
	 * @param
	 * @return A string of the message
	 */
	std::string operator >>(ConvertToString) {
		return stream_.str();
	}

private:
	std::stringstream stream_; /**< stream to hold message content */

	F(const F &);
	F & operator =(F &);
};

/**@brief Simple exception class, construct with std::string
 *
 */
class Exception: public std::exception {
private:
	std::string what_; /**< @brief Exception message  */
public:
	/**@brief Construct with std::string of what except message is
	 *
	 * @param ss The std::string of the message
	 */
	Exception(std::string ss) :
			what_(ss) {
	}
	~Exception() throw () {
	}
	/**@brief return c style string of exception message
	 *
	 * @return A c style string of message
	 */
	virtual const char* what() const throw () {
		return what_.c_str();
	}
};


/**@brief Generate the beginning of the warning message by stating filename, file line, and function name
 *
 * @param file The file name, generally created by __FILE__
 * @param line The line number, generally created by __LINE__
 * @param funcName The function name, generally created by __PRETTY_FUNCTION__
 * @return A slightly formated string with with file, line and funcName concatenated together
 */
inline std::string genWarningStr(const std::string & file, const std::string & line, const std::string & funcName){
	return file + ":" + line + " " + funcName;
}


} // namespace err
} // namespace njh
