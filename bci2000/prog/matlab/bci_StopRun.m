function bci_StopRun

% Filter stop run demo
% 
% Perform parameter updates at the end of a run.

% BCI2000 filter interface for Matlab
% juergen.mellinger@uni-tuebingen.de, 2005
% (C) 2000-2008, BCI2000 Project
% http://www.bci2000.org

% Parameters and states are global variables.
global bci_Parameters bci_States;

runCount = str2double( bci_Parameters.MyRunCount{ 1, 1 } );
runCount = runCount + 1;
bci_Parameters.MyRunCount{ 1, 1 } = num2str( runCount );
