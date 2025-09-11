call "%VS140COMNTOOLS%\..\..\VC\vcvarsall.bat" x64
msbuild AeroMap.sln /t:rebuild /p:configuration=Debug /p:platform=x64
msbuild AeroMap.sln /t:rebuild /p:configuration=Release /p:platform=x64
