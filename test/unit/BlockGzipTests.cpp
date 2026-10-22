#include <catch_amalgamated.hpp>
#include "njhcpp/IO/BlockGzip.hpp"

using namespace njh;

namespace {
//a fresh temporary directory removed at the end of the test
struct TempDir {
	bfs::path dir_;
	TempDir() :
			dir_(bfs::temp_directory_path() / bfs::unique_path("njhcppTest-%%%%-%%%%")) {
		bfs::create_directories(dir_);
	}
	~TempDir() {
		bfs::remove_all(dir_);
	}
};

std::string readAllGz(const bfs::path & fnp) {
	GZSTREAM::igzstream in(fnp);
	std::stringstream ss;
	ss << in.rdbuf();
	return ss.str();
}
}  // namespace

TEST_CASE("BlockGzip writes a normal gzip file and an index", "[BlockGzip]") {
	TempDir tmp;
	auto fnp = tmp.dir_ / "table.tab.txt.gz";
	std::string bigBlock;
	for (uint32_t i = 0; i < 50000; ++i) {
		bigBlock += "s3\t" + std::to_string(i) + "\n";
	}
	{
		BlockGzipOutputStream out(fnp, false);
		out << "s_Sample\tval\n";
		out.startBlock("s1");
		out << "s1\t1\n" << "s1\t2\n";
		out.endBlock();
		out.writeBlock("s2", "s2\t3\n");
		out.writeBlock("empty", "");
		out.writeBlock("s3", bigBlock);
	}
	REQUIRE(readAllGz(fnp) == "s_Sample\tval\ns1\t1\ns1\t2\ns2\t3\n" + bigBlock);

	BlockGzipIndex index(fnp);
	REQUIRE(index.keys() == std::vector<std::string> { "s1", "s2", "empty", "s3" });
	REQUIRE(index.has("s2"));
	REQUIRE_FALSE(index.has("s4"));
	REQUIRE(index.readBlock("s2") == "s2\t3\n");
	REQUIRE(index.readBlock("s1") == "s1\t1\ns1\t2\n");
	REQUIRE(index.readBlock("empty").empty());
	REQUIRE(index.readBlock("s3") == bigBlock);
	REQUIRE_THROWS(index.readBlock("s4"));
}

TEST_CASE("BlockGzip guards against misuse", "[BlockGzip]") {
	TempDir tmp;
	auto fnp = tmp.dir_ / "guards.txt.gz";
	{
		BlockGzipOutputStream out(fnp, false);
		out.writeBlock("s1", "a\n");
		REQUIRE_THROWS(out.writeBlock("s1", "b\n"));
		REQUIRE_THROWS(out.endBlock());
		out.startBlock("s2");
		REQUIRE_THROWS(out.startBlock("s3"));
		out << "c\n";
		//closing ends the open block
	}
	BlockGzipIndex index(fnp);
	REQUIRE(index.readBlock("s2") == "c\n");
	REQUIRE_THROWS(BlockGzipOutputStream(fnp, false));
	REQUIRE_NOTHROW(BlockGzipOutputStream(fnp, true));
}
