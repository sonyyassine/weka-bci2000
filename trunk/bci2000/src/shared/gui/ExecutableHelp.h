////////////////////////////////////////////////////////////////////////////////
// $Id: ExecutableHelp.h 1803 2008-02-21 20:32:15Z mellinger $
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A class that displays an executable's associated
//   help file.
//   The help file is a html file that has the same name as the executable,
//   except that it bears a .html extension.
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef EXECUTABLE_HELP_H
#define EXECUTABLE_HELP_H

#include <string>
#include <map>

class ExecutableHelp
{
 public:
  ExecutableHelp( int );
  bool Display() const;

  class HelpMap : public std::multimap<std::string, std::string>
  {
   public:
    void Clear()
      { this->clear(); }
    bool Empty() const
      { return this->empty(); }
    void Add( const std::string& key, const std::string& value )
      { this->insert( make_pair( key, value ) ); }
    bool Exists( const std::string& s ) const
      { return this->find( s ) != end(); }
    void SetPath( const std::string& s )
      { mPath = s; }
    bool Open( const std::string& inKey, const std::string& inContext = "" ) const;

   private:
    std::string mPath;
  };
  const HelpMap& ParamHelp() const
    { return mParamHelp; }
  const HelpMap& StateHelp() const
    { return mStateHelp; }

 private:
  void Initialize();
  void InitializeContextHelp();

  HelpMap mParamHelp,
          mStateHelp;

  std::string mHelpFile,
              mHelpFileDir;
};

const class ExecutableHelp& ExecutableHelp();

#endif // EXECUTABLE_HELP_H
