@ECHO OFF

REM --== Engine ==--

SET EnginePath=C:\Program Files\Epic Games\UE_4.27
SET BatchFilesPath=%EnginePath%\Engine\Build\BatchFiles
SET RunUAT=%BatchFilesPath%\RunUAT.bat

REM --== Project ==--

FOR %%I IN ("%~dp0.") DO SET "ProjectRoot=%%~dpI"
SET ProjectPureName=SimpleVisualizer
SET ProjectName=%ProjectPureName%.uplugin
SET ProjectPath=%ProjectRoot%%ProjectName%

REM --== Platform ==--

SET Platforms=Win64
SET ArchivePath=%ProjectRoot%Build\1.0.0

REM --== Commands ==--

"%RunUAT%" BuildPlugin ^
-Rocket ^
-Plugin="%ProjectPath%" ^
-TargetPlatforms="%Platforms%" ^
-Package="%ArchivePath%" ^
-VS2019 %*
