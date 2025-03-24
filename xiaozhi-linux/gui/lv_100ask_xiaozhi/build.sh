rm -rf build
mkdir -p build
cd build/
cmake -DCMAKE_TOOLCHAIN_FILE="../toolchain.cmake" ..
make -j32
wait
cd ../ 
cp bin/main bin/lvgl_xiaozhi
echo "get app bin/lvgl_xiaozhi"
