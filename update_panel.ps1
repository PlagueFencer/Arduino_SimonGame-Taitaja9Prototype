# Copies panel.html into panel_html.h so the ESP32 can serve it over Wi-Fi.
# Run this after every change to panel.html, then upload the sketch again:
#   powershell -ExecutionPolicy Bypass -File update_panel.ps1

$dir  = $PSScriptRoot
$html = [IO.File]::ReadAllText((Join-Path $dir "panel.html"))

$header = "// Generated from panel.html by update_panel.ps1 - edit panel.html, not this file.`n" +
          "#pragma once`n" +
          "const char PANEL_HTML[] PROGMEM = R`"PANEL(" + $html + ")PANEL`";`n"

[IO.File]::WriteAllText((Join-Path $dir "panel_html.h"), $header, (New-Object Text.UTF8Encoding $false))
Write-Host "panel_html.h updated"
