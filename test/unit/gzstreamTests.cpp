#include <catch_amalgamated.hpp>
#include "njhcpp/files/fileObjects/gzstream.hpp"

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

//larger than the stream buffer so reads and writes cross buffer refills
std::string makeLines(uint32_t count) {
	std::stringstream ss;
	for (uint32_t i = 0; i < count; ++i) {
		ss << "@read." << i << "\nACGTTGCA" << i % 97 << "\n+\nIIIIIIII\n";
	}
	return ss.str();
}
}  // namespace

TEST_CASE("gzstream round trips text larger than its buffer", "[gzstream]") {
	TempDir tmp;
	auto fnp = tmp.dir_ / "lines.txt.gz";
	auto text = makeLines(20000);
	REQUIRE(text.size() > 64 * 1024 * 4);
	{
		GZSTREAM::ogzstream out(fnp);
		out << text;
	}
	REQUIRE(readAllGz(fnp) == text);

	SECTION("reading line by line") {
		GZSTREAM::igzstream in(fnp);
		std::string line;
		uint32_t count = 0;
		while (std::getline(in, line)) {
			++count;
		}
		REQUIRE(count == 20000 * 4);
	}
}

TEST_CASE("gzstream compression level", "[gzstream]") {
	TempDir tmp;
	auto text = makeLines(20000);
	auto fastFnp = tmp.dir_ / "fast.txt.gz";
	auto smallFnp = tmp.dir_ / "small.txt.gz";
	{
		GZSTREAM::ogzstream out;
		out.open(fastFnp, 1, std::ios::out);
		out << text;
	}
	{
		GZSTREAM::ogzstream out;
		out.open(smallFnp, 9, std::ios::out);
		out << text;
	}
	REQUIRE(readAllGz(fastFnp) == text);
	REQUIRE(readAllGz(smallFnp) == text);
	REQUIRE(bfs::file_size(smallFnp) < bfs::file_size(fastFnp));
	GZSTREAM::gzstreambuf buf;
	REQUIRE_THROWS(buf.setCompressionLevel(10));
	REQUIRE_THROWS(buf.setCompressionLevel(-2));
}

TEST_CASE("gzstream members", "[gzstream]") {
	TempDir tmp;
	auto fnp = tmp.dir_ / "members.txt.gz";
	uint64_t afterFirst = 0;
	{
		GZSTREAM::ogzstream out(fnp);
		REQUIRE(0 == out.rdbuf()->compressedOffset());
		//nothing written so no member is ended
		REQUIRE_FALSE(out.rdbuf()->endMember());
		out << "first\n";
		REQUIRE(out.rdbuf()->endMember());
		afterFirst = out.rdbuf()->compressedOffset();
		REQUIRE(afterFirst > 0);
		out << "second\n";
		REQUIRE(out.rdbuf()->endMember());
		REQUIRE(out.rdbuf()->compressedOffset() > afterFirst);
		out << "third\n";
	}
	//the members read as one stream
	REQUIRE(readAllGz(fnp) == "first\nsecond\nthird\n");
}
