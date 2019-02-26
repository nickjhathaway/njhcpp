#include <iostream>
#include "njhcpp.h"



using namespace njh;

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
		njh::files::bfs::path testPath = "/home/hathawan";

		std::vector<uint16_t> ui16t_vec{};
		std::set<uint16_t> ui16t_set{};

		std::vector<uint32_t> ui32t_vec{};
		std::set<uint32_t> ui32t_set{};

		std::vector<uint64_t> ui64t_vec{};
		std::set<uint64_t> ui64t_set{};

		std::vector<int16_t> i16t_vec{};
		std::set<int16_t> i16t_set{};

		std::vector<int32_t> i32t_vec{};
		std::set<int32_t> i32t_set{};

		std::vector<int64_t> i64t_vec{};
		std::set<int64_t> i64t_set{};

		std::vector<size_t> st_vec{};
		std::set<size_t> st_set{};

		std::vector<std::string> str_vec{};
		std::set<std::string> str_set{};

		std::vector<double> dt_vec{};
		std::set<double> dt_set{};

		std::vector<long double> ldt_vec{};
		std::set<long double> ldt_set{};

		std::vector<float> ft_vec{};
		std::set<float> ft_set{};

		std::vector<char> cVal_vec{};
		std::set<char> cVal_set{};

		std::vector<njh::files::bfs::path> path_vec{};
		std::set<njh::files::bfs::path> path_set{};


	  njh::progutils::ProgramSetUp setUp(argc, argv);
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

	  setUp.setOption(ui16t_vec, "--ui16t_vec", "ui16t_vec");
	  setUp.setOption(ui16t_set, "--ui16t_set", "ui16t_set");

	  setUp.setOption(ui32t_vec, "--ui32t_vec", "ui32t_vec");
	  setUp.setOption(ui32t_set, "--ui32t_set", "ui32t_set");

	  setUp.setOption(ui64t_vec, "--ui64t_vec", "ui64t_vec");
	  setUp.setOption(ui64t_set, "--ui64t_set", "ui64t_set");

	  setUp.setOption(i16t_vec, "--i16t_vec", "i16t_vec");
	  setUp.setOption(i16t_set, "--i16t_set", "i16t_set");

	  setUp.setOption(i32t_vec, "--i32t_vec", "i32t_vec");
	  setUp.setOption(i32t_set, "--i32t_set", "i32t_set");

	  setUp.setOption(i64t_vec, "--i64t_vec", "i64t_vec");
	  setUp.setOption(i64t_set, "--i64t_set", "i64t_set");

	  setUp.setOption(st_vec, "--st_vec", "st_vec");
	  setUp.setOption(st_set, "--st_set", "st_set");

	  setUp.setOption(str_vec, "--str_vec", "str_vec");
	  setUp.setOption(str_set, "--str_set", "str_set");

	  setUp.setOption(dt_vec, "--dt_vec", "dt_vec");
	  setUp.setOption(dt_set, "--dt_set", "dt_set");

	  setUp.setOption(ldt_vec, "--ldt_vec", "ldt_vec");
	  setUp.setOption(ldt_set, "--ldt_set", "ldt_set");

	  setUp.setOption(ft_vec, "--ft_vec", "ft_vec");
	  setUp.setOption(ft_set, "--ft_set", "ft_set");

	  setUp.setOption(cVal_vec, "--cVal_vec", "cVal_vec");
	  setUp.setOption(cVal_set, "--cVal_set", "cVal_set");

	  setUp.setOption(path_vec, "--path_vec", "path_vec");
	  setUp.setOption(path_set, "--path_set", "path_set");

	  setUp.finishSetUp(std::cout);
	  setUp.writeParametersFile("", false, false);



	} catch (std::exception & e) {
		std::cerr << e.what() << std::endl;
		exit(1);
	}

  return 0;
}
