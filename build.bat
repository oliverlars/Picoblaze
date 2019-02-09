
@echo off

@set "LLVMPath=C:\Program Files\LLVM"
@set "VSPath=C:\Program Files (x86)\Microsoft Visual Studio\2017\Community"
@set "VSVersion=14.15.26726"
@set "WinSDKVersion=10.0.17134.0"
@set "WinSDKPath=C:\Program Files (x86)\Windows Kits\10"
@set "VSBasePath=%VSPath%\VC\Tools\MSVC\%VSVersion%"
@set "PATH=%PATH%;%LLVMPath%\bin;%VSBasePath%\bin\HostX64\x64"

@set CompilerFlags= ^
	-std=c++17 ^
	-stdlib=libc++ ^
    -Wno-everything

@set LinkFlags= ^
	-machine:x64 ^
	-subsystem:console

@set LinkLibs= ^
	/libpath:"%VSBasePath%\lib\x64" ^
	/libpath:"%WinSDKPath%\Lib\%WinSDKVersion%\ucrt\x64" ^
	/libpath:"%WinSDKPath%\Lib\%WinSDKVersion%\um\x64" ^
	/libpath:"C:\Program Files\LLVM\lib" ^
	libcmt.lib User32.lib 


@echo Compiling...
@clang++ -O3 compiler/picoblaze_compiler.cc -o picoblaze_compiler.o -c %CompilerFlags%
@lld-link.exe  "picoblaze_compiler.o" /OUT:"compiler/picoblaze.exe" %LinkFlags% %LinkLibs%
@clang++  -O3 vm/picoblaze_vm.cc -o picoblaze_vm.o -c %CompilerFlags%
@lld-link.exe  "picoblaze_vm.o" /OUT:"vm/picoblaze_vm.exe" %LinkFlags% %LinkLibs%

