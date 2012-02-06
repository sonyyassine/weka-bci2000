////////////////////////////////////////////////////////////////////////////////
// $Id: CoreModuleVCL.cpp 1723 2008-01-16 17:46:33Z mellinger $
// Author: juergen.mellinger@uni-tuebingen.de
// Description: A class that implements the CoreModule GUI interface functions
//          for VCL-based modules, and an appropriate WinMain function.
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include <vcl.h>
#include "CoreModuleVCL.h"

WINAPI
WinMain( HINSTANCE, HINSTANCE, LPSTR, int )
{
  try
  {
    Application->Initialize();
    CoreModuleVCL().Run( _argc, _argv );
  }
  catch (Exception &exception)
  {
    Application->ShowException(&exception);
  }
  return 0;
}
