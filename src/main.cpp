#include <iostream>
#include "bibcpp.h"

using namespace bib;

int main(int argc, char* argv[]){
  std::cout << bashCT::bold << "Hello bibcpp!" << bashCT::reset << std::endl;
	uint16_t ui16t = 16;
	uint32_t ui32t = 32;
	uint64_t ui64t = 64;
	size_t st = 64;
	int16_t i16t = 16;
	int32_t i32t = 32;
	int64_t i64t = 54;
	std::string str = "test";
	bool off = false;
	double dt = 10.01;
	long double ldt = 100.01;
	float ft = 1.01;
  bib::progutils::programSetUp setUp(argc, argv);
  setUp.setOption(ui16t, "--ui16t", "ui16t");
  setUp.setOption(ui32t, "--ui32t", "ui32t");
  setUp.setOption(ui64t, "--ui64t", "ui64t");
  setUp.setOption(st, "--st", "st");
  setUp.setOption(i16t, "--i16t", "i16t");
  setUp.setOption(i32t, "--i32t", "i32t");
  setUp.setOption(i64t, "--i64t", "i64t");
  setUp.setOption(str, "--str", "str");
  setUp.setOption(off, "--off", "off");
  setUp.setOption(dt, "--dt", "dt");
  setUp.setOption(ldt, "--ldt", "ldt");
  setUp.setOption(ft, "--ft", "ft");
  setUp.finishSetUp(std::cout);
  setUp.writeParametersFile("", false, false);


  return 0;
}
