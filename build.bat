@echo off
@setlocal EnableDelayedExpansion enableextensions
@cls
@cd %~dp0
@SET PROGRAM_NAME=%~dp0
@for /D %%a in ("%PROGRAM_NAME:~0,-1%.txt") do @SET PROGRAM_NAME=%%~na

@SET EABI=arm-none-eabi
@SET AS=%EABI%-as
@SET LD=%EABI%-ld
@SET OBJCOPY=%EABI%-objcopy
@SET GCC=%EABI%-gcc
@SET NM=%EABI%-nm
@SET ELFUSCATE=..\elfuscate1.exe

@SET LIB_BIP_PATH="..\libbip"
@SET LIB_BIP="%LIB_BIP_PATH%\libbip.a" 

@SET GCC_OPT=-mthumb -mcpu=cortex-m4 -mfloat-abi=hard -mfpu=auto -fsingle-precision-constant -fno-math-errno -ffast-math
@SET GCC_OPT=%GCC_OPT% -I "%LIB_BIP_PATH%" -c -Os -Wa,-R -Wall -fpie -pie -fpic -mlittle-endian  -ffunction-sections -fdata-sections 

@for /f "tokens=2 delims=[=]"  %%i in ('"!GCC! !GCC_OPT! -print-search-dirs"') do	SET LIB_SRCH=%%~i

set LIB_SRCH=!LIB_SRCH:;= -L !
	
@SET LD_OPT=-L!LIB_BIP_PATH! -L!LIB_SRCH! -EL -N -Os --cref -pie --gc-sections	
rem Включить подробный вывод линковщика (если потерялись библиотеки)
rem @SET LD_OPT=!LD_OPT! --verbose

if exist label.txt (
set /p LABEL=< label.txt
) else (
SET LABEL = %PROGRAM_NAME%
)

@call :echoColor 0F "====================================" 1
@call :echoColor 0F "Наименование проекта: "
@echo %PROGRAM_NAME%
@call :echoColor 0F "Компилятор: "
@echo %COMPILER%
@call :echoColor 0F "====================================" 1
@echo.	

@call :echoColor 0F "Начинаем сборку..." 1
@SET PARTNAME=%PROGRAM_NAME%
@call :echoColor 0B "Компиляция "
@call :echoColor 0E "%PARTNAME%" 1

@SET n=1
@for  %%f in (*.c) do ( 
@	SET FILES_TO_COMPILE=!FILES_TO_COMPILE! %%~nf.o
@call :EchoN "%n%.	%%~nf.c"
!GCC! !GCC_OPT! -o %%~nf.o %%~nf.c
@if errorlevel 1 goto :error
@call :echoColor 0A "...OK" 1
@SET /a n=n+1)
@SET /a n=n-1
@call :echoColor 0B "Итого: "
@call :echoColor 0E "%n%" 1

@call :echoColor 0B "Сборка:"
@call :echoColor 07 "	...создание elf файла"	1
%LD% %LD_OPT% -Map %PARTNAME%.map -o %PROGRAM_NAME%.elf %FILES_TO_COMPILE% -lbip -lgcc -lm -lc 
@if errorlevel 1 goto :error

if exist label.txt (
@call :echoColor 07 "	...название"	1
%OBJCOPY%  %PROGRAM_NAME%.elf --add-section .elf.label=label.txt
)

@call :EchoN "%PROGRAM_NAME%" > name.txt
@call :echoColor 07 "	...elf_name"	1
%OBJCOPY%  %PROGRAM_NAME%.elf --add-section .elf.name=name.txt
if exist name.txt del name.txt
@if errorlevel 1 goto :error

if exist asset.res (
@call :echoColor 07 "	...ресурсы"	1
%OBJCOPY%  %PROGRAM_NAME%.elf --add-section .elf.resources=asset.res
)

if exist settings.bin (
@call :echoColor 07 "	...настройки"	1
%OBJCOPY%  %PROGRAM_NAME%.elf --add-section .elf.settings=settings.bin
)

if exist %ELFUSCATE% (
@call :echoColor 07 "	...обфускация"	1
%ELFUSCATE% -f %PROGRAM_NAME_%.elf %PROGRAM_NAME%.elf > nul
del %PROGRAM_NAME%.elf > nul
move %PROGRAM_NAME_%.elf %PROGRAM_NAME%.elf > nul
)


@call :echoColor 0A "...OK" 1
@call :echoColor 0B "Сборка окончена." 1

:done_

@call :echoColor 0A "Готово." 1 
pause 
@goto :EOF

:error
@call :echoColor 4e ОШИБКА! 1
@endlocal & @SET ERROR=ERROR
@pause
@goto :EOF

::===========================================================
:: A function prints text in first parameter without CRLF 
:EchoN
    
@    <nul set /p strTemp=%~1
@    exit /b 0
::===========================================================
::	Вывод заданной строки заданным цветом
::	3 параметр если не пустой задает перевод строки
::  0 = Черный 	8 = Серый
::  1 = Синий 	9 = Светло-синий
::  2 = Зеленый A = Светло-зеленый
::  3 = Голубой B = Светло-голубой
::  4 = Красный C = Светло-красный
::  5 = Лиловый D = Светло-лиловый
::  6 = Желтый 	E = Светло-желтый
::  7 = Белый 	F = Ярко-белый
:echoColor [Color] [Text] [\n]
 @ if not defined BS for /F "tokens=1 delims=#" %%i in ('"prompt #$H#& echo on& for %%j in (.) do rem"') do set "BS=%%i"
 @ if not exist foo set /p .=.<nul>foo
 @ set "regex=%~2" !
 @ set "regex=%regex:"="%"
 @ findstr /a:%1 /prc:"\." "%regex%\..\foo" nul
 @ set /p .=%BS%%BS%%BS%%BS%%BS%%BS%%BS%%BS%%BS%<nul
 @ if "%3" neq "" echo.
 @exit /b 0
::===========================================================
====================================================
