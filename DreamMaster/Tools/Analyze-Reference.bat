@echo off
setlocal
if "%~1"=="" (
  echo Drag an audio file onto Analyze-Reference.bat to analyze it.
  echo No audio is uploaded. Only a local JSON analysis is written.
  pause
  exit /b 1
)
if not exist "%~dp0DreamMasterAnalyze.exe" (
  echo DreamMasterAnalyze.exe is missing. Keep both files in the same folder.
  pause
  exit /b 1
)
set "output=%~dpn1.dreammaster-analysis.json"
if exist "%output%" (
  echo An analysis already exists. Existing files are never overwritten:
  echo "%output%"
  pause
  exit /b 1
)
"%~dp0DreamMasterAnalyze.exe" "%~f1" > "%output%"
if errorlevel 1 (
  del /q "%output%" 2>nul
  echo Analysis failed. Your source audio was not modified.
  pause
  exit /b 1
)
echo Analysis saved to:
echo "%output%"
type "%output%"
pause
