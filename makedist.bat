rd /s /q dist

md dist
md dist\data

xcopy /s data dist\data
xcopy lib\win32\*.dll dist
xcopy Release\stix.exe dist
xcopy readme.txt dist

pause