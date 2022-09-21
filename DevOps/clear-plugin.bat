@ECHO OFF
REM DOCS: https://ss64.com/nt

REM --== Configuration ==--

SET FILES_TO_REMOVE=*.sln *.ipch
SET FOLDERS_TO_REMOVE=.vs Build Binaries Intermediate

REM --== Commands ==--

PUSHD "%~dp0\.."

FOR %%f IN (%FOLDERS_TO_REMOVE%) DO (
	RMDIR /S /Q %%f
)

FOR %%f IN (%FILES_TO_REMOVE%) DO (
	DEL /Q %%f
)

POPD
