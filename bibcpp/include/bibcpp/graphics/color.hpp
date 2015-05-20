#pragma once
/*
 * color.hpp
 *
 *  Created on: Nov 20, 2014
 *      Author: nickhathaway
 */

#include "bibcpp/utils/stringUtils.hpp" //hexToInt() intToHex()
#include "bibcpp/utils/utils.hpp" //roundDecPlaces()
#include "bibcpp/stdAddition.h" // to_string()
namespace bib {

/**@b A class for handling color manipulation and to output color info
 *
 */
class color {

 public:
  // Constructors
  /*! \brief Default Constructor to Black
   *
   *
   *  RGB initialed to all 0's so will be black
   */
  color() : red_(0), green_(0), blue_(0), alpha_(1), hexStr_("000000") {
    // default construct as black
    setHueSatLumVal();
  }

  /*! \brief Constructor by hex string
   *
   *
   *  Takes the 6 char hex string and sets color by that, should be
   *  standard length of six with first two letters be red, next two
   *  should be green, and next two should be blue
   */
  color(const std::string& hexStr) { setColorByHex(hexStr); }
  /*! \brief Constructor by RGB values
   *
   *
   *  Contruct with rgb color values, either range from 0 - 1 or 0 - 255
   *  will convert values given by dividing by maxNumber to create a number
   *between
   *  0 - 1
   */
  color(double red, double green, double blue, double maxNumber, double alpha = 1) {
    setColorByRGB(red, green, blue, maxNumber, alpha);
  }

  // memebers
  // rgb values for color, should range from 0 to 1 for level,
  // need to mutliple by 255 out to get the actual rgb values
  // recogized by must programs
  double red_;/**the red value in [0,1] */
  double green_;/**the green value in [0,1]  */
  double blue_;/**the blue value in [0,1]  */

  double alpha_;/**the alpha value in [0,1]  */

  double hue_;/**the hue value in [0,360]  */

  double vSat_;/**the saturation value in [0,1] for HSV  */
  double val_;/**the value in [0,1] for HSV */

  double lSat_;/**the saturation value in [0,1] for HSL */
  double lum_;/**the lum value in [0,1] for HSL */

  char cMaxChar_;
  // get max, min and change
  double cMax_;
  double cMin_;
  double change_;

  // hex string for colors
  std::string hexStr_; /**the hexstring representation of the color */
  // color maximum and min finders
  double colMax(double r, double g, double b, char& p) {
    if (r >= g && r >= b) {
      p = 'r';
      return r;
    } else if (g >= r && g >= b) {
      p = 'g';
      return g;
    } else {
      p = 'b';
      return b;
    }
  }
  double colMin(double r, double g, double b) {
    if (r <= g && r <= b) {
      return r;
    } else if (g <= r && g <= b) {
      return g;
    } else {
      return b;
    }
  }
  /*
   * this turning out to be more confusing than i thought it would be, i'll come
   * back to it latter nick h. 2014.06.23
  const static double colorGamma = 2.4;
  static double gtrans(double u, double gamma){
      if (u > 0.00304)
  	return 1.055 * std::pow(u, (1 / gamma)) - 0.055;
      else
  	return 12.92 * u;
  }

  static double ftrans(double u, double gamma){
      if (u > 0.03928)
  	return std::pow((u + 0.055) / 1.055, gamma);
      else
  	return u / 12.92;
  }
  static void deviceRgbToRgb(double devR, double devG, double devB,
  		double gamma,
  		double &r, double &g, double &b) {
		r = ftrans(devR, gamma);
		g = ftrans(devG, gamma);
		b = ftrans(devB, gamma);
  }

  static void rgbToDeviceRgb(double r, double g, double b, double gamma,
  			 double &devR, double &devG, double &devB){
		devR = gtrans(r, gamma);
		devG = gtrans(g, gamma);
		devB = gtrans(b, gamma);
  }*/
  // functions
  /**@b set the color with red,green,blue values, should range from [0,maxNumber]
   *
   * @param red the red value in [0,maxNumber]
   * @param green the green value in [0,maxNumber]
   * @param blue the blue value in [0,maxNumber]
   * @param maxNumber  The max number, usually 1 or 255
   * @param alpha the alpha value in [0,maxNumber]
   */
  void setColorByRGB(double red, double green, double blue,
                            double maxNumber, double alpha) {
    red_ = red / maxNumber;
    green_ = green / maxNumber;
    blue_ = blue / maxNumber;
    hexStr_ = intToHex(std::round(red_ * 255)) +
              intToHex(std::round(green_ * 255)) +
              intToHex(std::round(blue_ * 255));
    if(alpha > 1){
      alpha_ = alpha/maxNumber;
    }else{
      alpha_ = alpha;
    }
    if(roundDecPlaces(alpha_, 2) != 1.00){
    	hexStr_ += intToHex(std::round(alpha_*255));
    }

    setHueSatLumVal();
  }
  /**@b set the alpha value with the alpha in [0,maxValue]
   *
   * @param alpha
   * @param maxValue
   */
  void setAlpha(double alpha, double maxValue){
  	alpha_ = alpha/ maxValue;
  	if(hexStr_.length() == 8){
  		hexStr_.erase(6,2);
  		hexStr_ += intToHex(std::round(alpha_*255));
  	} else if (hexStr_.length() ==6){
  		hexStr_ += intToHex(std::round(alpha_*255));
  	}
  }
  /**@b set the color using the HSV values
   *
   * @param hue the hue value between [0,360] (hue can overflow but will be converted to [0,360]
   * @param sat the sat value between [0,1]
   * @param val the value between [0,1]
   */
  void setColorByHueSatVal(double hue, double sat, double val) {
    // hue 0 - 360 degrees, though it will be reduced for bigger degrees
    // sat 0 -1
    // val 0 -1
    hue_ = hue;
    vSat_ = sat;
    val_ = val;
    hue = std::fmod(hue, 360);
    double bigC = val * sat;
    double bigX = bigC * (1 - std::abs(std::fmod(hue / 60.0, 2.0) - 1));
    double littleM = val - bigC;
    // std::cout << bigC << std::endl;
    // std::cout << bigX << std::endl;
    // std::cout << littleM << std::endl;
    if (0 <= hue && hue < 60) {
      red_ = bigC + littleM;
      green_ = bigX + littleM;
      blue_ = 0 + littleM;
    } else if (60 <= hue && hue < 120) {
      red_ = bigX + littleM;
      green_ = bigC + littleM;
      blue_ = 0 + littleM;
    } else if (120 <= hue && hue < 180) {
      red_ = 0 + littleM;
      green_ = bigC + littleM;
      blue_ = bigX + littleM;
    } else if (180 <= hue && hue < 240) {
      red_ = 0 + littleM;
      green_ = bigX + littleM;
      blue_ = bigC + littleM;
    } else if (240 <= hue && hue < 300) {
      red_ = bigX + littleM;
      green_ = 0 + littleM;
      blue_ = bigC + littleM;
    } else if (300 <= hue && hue < 360) {
      red_ = bigC + littleM;
      green_ = 0 + littleM;
      blue_ = bigX + littleM;
    }
    hexStr_ = intToHex(std::round(red_ * 255)) +
              intToHex(std::round(green_ * 255)) +
              intToHex(std::round(blue_ * 255));

    setMaxMin();
    setLum();
  }
  /**@b set the color using the HSL values
   *
   * @param hue the hue value between [0,360] (hue can overflow but will be converted to [0,360]
   * @param sat the sat value between [0,1]
   * @param lum the lum between [0,1]
   */
  void setColorByHueSatLum(double hue, double sat, double lum) {
    // hue 0 - 360 degrees, though it will be reduced for bigger degrees
    // sat 0 - 1
    // lum 0 - 1
    hue = std::fmod(hue, 360);
    hue_ = hue;
    lSat_ = sat;
    lum_ = lum;
    double bigC = (1 - std::abs(2 * lum - 1)) * sat;
    double bigX = bigC * (1 - std::abs(std::fmod(hue / 60.0, 2.0) - 1));
    double littleM = lum - bigC / 2;
    // std::cout << bigC << std::endl;
    // std::cout << bigX << std::endl;
    // std::cout << littleM << std::endl;
    if (0 <= hue && hue < 60) {
      red_ = bigC + littleM;
      green_ = bigX + littleM;
      blue_ = 0 + littleM;
    } else if (60 <= hue && hue < 120) {
      red_ = bigX + littleM;
      green_ = bigC + littleM;
      blue_ = 0 + littleM;
    } else if (120 <= hue && hue < 180) {
      red_ = 0 + littleM;
      green_ = bigC + littleM;
      blue_ = bigX + littleM;
    } else if (180 <= hue && hue < 240) {
      red_ = 0 + littleM;
      green_ = bigX + littleM;
      blue_ = bigC + littleM;
    } else if (240 <= hue && hue < 300) {
      red_ = bigX + littleM;
      green_ = 0 + littleM;
      blue_ = bigC + littleM;
    } else if (300 <= hue && hue < 360) {
      red_ = bigC + littleM;
      green_ = 0 + littleM;
      blue_ = bigX + littleM;
    }
    hexStr_ = intToHex(std::round(red_ * 255)) +
              intToHex(std::round(green_ * 255)) +
              intToHex(std::round(blue_ * 255));
    setMaxMin();
    setVal();
  }
  /**@b set the color by giving the values of red, green, and blue with their hex string
   *
   * @param red The hexstring for red
   * @param green The hexstring for green
   * @param blue The hextring for blue
   */
  void setColorByHex(const std::string& red, const std::string& green,
                            const std::string& blue) {
    red_ = hexToInt(red) / 255.00;
    green_ = hexToInt(green) / 255.00;
    blue_ = hexToInt(blue) / 255.00;
    hexStr_ = red + green + blue;
    setHueSatLumVal();
  }
  /**@b Set the color by the full hexstring, can 6 or 8 (alpha value included) chars long and can start with # sign
   *
   * @param hexStr The hexstring
   */
  void setColorByHex(std::string hexStr) {
    if (hexStr.front() == '#') {
      hexStr.erase(hexStr.begin());
    }
    if (hexStr.length() != 6 && hexStr.length() != 8) {
      std::cout << "error, hex string for color should be six characters long or 8 characters long"
                << std::endl;
      std::cout << hexStr << std::endl;
    } else {
      setColorByHex(hexStr.substr(0, 2), hexStr.substr(2, 2),
                    hexStr.substr(4, 2));
      if(hexStr.length() == 8){
      	hexStr_ += hexStr.substr(6, 2);
      	alpha_ =  hexToInt(hexStr.substr(6, 2)) / 255.00;
      }else{
      	alpha_ = 1;
      }
      setHueSatLumVal();
    }
  }
  /**@b set the hue sat and lum vlues for the current red, green, and blue values
   *
   */
  void setHueSatLumVal() {

    // get max, min and change
    setMaxMin();
    // hue calc
    setHue();
    setLum();
    // lum and val calc
    setVal();
  }
  /**@b set the lum and sat values
   *
   */
  void setLum() {
    // lum sat calc
    lum_ = (cMax_ + cMin_) / 2;
    // sat calc
    if (change_ == 0) {
      lSat_ = 0;
    } else {
      lSat_ = change_ / (1 - std::abs(2 * lum_ - 1));
    }
  }
  /**@b set the value and sat values
   *
   */
  void setVal() {
    // val sat calc
    val_ = cMax_;

    if (cMax_ == 0) {
      vSat_ = 0;
    } else {
      vSat_ = change_ / cMax_;
    }
  }
  void setMaxMin() {
    cMax_ = colMax(red_, green_, blue_, cMaxChar_);
    cMin_ = colMin(red_, green_, blue_);
    change_ = cMax_ - cMin_;
  }
  /**@b calculate hue
   *
   */
  void setHue() {
    if (change_ == 0) {
      hue_ = 0;
    } else {
      if (cMaxChar_ == 'r') {
        hue_ = 60.0 * std::fmod((green_ - blue_) / change_, 6);
      } else if (cMaxChar_ == 'g') {
        hue_ = 60.0 * (((blue_ - red_) / change_) + 2);
      } else if (cMaxChar_ == 'b') {
        hue_ = 60.0 * (((red_ - green_) / change_) + 4);
      } else {
        std::cout << "this should not happen....alert someone" << std::endl;
      }
      if (hue_ < 0) {
        hue_ = hue_ + 360;
      }
    }
  }
  /**@b get the color in grayscale
   *
   * @return the grayscale equivalent for the color
   */
  color getGreyScaleColor() const {
    double averageRGB = (red_ + green_ + blue_) / 3;
    return color(averageRGB, averageRGB, averageRGB, 1);
  }
  /**@b get a made up distance of hsl
   *
   * @param otherColor the color to calc the distance from
   * @return the distance
   */
  double getHSLDistance(const color & otherColor)const{
  	double hueDiff = otherColor.hue_ - hue_;
  	if(std::abs(hueDiff) > 180){
  		hueDiff = std::abs(hueDiff) - 360;
  	}
  	hueDiff = hueDiff/180;
  	return std::sqrt(std::pow(hueDiff ,2) + std::pow(otherColor.lSat_ - lSat_ ,2) + std::pow(otherColor.lum_ - lum_ ,2));
  }
  /**@b the string value for red,green,blue in [0,255]
   *
   * @return a string with red,green,blue
   */
  std::string getRGBStr()const{
  	return estd::to_string(std::round(red_ * 255)) + ","
  			+ estd::to_string(std::round(green_*255)) + ","
  			+ estd::to_string(std::round(blue_*255));
  }
  /**@b print a description of the
   *
   * @param out
   * @param deep
   */
  void printDescription(std::ostream& out, bool deep) const {
    out << "color{" << std::endl << "red_:" << red_ << std::endl
        << "green_:" << green_ << std::endl << "blue_:" << blue_ << std::endl
        << "hue_:" << hue_ << std::endl << "vSat_:" << vSat_ << std::endl
        << "val_:" << val_ << std::endl << "lSat_:" << lSat_ << std::endl
        << "lum_:" << lum_ << std::endl << "cMaxChar_:" << cMaxChar_
        << std::endl << "cMax_:" << cMax_ << std::endl << "cMin_:" << cMin_
        << std::endl << "change_:" << change_ << std::endl
        << "hexStr_:" << hexStr_ << std::endl << "}" << std::endl;
  }
};



}  // namespace bib
