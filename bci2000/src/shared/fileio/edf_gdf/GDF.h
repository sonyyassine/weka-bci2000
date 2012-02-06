////////////////////////////////////////////////////////////////////////////////
// $Id: GDF.h 1723 2008-01-16 17:46:33Z mellinger $
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A C++ representation of a BCI2000 relevant subset of the EDF
//              data format as defined in Kemp et al, 1992, and the
//              GDF 1.25 data format as defined in Schloegl et al, 1998.
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef GDF_H
#define GDF_H

#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <sstream>
#include <fstream>

namespace GDF
{
  template<typename T, int bytesize, int code>
  struct GDFType
  {
    typedef T ValueType;
    enum { Code = code, SizeAssertion = ( bytesize == sizeof( T ) ) };
    // The bit field declaration becomes illegal when type and size don't match.
    struct { unsigned int bf : SizeAssertion; };
  };
  static const bool BigEndianMachine =
    ( *reinterpret_cast<const unsigned short*>( "\0\1" ) == 0x01 );

  // Schloegl et al, 1998, Table 1.
  //               Type             Size  Code
  typedef GDFType< signed char,        1, 1  > int8;
  typedef GDFType< unsigned char,      1, 2  > uint8;
  typedef GDFType< signed short,       2, 3  > int16;
  typedef GDFType< unsigned short,     2, 4  > uint16;
  typedef GDFType< signed int,         4, 5  > int32;
  typedef GDFType< unsigned int,       4, 6  > uint32;
  typedef GDFType< signed long long,   8, 7  > int64;
  typedef GDFType< unsigned long long, 8, 8  > uint64;

  typedef GDFType< float,              4, 16 > float32;
  typedef GDFType< double,             8, 17 > float64;

  const long long cInvalidDate = - ( 1LL << 62 );
  const long long cSecondsPerYear = ( 60 * 60 * 24 * 3652422LL ) / 10000;

  enum
  {
    unspecified = 0,
    // FileFormat
    EDF = 1,
    GDF = 2,
    // Sex
    male = 1,
    female = 2,
  };

  // An EncodedString replaces white space by underscores when written to a stream.
  class EncodedString : public std::string
  {
   public:
    EncodedString( const std::string& s = "" ) : std::string( s ) {}
    void WriteToStream( std::ostream& os ) const;
  };

  // Conversion of an extended time_t type into a string.
  std::string DateTimeToString( signed long long );

  // Field types as classes.
  template<int length> // A fixed-length string field.
    class Str : public std::string
    {
     public:
      Str( const std::string& = "" );
      Str( const char* );
      Str( double );
      void WriteToStream( std::ostream& ) const;
    };

  template<class T>
    class Num // A numeric field with a binary representation.
    {
     public:
      Num( typename T::ValueType = 0 );
      void WriteToStream( std::ostream& os ) const;
     private:
      typename T::ValueType mValue;
    };

  template<class F>
    void PutField( std::ostream& );

  template<class F, class V>
    void PutField( std::ostream&, const V& v );

  template<class F, class C>
    void PutArray( std::ostream&, const C& c );

  template<class F, class C, class P>
    void PutArray( std::ostream&, const C& c, P p );

}; // namespace GDF


template<class F>
inline
void
GDF::PutField( std::ostream& os )
{
  F().WriteToStream( os );
}

template<class F, class V>
inline
void
GDF::PutField( std::ostream& os, const V& v )
{
  F( v ).WriteToStream( os );
}

template<class F, class C>
inline
void
GDF::PutArray( std::ostream& os, const C& c )
{
  for( typename C::const_iterator i = c.begin(); i != c.end(); ++i )
    F().WriteToStream( os );
}

template<class F, class C, class P>
inline
void
GDF::PutArray( std::ostream& os, const C& c, P p )
{
  for( typename C::const_iterator i = c.begin(); i != c.end(); ++i )
    F( ( *i ).*p ).WriteToStream( os );
}

template<int tLength>
GDF::Str<tLength>::Str( const std::string& s )
: std::string( s )
{
  resize( tLength, ' ' );
}

template<int tLength>
GDF::Str<tLength>::Str( const char* s )
: std::string( s )
{
  resize( tLength, ' ' );
}

template<int tLength>
GDF::Str<tLength>::Str( double d )
{
  std::ostringstream oss;
  oss << d;
  int prec = tLength;
  while( oss.str().length() > tLength && prec > 0 )
  {
    oss.str( "" );
    oss << std::scientific << std::setprecision( prec ) << d;
    --prec;
  }
  *this = oss.str();
  resize( tLength, ' ' );
}

template<int tLength>
void
GDF::Str<tLength>::WriteToStream( std::ostream& os ) const
{
  os.write( this->data(), tLength );
}

template<class T>
GDF::Num<T>::Num( typename T::ValueType t )
: mValue( t )
{
}

template<class T>
void
GDF::Num<T>::WriteToStream( std::ostream& os ) const
{
  const char* value = reinterpret_cast<const char*>( &mValue );
  if( BigEndianMachine )
    for( int i = sizeof( typename T::ValueType ) - 1; i >= 0; --i )
      os.put( value[ i ] );
  else
    for( size_t i = 0; i < sizeof( typename T::ValueType ); ++i )
      os.put( value[ i ] );
}

inline
std::ostream&
operator<<( std::ostream& os, const GDF::EncodedString& s )
{
  s.WriteToStream( os );
  return os;
}

#endif // GDF_H
