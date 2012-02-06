:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:: $Id: FeedbackDemo_SignalGenerator.bat 1719 2008-01-16 15:40:24Z mellinger $
:: Description: BCI2000 startup script for the WinNT shell.
::
:: (C) 2000-2007, BCI2000 Project
:: http://www.bci2000.org
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
cd ..\prog
start operat.exe
start SignalGenerator.exe 127.0.0.1
start ARSignalProcessing.exe 127.0.0.1
start FeedbackDemo.exe 127.0.0.1
cd ..