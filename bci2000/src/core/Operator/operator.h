/* (C) 2000-2008, BCI2000 Project
/* http://www.bci2000.org
/*/
#ifndef OPERATOR_DEFINE
#define OPERATOR_DEFINE

#define INIFILENAME_PARAMETERS  "parameter.ini"
#define INIFILENAME_PREFERENCES "preferences.ini"

enum {
  ERR_NOERR = 0,
  
  ERR_STATENOTFOUND,
  ERR_SOURCENOTCONNECTED,

  ERR_MATLOADCOLSDIFF,
  ERR_MATNOTFOUND,
  ERR_COULDNOTWRITE,
};

#endif

