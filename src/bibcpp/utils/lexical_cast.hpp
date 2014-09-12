#pragma once
/*
 * lexical_cast.hpp
 *
 *  Created on: Sep 12, 2014
 *      Author: nickhathaway
 */


#include <typeinfo> //std::bad_cast, typeid, std::type_info
#include <string>

#include "bibcpp/stdAddition/misc.hpp" //to_string, isArithmetic, isString

namespace bib {

/*Lexical cast adapted from zi_lib lexical cast implementation but adapted for
 * conversion of strings in scientific to arithmetic type -nick
 *
 */

/**@brief Exception for bad lexical cast
 *
 */
class bad_lexical_cast: public std::bad_cast
{
private:
    const std::type_info *source_type_;
    const std::type_info *target_type_;

public:
    bad_lexical_cast():
        source_type_( &typeid( void )),
        target_type_( &typeid( void ))
    {
    }

    bad_lexical_cast( const std::type_info &source_type,
                      const std::type_info &target_type ):
        source_type_( &source_type),
        target_type_( &target_type)
    {
    }

    const std::type_info &source_type() const
    {
      return *source_type_;
    }

    const std::type_info &target_type() const
    {
      return *target_type_;
    }

    virtual const char *what() const throw()
    {
      return "bad lexical cast: "
          "source type value could not be interpreted as target";
    }

    virtual ~bad_lexical_cast() throw()
    {
    }

};


/**@brief Most likely won't be called, just so compilier doesn't complain about std::stod for non-string types
 *
 */
template< class Source>
inline std::string toSreamHelper(const Source & source){
	return to_string(source);
}

/**@brief For lexical cast to convert scientific notation properly
 *
 */
inline std::string toSreamHelper(const std::string & source){
	return estd::to_string(std::stod(source));
}

/**@brief Caster for to anything not a string, with a check for if it is a cast from
 * a string to a arithmetic type to help with scientific notation
 *
 *
 */
template< class Target, class Source >
struct lexical_caster
{
  static inline Target cast_it( const Source& source ) {
    Target ret;
    std::stringstream ss;
    if(estd::isArithmetic<Target>() && estd::isString<Source>()) {
    	ss << toSreamHelper(source);
    } else {
    	ss << source;
    }
    if ( ss && ss >> ret ){
      if ( ss.eof() ){
      	return ret;
      }
    }
    throw bad_lexical_cast( typeid( Source ), typeid( Target ) );
  }
};

/**@brief Caster for anything to string, just take the value of the stream
 *
 */
template< class Source >
struct lexical_caster< std::string, Source >
{
  static inline std::string cast_it( const Source& source )
  {
    std::ostringstream oss;
    if ( oss << source ){
    	return oss.str();
    }
    throw bad_lexical_cast( typeid( Source ), typeid( std::string ) );
  }
};


/**@b The actual function to call to cast
 *
 * @param source What's to be cast
 * @return the cast of source to type Target
 */
template< class Target, class Source >
Target lexical_cast( const Source& source )
{
	return lexical_caster< Target, Source >::cast_it( source );
}


/**@b Lexical cast to do entire container
 *
 * @param con The container of elements to be cast
 * @return A new container with the cast values
 */
template<typename SourceCon, typename OutCon>
OutCon lexical_cast_con(const SourceCon & con){
	OutCon ret(con.size());
	std::transform(con.begin(), con.end(), ret.begin(),
			[](const typename SourceCon::value_type & e){
		return lexical_cast<typename OutCon::value_type,typename SourceCon::value_type>(e);});
	return ret;
}

}  // namespace bib
