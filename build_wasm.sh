source emsdk/emsdk_env.sh

mkdir build_wasm
cd build_wasm

emcmake cmake ../
emmake make