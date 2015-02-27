@echo off
make.exe %1
if "%1"=="clean" goto END

if errorlevel 1 goto END
type *.mem

packihx VaporB72.ihx > VaporB72.hex
if errorlevel 1 goto END

convert2.vbs VaporB72.hex

if NOT EXIST "c:\program files\VaporWorks\Vlink V2.0" goto END

copy VaporB72.efw "c:\program files\VaporWorks\Vlink V2.0"
:END
