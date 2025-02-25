cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build
mv build/WebServer WebServer