#pragma once
/*
 * colorUtils.hpp
 *
 *  Created on: Nov 20, 2014
 *      Author: nickhathaway
 */



#include "bibcpp/graphics/color.hpp"
#include "bibcpp/graphics/colorConstants.hpp"

namespace bib {


inline std::vector<color> multipleHexToColor(const std::vector<std::string> & inColors){
	std::vector<color> outColors(inColors.size());
	for(const auto & pos : iter::range(inColors.size())){
		outColors[pos] = color(inColors[pos]);
	}
	return outColors;
}
inline void printColorsForR(const std::vector<color>& cols, std::ostream & out) {
  out << "c(";
  for (const auto& c : iter::range<uint64_t>(0, cols.size() - 1)) {
    out << "\"#" << cols[c].hexStr_ << "\", ";
  }
  out << "\"#" << cols.back().hexStr_ << "\")" << std::endl;
}
inline std::string getRStringForColors(const std::vector<color>& cols){
	std::stringstream tempStream;
	printColorsForR(cols, tempStream);
	return tempStream.str();
}

inline std::vector<color> evenHuesAll(double sat, double lum, uint32_t number) {
  std::vector<color> cols;
  double hueStep = 360.0 / number;
  for (const auto& h : getRange(0, 360 - hueStep, number)) {
    color currentCol;
    currentCol.setColorByHueSatLum(h, sat, lum);
    cols.emplace_back(currentCol);
  }
  return cols;
}
inline std::vector<color> evenHuesBetweenTwoInc(double sat, double lum, uint32_t number, double hueStart, double hueStop){
  std::vector<color> cols;
  for (const auto& h : getRange(hueStart, hueStop, number)) {
    color currentCol;
    currentCol.setColorByHueSatLum(h, sat, lum);
    cols.emplace_back(currentCol);
  }
  return cols;
}
inline std::vector<color> evenHuesInbetweenTwo(const color& col1, const color& col2,
                                        uint32_t colorNum) {
  double firstHue = col1.hue_;
  double secondHue = col2.hue_;
  double start = 0;
  double stop = 0;
  double step = 0;
  double hueDifference = std::abs(firstHue - secondHue);
  if (hueDifference > 180) {
    if (firstHue < secondHue) {
      firstHue += 360;
    } else {
      secondHue += 360;
    }
  }
  hueDifference = std::abs(firstHue - secondHue);
  // std::cout << hueDifference << std::endl;
  std::vector<color> colsStep;

  if (firstHue < secondHue) {
    start = firstHue + hueDifference / (colorNum + 1);
    stop = secondHue;
    step = hueDifference / (colorNum + 1);
  } else {
    start = firstHue - hueDifference / (colorNum + 1);
    stop = secondHue;
    step = -hueDifference / (colorNum + 1);
  }
  /*
  if(hueDifference > 180){
    hueDifference -= 180;
    if(col1.hue_ < col2.hue_ ){
      start = col1.hue_ - hueDifference/(colorNum + 1);
      stop = col2.hue_;
      step = - hueDifference/(colorNum + 1);
    }else{
      start = col1.hue_ + hueDifference/(colorNum + 1);
      stop = col2.hue_;
      step = hueDifference/(colorNum + 1);
    }
    std::cout << "hueDiff: " << hueDifference << std::endl;
    std::cout << "start: " << start << std::endl;
    std::cout << "stop: " << stop << std::endl;
    std::cout << "step: " <<  step << std::endl;
    std::cout << "colNum: " << colorNum << std::endl;
  }else{
    if(col1.hue_ < col2.hue_ ){
      start = col1.hue_ + hueDifference/(colorNum + 1);
      stop = col2.hue_;
      step = hueDifference/(colorNum + 1);
    }else{
      start = col1.hue_ - hueDifference/(colorNum + 1);
      stop = col2.hue_;
      step = - hueDifference/(colorNum + 1);
    }
  }*/

  /*std::cout << "hueDiff: " << hueDifference << std::endl;
  std::cout << "start: " << start << std::endl;
  std::cout << "stop: " << stop << std::endl;
  std::cout << "step: " <<  step << std::endl;
  std::cout << "colNum: " << colorNum << std::endl;*/
  if (hueDifference == 0) {
    // if hue is the same, just go to the other side of the wheel
    color colCopy = col1;
    colCopy.hue_ = colCopy.hue_ + 180;
    return std::vector<color>(colorNum, colCopy);
  }
  for (const auto& i : iter::range(start, stop, step)) {
    // std::cout << "currentI : " << std::endl;
    color tempCol;
    tempCol.setColorByHueSatLum(i, col1.lSat_, col1.lum_);
    colsStep.emplace_back(tempCol);
  }

  // std::cout << "colsStepSize: " << colsStep.size() << std::endl;
  return colsStep;
}

inline std::vector<std::string> colorsToHStrings(const std::vector<color>& cols) {
  std::vector<std::string> ans;
  for (const auto& col : cols) {
    ans.emplace_back("#" + col.hexStr_);
  }
  return ans;
}
inline std::vector<color> getColsBetweenInc(double hueStart, double hueStop,
                                        double lumStart, double lumStop,
                                        double satStart, double satStop,
                                        uint32_t num){
	std::vector<color> outColors;
	std::vector<double> hues = getRange(hueStart, hueStop, num);
	std::vector<double> sats = getRange(satStart, satStop, num);
	std::vector<double> lums = getRange(lumStart, lumStop, num);
	for(auto n : iter::range(num)){
    color tempCol;
    tempCol.setColorByHueSatLum(hues[n], sats[n], lums[n]);
    outColors.emplace_back(tempCol);
	}
	return outColors;
}
inline std::vector<color> getColsBetweenHueLum(double hueStart, double hueStop,
                                        double lumStart, double lumStop,
                                        uint32_t num) {
  std::vector<color> outTestCols;
  std::vector<double> hueRange = getRange(hueStart, hueStop, num);
  // printVector(hueRange, ",");
  // std::vector<double> satRange = getRange(redCol.lSat_, blueCol.lSat_, num);
  // std::cout << "satStart: " << redCol.lSat_ << " satStop: " << blueCol.lSat_
  // << std::endl;
  // printVector(satRange, ",");
  std::vector<double> lumRange = getRange(lumStart, lumStop, num);
  // std::cout << "lumStart: " << 0.5 << " lumStop: " << lumStop << std::endl;
  // printVector(lumRange, ",");
  for (const auto& i : iter::range(hueRange.size())) {
    color tempCol;
    tempCol.setColorByHueSatLum(hueRange[i], .999, lumRange[i]);
    outTestCols.emplace_back(tempCol);
  }
  return outTestCols;
}
inline std::vector<color> heatmapColors(uint32_t num, double hueStart = 0,
    double hueStop = 240, double lumStart = 0.5,
    double lumStop = 0.25) {
  return getColsBetweenHueLum(hueStart, hueStop, lumStart, lumStop, num);
}

inline std::pair<uint32_t, color> getClosetAnsiColor(const color & otherColor){
	double smallestDistance = UINT32_MAX;
	std::pair<uint32_t, color> ans;
	auto ansiColorCodeToColor = colorspace::createAnsiColorCodeToColor();
	for(const auto & col : ansiColorCodeToColor){
		double currentDistance = col.second.getHSLDistance(otherColor);
		if(currentDistance < smallestDistance){
			smallestDistance = currentDistance;
			ans = col;
		}
	}
	return ans;
}



inline std::vector<std::string> getColorInfo(const color & col, bool getHeader = false, bool ansi = false){
	if(getHeader){
		return std::vector<std::string>{"hexStr", "hue", "sat", "lum", "alpha", "r;g;b", "r", "g", "b", "closestAnsi"};
	}else{
		//"hexStr", "hue", "sat", "lum", "r;g;b", "closestWord", "closestAnsi"
		auto closeAnsi = getClosetAnsiColor(col);
		if(ansi){
			return std::vector<std::string>{estd::to_string(col.hexStr_), estd::to_string(col.hue_),
								estd::to_string(col.lSat_), estd::to_string(col.lum_),
								estd::to_string(col.alpha_), col.getRGBStr(),
								estd::to_string(std::round(col.red_ * 255)),
								estd::to_string(std::round(col.green_ *255)),
								estd::to_string(std::round(col.blue_ *255)),
								bib::bashCT::addBGColor(closeAnsi.first) + estd::to_string(closeAnsi.first) + bib::bashCT::reset,
								};
		}else{
			return std::vector<std::string>{estd::to_string(col.hexStr_), estd::to_string(col.hue_),
								estd::to_string(col.lSat_), estd::to_string(col.lum_),
								estd::to_string(col.alpha_), col.getRGBStr(),
								estd::to_string(std::round(col.red_ * 255)),
								estd::to_string(std::round(col.green_ *255)),
								estd::to_string(std::round(col.blue_ *255)),
								estd::to_string(closeAnsi.first),
								};
		}

	}

}


inline std::vector<std::string> getColorInfo(const color & col, const color & otherColor,
		bool getHeader = false, bool ansi = false){
	if(getHeader){
		return std::vector<std::string>{"hexStr", "hue", "sat", "lum", "alpha", "r;g;b", "r", "g", "b", "lastDiff", "closestAnsi"};
	}else{
		//"hexStr", "hue", "sat", "lum", "r;g;b", "lastDiff" ,"closestWord", "closestAnsi"
		auto closeAnsi = getClosetAnsiColor(col);
		if(ansi){
			return std::vector<std::string>{estd::to_string(col.hexStr_), estd::to_string(col.hue_),
						estd::to_string(col.lSat_), estd::to_string(col.lum_),
						estd::to_string(col.alpha_), col.getRGBStr(),
						estd::to_string(std::round(col.red_ * 255)),
						estd::to_string(std::round(col.green_ * 255)),
						estd::to_string(std::round(col.blue_ * 255)),
						estd::to_string(otherColor.getHSLDistance(col)),
				  	bib::bashCT::addBGColor(closeAnsi.first) + estd::to_string(closeAnsi.first) + bib::bashCT::reset
						};
		}else{
			return std::vector<std::string>{estd::to_string(col.hexStr_), estd::to_string(col.hue_),
						estd::to_string(col.lSat_), estd::to_string(col.lum_),
						estd::to_string(col.alpha_), col.getRGBStr(),
						estd::to_string(std::round(col.red_ * 255)),
						estd::to_string(std::round(col.green_ *255)),
						estd::to_string(std::round(col.blue_ *255)),
						estd::to_string(otherColor.getHSLDistance(col)),
				  	estd::to_string(closeAnsi.first)
						};
		}
	}
}


} /* namespace bib */


