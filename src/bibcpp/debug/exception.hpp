#pragma once

#include <stdexcept>
#include <sstream>

namespace bib {
namespace err {

class F {
// from http://stackoverflow.com/a/12262626
public:
	F() {
	}
	~F() {
	}

	template<typename Type>
	F & operator <<(const Type & value) {
		stream_ << value;
		return *this;
	}

	std::string str() const {
		return stream_.str();
	}
	operator std::string() const {
		return stream_.str();
	}

	enum ConvertToString {
		to_str
	};
	std::string operator >>(ConvertToString) {
		return stream_.str();
	}

private:
	std::stringstream stream_;

	F(const F &);
	F & operator =(F &);
};

class Exception: public std::exception {
private:
	std::string what_;
public:
	Exception(std::string ss) :
			what_(ss) {
	}
	~Exception() throw () {
	}
	virtual const char* what() const throw () {
		return what_.c_str();
	}
};
} // namespace err
} // namespace bib
