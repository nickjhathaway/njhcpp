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
/**@brief Namespace to hold ansi escape character for changing the text on the terminal common
 * See [here](http://misc.flogisoft.com/bash/tip_colors_and_formatting) for details on the different color and text modifying affects
 *
 */
namespace bashCT {
	// Reset
const std::string reset = "\033[0m";   /**< Text Reset*/

/**@brief Reset the text but then add any modifiers that you want to keep
 *
 * @param add A vector of strings of text modifying
 * @return return reset code plus add
 */
inline std::string resetAdd(const std::vector<std::string>& add){
	return reset + conToStr(add, "");
}
/**@brief Reset the text but then add a paramter you want to keep, for ex. reset to reset color but keep bold
 *
 * @param add A string to modify the text
 * @return return reset code plus add
 */
inline std::string resetAdd(std::string add){
	return reset + add;
}

// Regular Colors

const std::string black = "\033[30m"; /**< black text*/
const std::string red =   "\033[31m"; /**< red text*/
const std::string green = "\033[32m"; /**< green text*/
const std::string yellow ="\033[33m"; /**< yellow text*/
const std::string blue =  "\033[34m"; /**< blue text*/
const std::string purple ="\033[35m"; /**< purple text*/
const std::string cyan =  "\033[36m"; /**< cyan text*/
const std::string white = "\033[37m"; /**< white text*/

// affecting by adding bold, underline, or flashing text
const std::string bold =     "\033[1m"; /**< bold text*/
const std::string underline ="\033[4m"; /**< underlined text*/
const std::string flashing = "\033[5m"; /**< flashing text*/
// affect the color of the text by decreasing saturation or by inverting/switching background and text colors
const std::string lighten =  "\033[2m"; /**< decrease saturation of text*/
const std::string invert =   "\033[7m"; /**< invert background and text colors*/
// Background
const std::string on_Black =  "\033[40m"; /**< Black background*/
const std::string on_Red =    "\033[41m"; /**< Red background*/
const std::string on_Green =  "\033[42m"; /**< Green background*/
const std::string on_Yellow = "\033[43m"; /**< Yellow background*/
const std::string on_Blue =   "\033[44m"; /**< Blue background*/
const std::string on_Purple = "\033[45m"; /**< Purple background*/
const std::string on_Cyan =   "\033[46m"; /**< Cyan background*/
const std::string on_White =  "\033[47m"; /**< White background*/

// High Intensity
const std::string iBlack =  "\033[90m"; /**< Black high intensity text*/
const std::string iRed =    "\033[91m"; /**< Red high intensity text*/
const std::string iGreen =  "\033[92m"; /**< Green high intensity text*/
const std::string iYellow = "\033[93m"; /**< Yellow high intensity text*/
const std::string iBlue =   "\033[94m"; /**< Blue high intensity text*/
const std::string iPurple = "\033[95m"; /**< Purple high intensity text*/
const std::string iCyan =   "\033[96m"; /**< Cyan high intensity text*/
const std::string iWhite =  "\033[97m"; /**< White high intensity text*/


// High Intensity backgrounds
const std::string on_IBlack =  "\033[100m"; /**< Black high intensity background*/
const std::string on_IRed =    "\033[101m"; /**< Red high intensity background*/
const std::string on_IGreen =  "\033[102m"; /**< Green high intensity background*/
const std::string on_IYellow = "\033[103m"; /**< Yellow high intensity background*/
const std::string on_IBlue =   "\033[104m"; /**< Blue high intensity background*/
const std::string on_IPurple = "\033[105m"; /**< Purple high intensity background*/
const std::string on_ICyan =   "\033[106m"; /**< Cyan high intensity background*/
const std::string on_IWhite =  "\033[107m"; /**< White high intensity background*/

/**the regex pattern representing the bash terminal formating*/
const std::regex formatPattern {"\\\033\\[\\d+m"};

/**@brief Change the terminal text color to the one of the 216 available colors
 *
 * @param colorCode A number between 16 and 232 to indicate the color to be used
 * @return A string with the escape text code for this color
 */
inline std::string addColor(uint32_t colorCode) {
	if (colorCode >= 16 && colorCode <= 232) {
		return "\033[38;5;" + std::to_string(colorCode) + "m";
	} else {
		std::cerr << black << bold << "bashColorText::addColor()" << red
				<< " Color code needs to be between 16 and 232" << reset << "\n";
		return "";
	}
}

/**@brief Change the background terminal color, similar to the
 *
 * @param colorCode A number between 16 and 232 to indicate the color to be used
 * @return A string with the escape text code for this background color
 */
inline std::string addBGColor(uint32_t colorCode) {
	if (colorCode >= 16 && colorCode <= 232) {
		return "\033[48;5;" + std::to_string(colorCode) + "m";
	} else {
		std::cerr << black << bold << "bashColorText::addBGColor()" << red
				<< " Color code needs to be between 16 and 232" << reset << "\n";
		return "";
	}
}

/**@brief Print out the possible colors between 16 and 232 for demonstration
 *
 * @param out The out stream to print to, should be the terminal or something that understand these escape character codes
 */
inline void outputPossibleColors(std::ostream & out) {
	std::vector<uint32_t> possColors(216);
	std::iota(possColors.begin(), possColors.end(), 16U);
	for (auto i : possColors) {
		out << addBGColor(i) << i << reset << "\n";
	}
}

/**@brief Short hand to add bold red text and then reset so it doesn't affect the rest of the text
 *
 * @param str The string to turn into bold red text
 * @return The string given by str but now with the bold red text code and the reset code
 */
inline std::string boldRed(const std::string & str) {
	return red + bold + str + reset;
}
/**@brief Short hand to add bold green text and then reset so it doesn't affect the rest of the text
 *
 * @param str The string to turn into bold red text
 * @return The string given by str but now with the bold red text code and the reset code
 */
inline std::string boldGreen(const std::string & str) {
	return green + bold + str + reset;
}

/**@brief Short hand to add bold black text and then reset so it doesn't affect the rest of the text
 *
 * @param str The string to turn into bold red text
 * @return The string given by str but now with the bold red text code and the reset code
 */
inline std::string boldBlack(const std::string & str) {
	return black + bold + str + reset;
}

/**@brief Get an actual length the string will appear on the terminal
 *
 * @param str The string to get the length from
 * @return The printing length
 */
inline uint32_t getPrintLen(const std::string & str) {

	std::smatch match;
	std::regex_search(str, match, formatPattern);
	uint32_t matchLen = match.length();
	std::string strMatch = match.suffix().str();
	while (std::regex_search(strMatch, match, formatPattern)) {
		strMatch = match.suffix().str();
		matchLen += match.length();
	}
	return str.length() - matchLen;
}

/**@brief remove the terminal formatting for printing to files and such
 *
 * @param str the string to trim
 * @return A string with no formating
 */
inline std::string trimForNonTerminalOut(std::string str){
	return std::regex_replace(str, formatPattern, "");
}

} // namespace bashCT

/**@brief Return a string with the text centered for the max width of the line
 * @todo Check that text size is shorter than maxWidth
 * @param text The text to center
 * @param maxWidth The max width of the line
 * @return A new string with the text centered for the maxWidth
 */
inline std::string centerText(const std::string& text, uint32_t maxWidth) {
  uint32_t halfWay = round(maxWidth / 2.0);
  uint32_t halfText = round(bashCT::getPrintLen(text) / 2.0);
  return std::string(halfWay - halfText, ' ') + text;
}

/**@brief Convert a bool to "true" in green text or "false" in red text for easier viewing
 *
 * @param b The bool to convert
 * @return A string with "true" in green text or "false" in red text
 */
inline std::string colorBool(bool b){
	std::string ret = "";
	if(b){
		ret = bashCT::boldGreen(bib::boolToStr(b));
	}else{
		ret = bashCT::boldRed(bib::boolToStr(b));
	}
	return ret;
};

} // namespace bib
