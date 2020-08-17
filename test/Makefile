UDPtest: UDP_test.cpp
	g++ UDP_test.cpp -o UDPtest

test1: test_1.cpp test.hpp
	g++ test_1.cpp -I /usr/local/include/opencv -I /usr/local/include/opencv2 -lm /usr/local/lib/libopencv_* -o test1

rs422test: rs422_test.cpp
	g++ rs422_test.cpp -o rs422test

maintest: maintest.cpp test.hpp
	g++ maintest.cpp /usr/local/xclib/lib/xclib_aarch64.so -I /usr/local/xclib/inc/ -I /usr/local/include/opencv -I /usr/local/include/opencv2 -lm /usr/local/lib/libopencv_* -lm //lib/aarch64-linux-gnu/libpthread.so.0 -o maintest

calibration: calibration.cpp test.hpp
	g++ calibration.cpp /usr/local/xclib/lib/xclib_aarch64.so -I /usr/local/xclib/inc/ -I /usr/local/include/opencv -I /usr/local/include/opencv2 -lm /usr/local/lib/libopencv_* -lm //lib/aarch64-linux-gnu/libpthread.so.0 -o calibration
