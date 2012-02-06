////////////////////////////////////////////////////////////////////////////////
// $Id: LogFile.h 1723 2008-01-16 17:46:33Z mellinger $
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A std::ofstream descendant that centralizes/encapsulates details
//   of a log file.
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#ifndef LOG_FILE_H
#define LOG_FILE_H

#include <fstream>
#include <string>
#include "Environment.h"

class LogFile: public std::ofstream, private EnvironmentExtension
{
  public:
    LogFile()
      : mExtension( ".log" )
      {}
    explicit LogFile( const std::string& fileExtension )
      : mExtension( fileExtension )
      {}

  private:
    LogFile( const LogFile& );
    LogFile& operator=( const LogFile& );

  public:
    void Publish()
      {}
    void Preflight() const;
    void Initialize()
      {}
    void StartRun();
    void Process()
      {}

  private:
    std::string FilePath() const;

    std::string mExtension;
};

#endif // LOG_FILE_H
