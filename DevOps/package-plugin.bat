@ECHO OFF

REM --== Manual Config ==--

SET PluginPureName=ShapesVisualizer
SET Platforms=Win64

REM --== Script Params ==--

SET EngineVersion=%~1
SET EnginePath=%~2

REM --== Config ==--

SET BatchFilesPath=%EnginePath%\Engine\Build\BatchFiles
SET RunUAT=%BatchFilesPath%\RunUAT.bat

FOR %%I IN ("%~dp0.") DO SET "PluginRoot=%%~dpI"
SET PluginName=%PluginPureName%.uplugin
SET PluginPath=%PluginRoot%%PluginName%
SET TemplateName=%PluginPureName%.template
SET TemplatePath=%~dp0%TemplateName%

SET PackagePath=%PluginRoot%Build\%EngineVersion%
SET ZipPath=%PluginRoot%Build\%PluginPureName%_%EngineVersion%.zip

REM --== Check Errors ==--

IF [%EngineVersion%] == [] GOTO ErrInvalidParams
IF "%EnginePath%" == "" GOTO ErrInvalidParams
IF NOT EXIST "%RunUAT%" GOTO ErrNoRunUAT
IF NOT EXIST "%TemplatePath%" GOTO ErrNoTemplate

REM --== Make new *.uplugin ==--

DEL /Q "%PluginPath%"

FOR /F "usebackq delims= eol=" %%a IN ("%TemplatePath%") DO (
    CALL ECHO %%a >> "%PluginPath%"
)

REM --== Build Plugin ==--

CALL "%RunUAT%" BuildPlugin ^
-Plugin="%PluginPath%" ^
-TargetPlatforms="%Platforms%" ^
-Package="%PackagePath%" ^
-Rocket -VS2019

IF NOT %ERRORLEVEL% == 0 GOTO ErrUATFailed

REM --== Compress ==--

CALL tar -cav ^
-f "%ZipPath%" ^
-C "%PackagePath%" ^
*

REM --== Success ==--

GOTO :EOF

REM --== Errors ==--

:ErrInvalidParams
ECHO [SCRIPT]: Invalid input parameters
GOTO :EOF

:ErrNoRunUAT
ECHO [SCRIPT]: File "%RunUAT%" not found
GOTO :EOF

:ErrNoTemplate
ECHO [SCRIPT]: File "%TemplatePath%" not found
GOTO :EOF

:ErrUATFailed
ECHO [SCRIPT]: RunUAT failed
GOTO :EOF
