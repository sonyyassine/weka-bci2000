:: $Id: install.cmd 1451 2007-07-24 15:27:44Z mellinger $
:: Additional script for installation of support files.
:: This script gets called with the binary target directory as its
:: argument.

@if [%1]==[] goto error

@goto doit

:error
@echo The install script takes a target directory as an argument.
@goto end

:doit
:: Put your installation actions here.
copy /Y BioRadio150_SDK\BioRadio150DLL.dll %1\

:end
