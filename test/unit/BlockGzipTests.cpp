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

namespace {
std::string readBytes(const bfs::path & fnp, uint64_t offset, uint64_t size) {
	std::ifstream in(fnp.string(), std::ios::binary);
	in.seekg(offset);
	std::string ret(size, '\0');
	in.read(ret.data(), ret.size());
	return ret;
}

//write a block gzip file with the given blocks, and an optional header before them
void writeBlocks(const bfs::path & fnp, const std::vector<std::pair<std::string, std::string>> & blocks,
		const std::string & header = "", int compressionLevel = GZSTREAM::defaultCompressionLevel) {
	BlockGzipOutputStream out(fnp, false, compressionLevel);
	out << header;
	for (const auto & block : blocks) {
		out.writeBlock(block.first, block.second);
	}
}
}  // namespace

TEST_CASE("concatBlockGzip joins files by copying their blocks", "[BlockGzip]") {
	TempDir tmp;
	std::string bigBlock;
	for (uint32_t i = 0; i < 50000; ++i) {
		bigBlock += "fam3\t" + std::to_string(i) + "\n";
	}
	std::vector<std::vector<std::pair<std::string, std::string>>> batches {
		{ { "fam1", "fam1\ta\nfam1\tb\n" }, { "fam2", "fam2\tc\n" } },
		{ { "empty", "" }, { "fam3", bigBlock } },
		{ { "fam4", "fam4\td\n" }, { "fam5", "fam5\te\nfam5\tf\n" }, { "fam6", "fam6\tg\n" } } };
	std::vector<bfs::path> inputs;
	std::string expectedText;
	std::vector<std::string> expectedKeys;
	for (uint32_t i = 0; i < batches.size(); ++i) {
		inputs.emplace_back(tmp.dir_ / ("batch" + std::to_string(i) + ".txt.gz"));
		//different compression levels, the blocks are copied so it doesn't matter
		writeBlocks(inputs.back(), batches[i], "", 1 + i * 4);
		expectedText += readAllGz(inputs.back());
		for (const auto & block : batches[i]) {
			expectedKeys.emplace_back(block.first);
		}
	}
	auto out = tmp.dir_ / "all.txt.gz";
	concatBlockGzip(inputs, out);

	REQUIRE(readAllGz(out) == expectedText);
	BlockGzipIndex index(out);
	REQUIRE(index.keys() == expectedKeys);
	auto outEntries = BlockGzipIndex::readIndex(out);
	uint32_t entryPos = 0;
	for (uint32_t i = 0; i < batches.size(); ++i) {
		auto inEntries = BlockGzipIndex::readIndex(inputs[i]);
		for (uint32_t j = 0; j < batches[i].size(); ++j) {
			REQUIRE(index.readBlock(batches[i][j].first) == batches[i][j].second);
			//the compressed bytes are copied, not recompressed
			const auto & outEntry = outEntries[entryPos++];
			REQUIRE(outEntry.size == inEntries[j].size);
			REQUIRE(readBytes(out, outEntry.offset, outEntry.size) == readBytes(inputs[i], inEntries[j].offset, inEntries[j].size));
		}
	}
	REQUIRE_THROWS(concatBlockGzip(inputs, out));
	REQUIRE_NOTHROW(concatBlockGzip(inputs, out, true, true));
	REQUIRE_THROWS(concatBlockGzip({ inputs[0], out }, out, true, true));
}

TEST_CASE("concatBlockGzip writes a shared header once and keeps text between blocks", "[BlockGzip]") {
	TempDir tmp;
	std::string header = "family\tval\n";
	auto a = tmp.dir_ / "a.txt.gz";
	auto b = tmp.dir_ / "b.txt.gz";
	auto noHeader = tmp.dir_ / "noHeader.txt.gz";
	writeBlocks(a, { { "fam1", "fam1\t1\n" } }, header);
	{
		BlockGzipOutputStream out(b, false);
		out << header;
		out.writeBlock("fam2", "fam2\t2\n");
		out << "unindexed\n";
		out.writeBlock("fam3", "fam3\t3\n");
		out << "trailing\n";
	}
	writeBlocks(noHeader, { { "fam4", "fam4\t4\n" } });
	auto out = tmp.dir_ / "all.txt.gz";
	concatBlockGzip({ a, b, noHeader }, out);
	REQUIRE(readAllGz(out) == header + "fam1\t1\nfam2\t2\nunindexed\nfam3\t3\ntrailing\nfam4\t4\n");
	BlockGzipIndex index(out);
	REQUIRE(index.keys() == std::vector<std::string> { "fam1", "fam2", "fam3", "fam4" });
	REQUIRE(index.readBlock("fam3") == "fam3\t3\n");

	auto otherHeader = tmp.dir_ / "otherHeader.txt.gz";
	writeBlocks(otherHeader, { { "fam5", "fam5\t5\n" } }, "family\tother\n");
	auto out2 = tmp.dir_ / "all2.txt.gz";
	REQUIRE_THROWS(concatBlockGzip({ a, otherHeader }, out2));
	REQUIRE_FALSE(bfs::exists(out2));
}

TEST_CASE("concatBlockGzip handles duplicate keys", "[BlockGzip]") {
	TempDir tmp;
	auto a = tmp.dir_ / "a.txt.gz";
	auto b = tmp.dir_ / "b.txt.gz";
	writeBlocks(a, { { "fam1", "fam1\tfirst\n" }, { "fam2", "fam2\t2\n" } });
	writeBlocks(b, { { "fam3", "fam3\t3\n" }, { "fam1", "fam1\tsecond\n" }, { "fam4", "fam4\t4\n" } });
	auto out = tmp.dir_ / "all.txt.gz";
	REQUIRE_THROWS(concatBlockGzip({ a, b }, out));
	REQUIRE_FALSE(bfs::exists(out));
	REQUIRE_FALSE(bfs::exists(BlockGzipOutputStream::indexPath(out)));

	concatBlockGzip({ a, b }, out, false);
	REQUIRE(readAllGz(out) == "fam1\tfirst\nfam2\t2\nfam3\t3\nfam4\t4\n");
	BlockGzipIndex index(out);
	REQUIRE(index.keys() == std::vector<std::string> { "fam1", "fam2", "fam3", "fam4" });
	REQUIRE(index.readBlock("fam1") == "fam1\tfirst\n");
	REQUIRE(index.readBlock("fam4") == "fam4\t4\n");
}

TEST_CASE("concatBlockGzip checks each index matches its file", "[BlockGzip]") {
	TempDir tmp;
	auto good = tmp.dir_ / "good.txt.gz";
	writeBlocks(good, { { "fam1", "fam1\t1\n" } });
	auto out = tmp.dir_ / "all.txt.gz";

	//truncated, the last block runs past the end of the file
	auto truncated = tmp.dir_ / "truncated.txt.gz";
	writeBlocks(truncated, { { "fam2", "fam2\t2\n" }, { "fam3", "fam3\t3\n" } });
	bfs::resize_file(truncated, bfs::file_size(truncated) - 5);
	REQUIRE_THROWS(concatBlockGzip({ good, truncated }, out));
	REQUIRE_FALSE(bfs::exists(out));

	//extra bytes after the indexed blocks that aren't a gzip member
	auto extra = tmp.dir_ / "extra.txt.gz";
	writeBlocks(extra, { { "fam4", "fam4\t4\n" } });
	{
		std::ofstream app(extra.string(), std::ios::binary | std::ios::app);
		app << "not gzip";
	}
	REQUIRE_THROWS(concatBlockGzip({ good, extra }, out));

	//an index from a different file, offsets don't land on members
	auto stale = tmp.dir_ / "stale.txt.gz";
	writeBlocks(stale, { { "fam5", std::string(1000, 'a') }, { "fam6", "fam6\t6\n" } });
	{
		std::ofstream idx(BlockGzipOutputStream::indexPath(stale).string(), std::ios::trunc);
		idx << "key\toffset\tsize\n" << "fam5\t3\t10\n";
	}
	REQUIRE_THROWS(concatBlockGzip({ good, stale }, out));
	REQUIRE_FALSE(bfs::exists(out));
	REQUIRE_THROWS(concatBlockGzip({ good, tmp.dir_ / "missing.txt.gz" }, out));
}
