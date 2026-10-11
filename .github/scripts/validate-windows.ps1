param([switch]$ExpectEditorCrash)
$ErrorActionPreference = 'Stop'
$PSNativeCommandUseErrorActionPreference = $false
New-Item -ItemType Directory -Path validation -Force | Out-Null
Invoke-WebRequest 'https://github.com/Tracktion/pluginval/releases/download/v1.0.4/pluginval_Windows.zip' -OutFile pluginval.zip
$expectedHash = 'C08E61CE3B96DB41636F8EC7E76F4C7E2C13EBDAC7FA1B5A1F52B4F32EC715AB'
if ((Get-FileHash pluginval.zip -Algorithm SHA256).Hash -ne $expectedHash) { throw 'pluginval archive checksum mismatch' }
Expand-Archive pluginval.zip -DestinationPath pluginval -Force
$validator = (Get-ChildItem pluginval -Filter pluginval.exe -Recurse | Select-Object -First 1).FullName
if (!$validator) { throw 'pluginval.exe not found' }
$bundle = (Resolve-Path 'build/DreamVox_artefacts/Release/VST3/DreamVox.vst3').Path
$repeats = if ($ExpectEditorCrash) { 1 } else { 3 }
# GUI tests are deliberately enabled: editor construction caused the regression.
& $validator --strictness-level 5 --repeat $repeats --random-seed 12345 --timeout-ms 60000 --validate $bundle 2>&1 | Tee-Object validation/pluginval.txt
$result = $LASTEXITCODE
"Exit code: $result" | Tee-Object validation/exit-code.txt
$log = Get-Content validation/pluginval.txt -Raw
if ($ExpectEditorCrash) {
    if ($result -notin @(-1073741819, 3221225477) -or $log -notmatch 'Editor') {
        throw "Expected Editor access violation (0xC0000005), got $result. See validation log."
    }
    'Original editor access violation reproduced.' | Tee-Object validation/result.txt
} else {
    if ($result -ne 0) { throw "pluginval failed with exit code $result" }
    if ($log -notmatch 'Editor' -or $log -notmatch 'ALL TESTS PASSED') { throw 'Missing editor test or final success marker' }
    'pluginval 1.0.4, strictness 5, three repeats, GUI enabled: PASS.' | Tee-Object validation/result.txt
}
$global:LASTEXITCODE = 0
