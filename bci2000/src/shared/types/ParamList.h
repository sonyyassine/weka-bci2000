////////////////////////////////////////////////////////////////////////////////
// $Id: ParamList.h 1723 2008-01-16 17:46:33Z mellinger $
// Authors: gschalk@wadsworth.org, juergen.mellinger@uni-tuebingen.de
// Description: ParamList is a BCI2000 type that represents a collection of
//   parameters.
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef PARAM_LIST_H
#define PARAM_LIST_H

#include <string>
#include <vector>
#include <map>
#include "Param.h"
#include "EncodedString.h"

class ParamList
{
 public:
        Param&  operator[]( const std::string& name );
  const Param&  operator[]( const std::string& name ) const;
        Param&  operator[]( size_t index )
                { return mParams.at( index ).Param; }
  const Param&  operator[]( size_t index ) const
                { return mParams.at( index ).Param; }

        int     Size() const
                { return mParams.size(); }
        bool    Empty() const
                { return mParams.empty(); }
        void    Clear();

        bool    Exists( const std::string& name ) const
                { return mNameIndex.find( name ) != mNameIndex.end(); }
        void    Add( const Param& p, float sortingHint = 0.0 );
        bool    Add( const std::string& paramDefinition );
        void    Delete( const std::string& name );

        bool    Save( const std::string& filename ) const;
        bool    Load( const std::string& filename,
                      bool importNonexisting = true );

        void    Sort();

  // These contain all formatted I/O functionality.
        std::ostream& WriteToStream( std::ostream& ) const;
        std::istream& ReadFromStream( std::istream& );

  // These define binary I/O.
        std::ostream& WriteBinary( std::ostream& ) const;
        std::istream& ReadBinary( std::istream& );

 private:
        void    RebuildIndex();

  struct ParamEntry
  {
    ParamEntry()
      : SortingHint( 0.0 )
      {}
    class Param Param;
    float SortingHint;
    static bool Compare( const ParamEntry& p, const ParamEntry& q )
      { return p.SortingHint < q.SortingHint; }
  };
  typedef std::vector<ParamEntry> ParamContainer;
  ParamContainer mParams;

  typedef std::map<std::string, int, Param::NameCmp> NameIndex;
  NameIndex mNameIndex;
};


inline
std::ostream& operator<<( std::ostream& s, const ParamList& p )
{ return p.WriteToStream( s ); }

inline
std::istream& operator>>( std::istream& s, ParamList& p )
{ return p.ReadFromStream( s ); }

#endif // PARAM_LIST_H

