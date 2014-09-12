#pragma once
/*
 * textFormatter.hpp
 *
 *  Created on: Sep 12, 2014
 *      Author: nickhathaway
 */

#include <string>
#include <vector>
#include <numeric>  // iota
#include <iostream> // ostream
#include <cstdint>  // uint32_t

#include "bibcpp/utils/stringUtils.hpp" // conToStr


namespace bib {
/**@b Namespace to hold ansi escape character for changing the text on the terminal
 *
 */
namespace bashCT {
	// Reset
const std::string reset = "\033[0m";   /**Text Reset*/

/**@b Reset the text but then add any modifies that you want to keep
 *
 * @param add A vector of strings of text modifying
 * @return return reset code plus add
 */
inline std::string resetAdd(const std::vector<std::string>& add){
	return reset + conToStr(add, "");
}
/**@b Reset the text but then add a paramter you want to keep, for ex. reset to reset color but keep bold
 *
 * @param add A string to modify the text
 * @return return reset code plus add
 */
inline std::string resetAdd(std::string add){
	return reset + add;
}

// Regular Colors
const std::string black = "\033[30m";  /**black*/
const std::string red =   "\033[31m";  /**red*/
const std::string green = "\033[32m";  /**green*/
const std::string yellow ="\033[33m";  /**yellow*/
const std::string blue =  "\033[34m";  /**blue*/
const std::string purple ="\033[35m";  /**purple*/
const std::string cyan =  "\033[36m";  /**cyan*/
const std::string white = "\033[37m";  /**white*/

// affecting by adding bold, underline, or flashing text
const std::string bold =     "\033[1m";
const std::string underline ="\033[4m";
const std::string flashing = "\033[5m";
// affect the color of the text by decreasing saturation or by inverting/switching background and text colors
const std::string lighten =  "\033[2m";
const std::string invert =   "\033[7m";
// Background
const std::string on_Black = "\033[40m";      // Black
const std::string on_Red = "\033[41m";        // Red
const std::string on_Green = "\033[42m";      // Green
const std::string on_Yellow = "\033[43m";     // Yellow
const std::string on_Blue = "\033[44m";       // Blue
const std::string on_Purple = "\033[45m";     // Purple
const std::string on_Cyan = "\033[46m";       // Cyan
const std::string on_White = "\033[47m";      // White

// High Intensity
const std::string iBlack = "\033[90m";      // Black
const std::string iRed = "\033[91m";        // Red
const std::string iGreen = "\033[92m";      // Green
const std::string iYellow = "\033[93m";     // Yellow
const std::string iBlue = "\033[94m";       // Blue
const std::string iPurple = "\033[95m";     // Purple
const std::string iCyan = "\033[96m";       // Cyan
const std::string iWhite = "\033[97m";      // White


// High Intensity backgrounds
const std::string on_IBlack = "\033[100m";  // Black
const std::string on_IRed = "\033[101m";    // Red
const std::string on_IGreen = "\033[102m";  // Green
const std::string on_IYellow = "\033[103m"; // Yellow
const std::string on_IBlue = "\033[104m";   // Blue
const std::string on_IPurple = "\033[105m"; // Purple
const std::string on_ICyan = "\033[106m";   // Cyan
const std::string on_IWhite = "\033[107m";  // White

	inline std::string addColor(uint32_t colorCode){
		if(colorCode >= 16 && colorCode <= 232){
			return "\033[38;5;" + std::to_string(colorCode) + "m";
		}else{
			std::cerr << black << bold << "bashColorText::addColor()"
					<< red << " Color code needs to be between 16 and 232" << reset << "\n";
			return "";
		}
	}

	inline std::string addBGColor(uint32_t colorCode){
		if(colorCode >= 16 && colorCode <= 232){
			return "\033[48;5;" + std::to_string(colorCode) + "m";
		}else{
			std::cerr << black << bold << "bashColorText::addBGColor()"
					<< red << " Color code needs to be between 16 and 232" << reset << "\n";
			return "";
		}
	}

	inline void outputPossibleColors(std::ostream & out){
		std::vector<uint32_t> possColors(216);
		std::iota(possColors.begin(), possColors.end(), 16U);
		for(auto i : possColors){
			out << addBGColor(i) << i << reset << "\n";
		}
	}

	inline std::string boldRed(const std::string & str){
		return red + bold + str + reset;
	}

	inline std::string boldGreen(const std::string & str){
		return green + bold + str + reset;
	}

	inline std::string boldBlack(const std::string & str){
		return black + bold + str + reset;
	}

} // namespace bashCT

} // namespace bibcpp
