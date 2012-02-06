/////////////////////////////////////////////////////////////////////////////
// $Id: MeasurementUnits.h 1731 2008-01-20 17:26:38Z mellinger $
// Author: juergen.mellinger@uni-tuebingen.de
// Description: Utilities for handling measurement units and conversions.
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
//////////////////////////////////////////////////////////////////////////////
#ifndef MEASUREMENT_UNITS_H
#define MEASUREMENT_UNITS_H

#include "PhysicalUnit.h"
#include <string>

// This class converts strings such as "123.3" or "12ms" to plain numbers that
// represent values in global units.
class MeasurementUnits
{
  public:
    static float ReadAsTime( const std::string& value )
    { return sTimeUnit.PhysicalToRaw( value ); }
    static void InitializeTimeUnit( double inUnitsPerSec )
    { sTimeUnit.SetOffset( 0 ).SetGain( 1.0 / inUnitsPerSec ).SetSymbol( "s" ); }

    static float ReadAsFreq( const std::string& value )
    { return sFreqUnit.PhysicalToRaw( value ); }
    static void InitializeFreqUnit( double inUnitsPerHertz )
    { sFreqUnit.SetOffset( 0 ).SetGain( 1.0 / inUnitsPerHertz ).SetSymbol( "Hz" ); }

    static float ReadAsVoltage( const std::string& value )
    { return sVoltageUnit.PhysicalToRaw( value ); }
    static void InitializeVoltageUnit( double inUnitsPerVolt )
    { sVoltageUnit.SetOffset( 0 ).SetGain( 1.0 / inUnitsPerVolt ).SetSymbol( "V" ); }

  private:
    static PhysicalUnit sTimeUnit;
    static PhysicalUnit sFreqUnit;
    static PhysicalUnit sVoltageUnit;
};

#endif // MEASUREMENT_UNITS_H
