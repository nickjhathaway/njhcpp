#include <catch_amalgamated.hpp>
#include "njhcpp/system/sysUtils.hpp"

#include <chrono>
#include <future>

using namespace njh;

namespace {
//runs the command on a detached thread so a hang fails the test instead of hanging it
std::optional<sys::RunOutput> runWithin(const std::string & cmd, std::chrono::seconds limit) {
	auto done = std::make_shared<std::promise<sys::RunOutput>>();
	auto finished = done->get_future();
	std::thread([cmd, done]() {
		done->set_value(sys::run({cmd}));
	}).detach();
	if (std::future_status::ready != finished.wait_for(limit)) {
		return std::nullopt;
	}
	return finished.get();
}
}  // namespace

TEST_CASE("sys::run captures stdout, stderr and success", "[sysRun]") {
	auto out = sys::run({"echo", "hello;", "echo", "oops", "1>&2"});
	REQUIRE(out.success_);
	REQUIRE(0 == out.returnCode_);
	REQUIRE("hello" == out.stdOut_);
	REQUIRE("oops" == out.stdErr_);
}

TEST_CASE("sys::run reports the exit code", "[sysRun]") {
	auto out = sys::run({"exit 3"});
	REQUIRE_FALSE(out.success_);
	REQUIRE(3 == out.returnCode_);
	auto missing = sys::run({"thisCommandDoesNotExist_njhcpp"});
	REQUIRE_FALSE(missing.success_);
	REQUIRE(127 == missing.returnCode_);
	auto killed = sys::run({"kill -9 $$"});
	REQUIRE_FALSE(killed.success_);
	REQUIRE(128 + 9 == killed.returnCode_);
}

TEST_CASE("sys::run doesn't block on a lot of stderr", "[sysRun]") {
	auto out = runWithin("head -c 1000000 /dev/zero | tr '\\0' 'e' 1>&2; echo done", std::chrono::seconds(20));
	REQUIRE(out.has_value());
	REQUIRE(out->success_);
	REQUIRE("done" == out->stdOut_);
	REQUIRE(1000000 == out->stdErr_.size());
}

TEST_CASE("sys::run reads a lot of stdout and stderr written together", "[sysRun]") {
	auto out = runWithin("for i in $(seq 1 20); do head -c 100000 /dev/zero | tr '\\0' 'o'; head -c 100000 /dev/zero | tr '\\0' 'e' 1>&2; done", std::chrono::seconds(20));
	REQUIRE(out.has_value());
	REQUIRE(2000000 == out->stdOut_.size());
	REQUIRE(2000000 == out->stdErr_.size());
}

TEST_CASE("sys::run from many threads at once", "[sysRun]") {
	std::vector<std::string> outs(40);
	std::vector<std::thread> threads;
	for (uint32_t pos = 0; pos < outs.size(); ++pos) {
		threads.emplace_back([pos, &outs]() {
			outs[pos] = sys::run({"sleep 0.$((RANDOM % 3)); echo", estd::to_string(pos)}).stdOut_;
		});
	}
	for (auto & t : threads) {
		t.join();
	}
	for (uint32_t pos = 0; pos < outs.size(); ++pos) {
		REQUIRE(estd::to_string(pos) == outs[pos]);
	}
}
