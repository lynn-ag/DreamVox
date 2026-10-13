@echo off
setlocal
if "%~1"=="" (
 echo Drag a .dreammaster-analysis.json file onto Compare-Sonic-DNA.bat.
 echo Create DreamMaster-SonicDNA.json with Create-Sonic-DNA.bat first.
 pause
 exit /b 1
)
if not exist "%~dp0DreamMasterMatch.exe" (
 echo DreamMasterMatch.exe is missing from the tools folder.
 pause
 exit /b 1
)
if not exist "%~dp0DreamMaster-SonicDNA.json" (
 echo DreamMaster-SonicDNA.json must be beside the tools.
 pause
 exit /b 1
)
set "output=%~dpn1.dreammaster-match.json"
if exist "%output%" (
 echo Existing analysis is not overwritten: "%output%"
 pause
 exit /b 1
)
"%~dp0DreamMasterMatch.exe" "%~dp0DreamMaster-SonicDNA.json" "%~f1" > "%output%"
if errorlevel 1 (
 del /q "%output%" 2>nul
 echo Comparison failed; source audio and reference reports are unchanged.
 pause
 exit /b 1
)
echo Read-only match analysis saved to "%output%"
type "%output%"
pause
