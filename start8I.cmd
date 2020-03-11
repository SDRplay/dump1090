@echo off
title CTRL-C to EXIT - v1.46 High Perf Interactive dump1090
.\DisableClose.exe
.\sdrplay_dump1090.exe --dev-sdrplay --interactive --net --oversample
pause
