#include <iostream>
#include "bibcpp.h"

using namespace bib;

int main(int argc, char* argv[]){
	try {
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
		char cVal = 'A';
		bib::files::bfs::path testPath = "/home/hathawan";
	  bib::progutils::ProgramSetUp setUp(argc, argv);
	  setUp.setOption(cVal, "--cVal", "cVal");
	  setUp.setOption(testPath, "--testPath", "testPath");
	  setUp.setOption(ui16t, "--ui16t", "ui16t");
	  setUp.setOption(ui32t, "--ui32t", "ui32t");
	  setUp.setOption(ui64t, "--ui64t", "ui64t");
	  setUp.setOption(st, "--st", "st");
	  setUp.setOption(i16t, "--i16t", "i16t");
	  setUp.setOption(i32t, "--i32t", "i32t");
	  setUp.setOption(i64t, "--i64t", "i64t");
	  setUp.setOption(str, "--str", "str");
	  setUp.setOption(off, "--off", "off");
	  setUp.setOption(dt, "--dt,-d", "dt");
	  setUp.setOption(ldt, "--ldt", "ldt");
	  setUp.setOption(ft, "--ft,-f", "ft");
	  setUp.finishSetUp(std::cout);
	  setUp.writeParametersFile("", false, false);
		//;
//		auto val = std::is_same<char, typename std::decay<char>::type>::value;
//		auto supported = progutils::CmdArgs::is_cmdArg_supported_type<char>::value;
//		auto supported2 = setUp.commands_.isSupportedType<char>();
//		std::cout <<  bib::colorBool(val) << std::endl;
//		std::cout <<  bib::colorBool(supported) << std::endl;
//		std::cout <<  bib::colorBool(supported2) << std::endl;
//		std::cout << typeStr<char>() << std::endl;

	} catch (std::exception & e) {
		std::cerr << e.what() << std::endl;
		exit(1);
	}



  return 0;
}
