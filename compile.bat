@echo on
set home=%CD%
echo %home%
cd dalvikhook\jni\
call make.bat 
cd %home%
ndk-build

