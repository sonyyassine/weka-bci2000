//////////////////////////////////////////////////////////////////////////////////////
//
// File:        WindowingFunction.h
//
// Author:      juergen.mellinger@uni-tuebingen.de
//
// Description: A class that encapsulates details about windowing functions
//              used for sidelobe suppression in spectral analysis.
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
//////////////////////////////////////////////////////////////////////////////////////
#ifndef WindowingFunctionH
#define WindowingFunctionH

class WindowingFunction
{
  // Class-related information.
  public:
    typedef float NumType;
    typedef enum
    {
      None = 0,
      Hamming,
      Hann,
      Blackman,

      NumWindows
    } Window;
    static const char* WindowNames( int i );

  private:
    static const struct WindowProperties
    {
      Window      mWindow;
      const char* mName;
      NumType     ( *mComputeValue )( NumType );
    } sWindowProperties[];
    static NumType ComputeNone( NumType );
    static NumType ComputeHamming( NumType );
    static NumType ComputeHann( NumType );
    static NumType ComputeBlackman( NumType );

  // Instance-related information.
  public:
    WindowingFunction();
    WindowingFunction( int i );
    const char* Name() const;
    NumType Value( NumType ) const; // The argument specifies the position in the window.
                                    // It must be in the [0.0 ... 1.0[ range.
  private:
    Window mWindow;
};

#endif // WindowingFunctionH
