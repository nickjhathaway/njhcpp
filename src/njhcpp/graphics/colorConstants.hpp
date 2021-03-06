#pragma once
/*
 * colorConstants.hpp
 *
 *  Created on: Nov 20, 2014
 *      Author: nickhathaway
 */


#include <unordered_map>
#include <njhcpp/graphics/color.hpp>
namespace njh{

/**@brief Namespace to hold color constants
 */
namespace colorspace{




/**@brief Func to return a hand crafted conversion of letters to color codes
 *
 * @return An unordered_map of char to color code
 */
inline std::unordered_map <char, uint32_t> createLettersToAnsiCoode() {
		return  { {'A',160},
			{	'B',178},
			{	'C',112},
			{	'D',41},
			{	'E',44},
			{	'F',26},
			{	'G',92},
			{	'H',163},
			{	'I',196},
			{	'J',214},
			{	'K',190},
			{	'L',82},
			{	'M',48},
			{	'N',51},
			{	'O',33},
			{	'P',57},
			{	'Q',165},
			{	'R',199},
			{	'S',131},
			{	'T',107},
			{	'U',73},
			{	'V',97},
			{	'W',167},
			{	'X',185},
			{	'Y',77},
			{	'Z',80},
			{	'a',62},
			{	'b',170},
			{	'c',174},
			{	'd',150},
			{	'e',116},
			{	'f',140},
			{	'g',203},
			{	'h',221},
			{	'i',155},
			{	'j',84},
			{	'k',87},
			{	'l',69},
			{	'm',135},
			{	'n',206},
			{	'o',181},
			{	'p',152},
			{	'q',210},
			{	'r',228},
			{	's',120},
			{	't',123},
			{	'u',105},
			{	'v',213},
			{	'w',217},
			{	'x',193},
			{	'y',159},
			{	'z',183}
		};
		//return ret;
}

/**@brief Func to create njh::color objects for all ansi color codes
 *
 * @return An unordered_map with color code to njh::color class for that color code
 */
inline std::unordered_map <uint32_t, color> createAnsiColorCodeToColor() {
	std::vector<uint32_t> nums {0,95,135,175, 215, 255};
	uint32_t ansi = 16;
	std::unordered_map <uint32_t, color> ret;
	for(const auto & num1 : nums){
		for(const auto & num2 : nums){
			for(const auto & num3 : nums){
				ret.emplace(ansi,color(num1,num2,num3,255));
				++ansi;
			}
		}
	}
	return ret;
}



static const std::vector<std::string> colorBindPalette{"#006E82","#AA0A3C","#8214A0","#FA7850","#005AC8","#0AB45A","#00A0FA","#F0F032","#FA78FA","#A0FA82","#14D2DC","#FAE6BE"};



} //namespace colorspace
} //namespace njh
