#include <catch_amalgamated.hpp>
#include "njhcpp/files/fileSystemUtils.hpp"

#include <fstream>

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

void writeFile(const bfs::path & fnp, const std::string & text) {
	std::ofstream out(fnp.string());
	out << text;
}

//a directory with a file in it and a file in a sub directory, standing in for data a link points to
bfs::path makeData(const bfs::path & dir) {
	bfs::create_directories(dir / "sub");
	writeFile(dir / "keep.txt", "keep");
	writeFile(dir / "sub" / "keep2.txt", "keep2");
	return dir;
}

void requireDataIntact(const bfs::path & dir) {
	REQUIRE(bfs::exists(dir / "keep.txt"));
	REQUIRE(bfs::exists(dir / "sub" / "keep2.txt"));
}
}  // namespace

TEST_CASE("rmDirForce removes a directory tree", "[rmDirForce]") {
	TempDir tmp;
	auto toRemove = tmp.dir_ / "out";
	makeData(toRemove);
	REQUIRE(files::rmDirForce(toRemove));
	REQUIRE_FALSE(bfs::exists(toRemove));
}

TEST_CASE("rmDirForce removes symlinks inside the directory without touching what they point to", "[rmDirForce]") {
	TempDir tmp;
	auto data = makeData(tmp.dir_ / "data");
	auto toRemove = tmp.dir_ / "out";
	bfs::create_directories(toRemove / "nested");
	writeFile(toRemove / "own.txt", "own");
	bfs::create_directory_symlink(data, toRemove / "linkToDir");
	bfs::create_directory_symlink(data, toRemove / "nested" / "linkToDir");
	bfs::create_symlink(data / "keep.txt", toRemove / "linkToFile");
	bfs::create_symlink(tmp.dir_ / "doesNotExist", toRemove / "danglingLink");

	REQUIRE(files::rmDirForce(toRemove));
	REQUIRE_FALSE(bfs::exists(bfs::symlink_status(toRemove)));
	requireDataIntact(data);
}

TEST_CASE("rmDirForce on a symlink to a directory removes only the link", "[rmDirForce]") {
	TempDir tmp;
	auto data = makeData(tmp.dir_ / "data");
	auto link = tmp.dir_ / "link";
	bfs::create_directory_symlink(data, link);

	REQUIRE(files::rmDirForce(link));
	REQUIRE_FALSE(bfs::exists(bfs::symlink_status(link)));
	requireDataIntact(data);
}

TEST_CASE("rmDirForce with a trailing slash on a symlink to a directory removes only the link", "[rmDirForce]") {
	TempDir tmp;
	auto data = makeData(tmp.dir_ / "data");
	auto link = tmp.dir_ / "link";
	bfs::create_directory_symlink(data, link);

	REQUIRE(files::rmDirForce(link.string() + "/"));
	REQUIRE_FALSE(bfs::exists(bfs::symlink_status(link)));
	requireDataIntact(data);
}

TEST_CASE("rmDirForce with a trailing slash on a directory removes it", "[rmDirForce]") {
	TempDir tmp;
	auto toRemove = makeData(tmp.dir_ / "out");
	REQUIRE(files::rmDirForce(toRemove.string() + "//"));
	REQUIRE_FALSE(bfs::exists(toRemove));
}

TEST_CASE("makeDir with overWrite on a symlink to a directory leaves the target alone", "[rmDirForce]") {
	TempDir tmp;
	auto data = makeData(tmp.dir_ / "data");
	auto link = tmp.dir_ / "out";
	bfs::create_directory_symlink(data, link);

	files::makeDir(files::MkdirPar(link, true));
	REQUIRE(bfs::is_directory(bfs::symlink_status(link)));
	requireDataIntact(data);
}

TEST_CASE("makeDir with overWrite leaves the targets of symlinks in the old directory alone", "[rmDirForce]") {
	TempDir tmp;
	auto data = makeData(tmp.dir_ / "data");
	auto outDir = tmp.dir_ / "out";
	bfs::create_directories(outDir);
	bfs::create_directory_symlink(data, outDir / "linkToDir");

	files::makeDir(files::MkdirPar(outDir, true));
	REQUIRE(bfs::is_directory(outDir));
	REQUIRE(bfs::is_empty(outDir));
	requireDataIntact(data);
}
