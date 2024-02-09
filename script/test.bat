@rem ------------------------------------------------------------------------------
@rem Copyright (C) Intel Corporation
@rem
@rem SPDX-License-Identifier: MIT
@rem ------------------------------------------------------------------------------
@rem Run the test suite.
@rem
@rem  Scope can be limited by providing subset of tests as argumene from among:
@rem  lint, unit

@echo off
setlocal ENABLEEXTENSIONS ENABLEDELAYEDEXPANSION

for %%Q in ("%~dp0\.") DO set "script_dir=%%~fQ"
pushd %script_dir%\..
  set "proj_dir=%cd%"
popd

if "%~1"=="" (
 set "do_lint=1" & goto done
 )
:loop
if "%1"=="lint" (set "do_lint=1") else ^
echo invalid option: '%1' && exit /b 1
shift
if not "%~1"=="" goto loop
:done


if "%do_lint%" == "1" (
  pushd %proj_dir%
    pre-commit run --all-files || EXIT /b 1
    pre-commit run --hook-stage manual gitlint-ci || EXIT /b 1
  popd
  if !errorlevel! neq 0 exit /b !errorlevel!
)

endlocal
