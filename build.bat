@echo off

mkdir build
pushd build
cl /std:c++20 /c ..\d2dl.cpp
cl ..\main.c d2dl.obj gdi32.lib user32.lib d2d1.lib dwrite.lib
popd
