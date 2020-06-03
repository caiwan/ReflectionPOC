@echo off

if not exist msvc md msvc
if exist msvc\CMakeCache.txt del msvc\CMakeCache.txt

cd msvc
cmake -DCMAKE_GENERATOR_PLATFORM=x64 ../
cd ..
