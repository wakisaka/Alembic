@echo off
set deploypath=..\..\..\..\..\..\Deploy
set src=..\..\..\..\..\pyilmbase-1.0.0\PyImath

if not exist %deploypath% mkdir %deploypath%

set intdir=%1%
if %intdir%=="" set intdir=Release
echo Installing into %intdir%
set instpath=%deploypath%\lib\%intdir%
if not exist %instpath% mkdir %instpath%
copy ..\%intdir%\PyImath_alembic.lib %instpath%
copy ..\%intdir%\PyImath_alembic.exp %instpath%

set instpath=%deploypath%\bin\%intdir%
if not exist %instpath% mkdir %instpath%
copy ..\%intdir%\PyImath_alembic.dll %instpath%

cd %src%
set instpath=..\..\..\Deploy\include
mkdir %instpath%
copy *.h %instpath%

