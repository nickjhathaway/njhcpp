#pragma once
/*
 * LockableJsonLog.hpp
 *
 *  Created on: Mar 18, 2017
 *      Author: nick
 */



#include "njhcpp/jsonUtils.h"
#include "njhcpp/IO.h"


namespace njh {

class LockableJsonLog {

public:

	LockableJsonLog(const bfs::path &logFnp, bool overWrite = true) :
			logFileOpts_(logFnp) {
		logFileOpts_.overWriteFile_ = overWrite;
		log_["date"] = njh::json::toJson(njh::getCurrentDateFull());
	}

	Json::Value log_;
	std::mutex logMut_;

	OutOptions logFileOpts_;

	njh::stopWatch watch_;

	void addToLog(const std::string &uid, const Json::Value &adding) {
		std::lock_guard<std::mutex> lock(logMut_);
		if (log_.isMember(uid)) {
			std::stringstream ss;
			ss << __PRETTY_FUNCTION__ << ", error, log already has info for " << uid
					<< "\n";
			throw std::runtime_error { ss.str() };
		}
		log_[uid] = adding;
	}

	void writeLog() {
		std::lock_guard<std::mutex> lock(logMut_);
		std::ofstream outFile;
		logFileOpts_.openFile(outFile);
		log_["totalTime"] = njh::json::toJson(watch_.totalTime());
		outFile << log_ << std::endl;
		;
	}
};

} /* namespace njhseq */


