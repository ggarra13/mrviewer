@echo off
call "%vs100comntools%vsvars32.bat"
cl /EP /I include src\x86\win32.S > src\x86\win32.asm
cl /EP /I include src\x86\win64.S > src\x86\win64.asm
pause
