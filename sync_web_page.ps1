# sync_web_page.ps1
# Reads preview/index.html, strips out the simulation fallback block,
# and wraps the result in a C PROGMEM string for web_page.h

$inputFile  = "preview\index.html"
$outputFile = "firmware\fan_control\web_page.h"

# Read all lines
$lines = Get-Content $inputFile

# Find the simulation block boundaries
# The simulation block starts at "/* -- Interactive Simulation Mode (Fallback) ---*/"
# and goes until the closing of the setTimeout block before </script>
$simStart = -1
$simEnd = -1

for ($i = 0; $i -lt $lines.Count; $i++) {
    if ($lines[$i] -match "Interactive Simulation Mode") {
        $simStart = $i
    }
    if ($simStart -ge 0 -and $lines[$i] -match "^\s*\}, 2000\);") {
        $simEnd = $i
        break
    }
}

Write-Host "Simulation block: lines $($simStart+1) to $($simEnd+1)"

# Build the HTML content: everything before sim block + everything after
if ($simStart -ge 0 -and $simEnd -ge 0) {
    $beforeSim = $lines[0..($simStart - 1)]
    $afterSim  = $lines[($simEnd + 1)..($lines.Count - 1)]
    $htmlLines = $beforeSim + $afterSim
} else {
    Write-Host "WARNING: Could not find simulation block boundaries. Using full file."
    $htmlLines = $lines
}

$htmlContent = $htmlLines -join "`n"

# Build the header file
$output = @"
#ifndef WEB_PAGE_H
#define WEB_PAGE_H

const char DASHBOARD_HTML[] PROGMEM = R"rawliteral(
$htmlContent
)rawliteral";

#endif
"@

# Write output
[System.IO.File]::WriteAllText(
    (Join-Path $PSScriptRoot $outputFile),
    $output,
    [System.Text.UTF8Encoding]::new($false)
)

$lineCount = ($htmlLines).Count
Write-Host "Successfully synced web_page.h ($lineCount lines of HTML)"
