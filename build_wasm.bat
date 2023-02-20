@echo off
call emsdk\emsdk_env.bat

mkdir build_wasm
cd build_wasm

call emcmake cmake ../
call emmake make