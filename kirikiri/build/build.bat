if "%VSCMD_VER%" EQU "" (
  echo "init msvc env"
  call "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat"
)

MSBUILD %~dp0\..\kirikiri.sln /t:rebuild /p:Configuration="Debug";Platform=x86 /m
pause
MSBUILD %~dp0\..\kirikiri.sln /t:rebuild /p:Configuration="Release";Platform=x86 /m
pause


set arc_name=krkr-v1.0.2.zip
set pdb_name=krkr-v1.0.2-pdb.zip
set tmp_folder1=krkr
set tmp_folder2=krkr-pdb

cd %~dp0
rmdir /S /Q %tmp_folder1%
xcopy ..\bin\*.exe %tmp_folder1%\ /E /V /Q /H
xcopy ..\bin\*.dll %tmp_folder1%\ /E /V /Q /H
"c:\Program Files\7-zip\7z.exe" a "%arc_name%" %tmp_folder1%
"c:\Program Files\7-zip\7z.exe" t "%arc_name%" *
pause

rmdir /S /Q %tmp_folder2%
xcopy ..\pdb\*.pdb %tmp_folder2%\ /E /V /Q /H
"c:\Program Files\7-zip\7z.exe" a "%pdb_name%" %tmp_folder2%
"c:\Program Files\7-zip\7z.exe" t "%pdb_name%" *
pause

rmdir /S /Q %tmp_folder1%
rmdir /S /Q %tmp_folder2%
