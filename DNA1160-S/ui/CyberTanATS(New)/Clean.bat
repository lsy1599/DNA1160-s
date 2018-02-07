del /s Thumbs.db
del /s *.ipch
del /s *.7z
REM del /s *.exe
del /s *.png
del /s *.ncb
del /s *.pch
del /s *.pdb
del /s *.ilk
del /s *.obj
del /s *.idb
del /s *.dep
del /s *.res
REM del /s *.manifest
del /s UpgradeLog.htm
del /s UpgradeLog.XML
del /s *.sdf
del /s *.tlog
del /s *.log
del /s *.lastbuildstate
del /s *.tmp
del /s *.old
del /s *.bak
del /s *.gid
del /s *._mp
del /s *.chk
del /s *.Cache
del /s *.cache
del /s *.VC.db

RD /s /Q Debug
RD /s /Q Release
RD /s /Q ipch
RD /s /Q _UpgradeReport_Files
REM RD /s /Q Build

FOR  %%i in (*.sln) DO (SET SolutionOrProjectPath="%~dp0"%%~ni)
RD /s /Q %SolutionOrProjectPath%\Debug
RD /s /Q %SolutionOrProjectPath%\Release

call CyberTanATS/Clean.bat