////////////////////////////////////////////////////////////////////////////////
// $Id: SysCommand.cpp 1723 2008-01-16 17:46:33Z mellinger $
// Authors: schalk@wadsworth.org, juergen.mellinger@uni-tuebingen.de
// Description: BCI2000 type for system commands.
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "SysCommand.h"

#include <string>
#include <iostream>

using namespace std;

const SysCommand SysCommand::EndOfState( "EndOfState" );
const SysCommand SysCommand::EndOfParameter( "EndOfParameter" );
const SysCommand SysCommand::Start( "Start" );
const SysCommand SysCommand::Reset( "Reset" );
const SysCommand SysCommand::Suspend( "Suspend" );


ostream&
SysCommand::WriteToStream( ostream& os ) const
{
  string::const_iterator p = mBuffer.begin();
  while( p != mBuffer.end() )
  {
    if( *p == '}' )
      os.put( '\\' );
    os.put( *p++ );
  }
  return os;
}

istream&
SysCommand::ReadBinary( istream& is )
{
  return std::getline( is, mBuffer, '\0' );
}

ostream&
SysCommand::WriteBinary( ostream& os ) const
{
  os.write( mBuffer.data(), mBuffer.size() );
  os.put( '\0' );
  return os;
}

bool
SysCommand::operator<( const SysCommand& s ) const
{
  return mBuffer < s.mBuffer;
}

bool
SysCommand::operator==( const SysCommand& s ) const
{
  return mBuffer == s.mBuffer;
}
