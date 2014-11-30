#pragma once
/*
 * colorUtils.hpp
 *
 *  Created on: Nov 20, 2014
 *      Author: nickhathaway
 */



#include "bibcpp/graphics/color.hpp"
#include "bibcpp/graphics/colorConstants.hpp"
#include "bibcpp/bashUtils.h"
#include "bibcpp/utils/vecUtils.hpp" //catVecs
namespace bib {

/**@b Get a vector of colors from a vector of hex strings colors
 *
 * @param inColors A vector of hex strings
 * @return A vector of colors for the hex strings
 */
inline std::vector<color> multipleHexToColor(const std::vector<std::string> & inColors){
	std::vector<color> outColors(inColors.size());
	for(const auto & pos : iter::range(inColors.size())){
		outColors[pos] = color(inColors[pos]);
	}
	return outColors;
}

/**@b Print to out hex strings in a format that can be copied into R
 *
 * @param cols The vector of colors
 * @param out the ostream to print the hex color strings to
 */
inline void printColorsForR(const std::vector<color>& cols, std::ostream & out) {
  out << "c(";
  for (const auto& c : iter::range<uint64_t>(0, cols.size() - 1)) {
    out << "\"#" << cols[c].hexStr_ << "\", ";
  }
  out << "\"#" << cols.back().hexStr_ << "\")" << std::endl;
}
/**@b Get a string of hex colors in a format to pass to R
 *
 * @param cols The vector of colors to get the formatted string for R
 * @return The string formated for R
 */
inline std::string getRStringForColors(const std::vector<color>& cols){
	std::stringstream tempStream;
	printColorsForR(cols, tempStream);
	return tempStream.str();
}

/**@b Get colors around the color wheel for the given saturation and luminance
 *
 * @param sat The saturation value
 * @param lum The luminance value
 * @param number The number of colors to get
 * @return A vector of colors around the color wheel for a given sat and lum
 */
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

/**@b Convert a vector of colors to a vector of hex color strings
 *
 * @param cols The colors to convert
 * @return The vector of hex color strings
 */
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

/**@b Get a range of colors that could be used for a heatmap
 *
 * @param num The number of colors to get
 * @param hueStart
 * @param hueStop
 * @param lumStart
 * @param lumStop
 * @param satStart
 * @param satStop
 * @return
 */
inline std::vector<color> heatmapColors(uint32_t num) {
	uint32_t quater = num/4;
	double hueStart = 0, hueStop = 60,
			lumStart = 0.5,lumStop = .99,
			satStart = 1, satStop = 1;
	if(quater > 0){
		return catVecs(getColsBetweenInc(hueStart, hueStop, lumStart, lumStart , satStart, satStop, num - quater),
				getColsBetweenInc(hueStop, hueStop, lumStart + (lumStop - lumStart)/quater, lumStop,satStart, satStop, quater));
	}else{
		return getColsBetweenInc(hueStart, hueStop, lumStart, lumStop,satStart, satStop, num);
	}

}

inline std::pair<uint32_t, color> getClosetAnsiColor(const color & otherColor){
	double smallestDistance = std::numeric_limits<uint32_t>::max();
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

/**@b Hand selected color range, works well only to about 12 colors or so
 *
 * @param num The number of colors to get (only gives really distinct colors up to about 12 colors)
 * @param hueStart The start of the hue selection (default 120)
 * @param hueStop The stop of the hue selection (default 420)
 * @param lumStart The start of the lum selection (default 0.40)
 * @param lumStop The stop of the lum selection (default 0.70)
 * @param satStart The start of the sat selection (default 0.80)
 * @param satStop The start of the lum selection (default 1.00)
 * @return A vector of pseudo-evenly spaced colors
 */
inline std::vector<color> njhColors(uint32_t num, double hueStart = 120, double hueStop = 420,
    double lumStart = 0.40, double lumStop = 0.70,
    double satStart = 0.80, double satStop = 1.00){
	return getColsBetweenInc(hueStart, hueStop, lumStart, lumStop, satStart, satStop, num);
}



} /* namespace bib */


