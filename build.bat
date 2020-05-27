@echo off
setlocal EnableDelayedExpansion

set name=faito

set live_code_reloading=true
rem set live_code_reloading=false

rem nologo: disable visual studio copyright info
rem Zi: generate debug information
rem Od: optimize for debugging
rem Mtd: use multithreaded static debug runtime lib (no visual studio redistributables)
rem EHsc: use minimal exception handling?
set options=/nologo /Zi /Od /MTd /EHsc /I "%cd%\3rd_party"

set t=%time:~0,8%
set t=%t::=-%

set current=%cd%

if not exist build\ mkdir build
pushd build

del *.pdb > NUL 2> NUL  
del live.dll > NUL 2> NUL

if %live_code_reloading% == true (
    set options=%options% /DLIVE_CODE_RELOADING
    rem while compile lock exists our application does not try to reload the new dll
    echo compiling... > compile.lock
    rem !options! uses changes to options immediately, otherwise %options% is only updated after if statement  
    cl /Fe%name% %current%/source/main.cpp !options! /LD /INCREMENTAL:NO /link /PDB:"%name%%date%-%t%.pdb" 
    del compile.lock
)

if not exist live.dll (
    cl /Fe%name% %current%/source/win32_main.cpp %options% /link /INCREMENTAL:NO	 Shell32.lib
)

popd