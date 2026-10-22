#include <catch_amalgamated.hpp>
#include "njhcpp/IO/OutputStream.hpp"
#include "njhcpp/IO/InputStream.hpp"

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

std::string readAll(const bfs::path & fnp) {
	InputStream in { InOptions(fnp) };
	std::stringstream ss;
	ss << in.rdbuf();
	return ss.str();
}
}  // namespace

TEST_CASE("OutOptions gzip compression level", "[OutOptions]") {
	TempDir tmp;
	std::stringstream text;
	for (uint32_t i = 0; i < 20000; ++i) {
		text << "sample" << i % 300 << "\thap" << i % 17 << "\t" << i * 31 % 1000 << "\n";
	}
	auto write = [&tmp, &text](const std::string & name, int level) {
		OutOptions opts(tmp.dir_ / name);
		opts.gzCompressionLevel_ = level;
		{
			OutputStream out(opts);
			out << text.str();
		}
		return tmp.dir_ / name;
	};
	auto defaultFnp = write("default.txt.gz", GZSTREAM::defaultCompressionLevel);
	auto zlibDefaultFnp = write("zlibDefault.txt.gz", -1);
	auto fastFnp = write("fast.txt.gz", 1);
	auto smallFnp = write("small.txt.gz", 9);
	REQUIRE(readAll(defaultFnp) == text.str());
	REQUIRE(readAll(zlibDefaultFnp) == text.str());
	REQUIRE(bfs::file_size(zlibDefaultFnp) < bfs::file_size(defaultFnp));
	REQUIRE(readAll(fastFnp) == text.str());
	REQUIRE(readAll(smallFnp) == text.str());
	REQUIRE(bfs::file_size(smallFnp) < bfs::file_size(fastFnp));

	SECTION("the level round trips through json") {
		OutOptions opts(tmp.dir_ / "x.txt.gz");
		opts.gzCompressionLevel_ = 3;
		OutOptions fromJson(opts.toJson());
		REQUIRE(3 == fromJson.gzCompressionLevel_);
		OutOptions noLevel { Json::Value(Json::objectValue) };
		REQUIRE(GZSTREAM::defaultCompressionLevel == noLevel.gzCompressionLevel_);
		REQUIRE(GZSTREAM::defaultCompressionLevel == OutOptions().gzCompressionLevel_);
	}
	SECTION("an invalid level throws when opening") {
		OutOptions opts(tmp.dir_ / "bad.txt.gz");
		opts.gzCompressionLevel_ = 12;
		REQUIRE_THROWS(OutputStream(opts));
	}
}
