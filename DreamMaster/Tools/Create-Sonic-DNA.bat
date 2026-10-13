@echo off
setlocal
if "%~2"=="" (
 echo Drag at least two DIFFERENT .dreammaster-analysis.json files onto this batch file.
 echo Generate each analysis using Analyze-Reference.bat first.
 pause
 exit /b 1
)
if not exist "%~dp0DreamMasterProfile.exe" (
 echo DreamMasterProfile.exe is missing. Keep it in the same folder.
 pause
 exit /b 1
)
set "output=%~dp0DreamMaster-SonicDNA.json"
if exist "%output%" (
 echo A Sonic DNA profile already exists. It will not be overwritten:
 echo "%output%"
 pause
 exit /b 1
)
"%~dp0DreamMasterProfile.exe" %* > "%output%"
if errorlevel 1 (
 del /q "%output%" 2>nul
 echo Sonic DNA profile creation failed. Your audio and reference reports are unchanged.
 pause
 exit /b 1
)
echo Profile created: "%output%"
type "%output%"
pause
