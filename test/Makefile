UDPtest: UDP_test.cpp
	g++ UDP_test.cpp -o UDPtest

test1: test_1.cpp function.cpp main.hpp
	g++ test_1.cpp function.cpp /usr/local/xclib/lib/xclib_aarch64.so -I /usr/local/xclib/inc/ -I /usr/local/include/opencv -I /usr/local/include/opencv2 -lm /usr/local/lib/libopencv_* -lm //lib/aarch64-linux-gnu/libpthread.so.0 -o test1

rs422test: rs422_test.cpp
	g++ rs422_test.cpp -o rs422test

maintest: maintest.cpp function.cpp main.hpp 
	g++ maintest.cpp function.cpp /usr/local/xclib/lib/xclib_aarch64.so -I /usr/local/xclib/inc/ -I /usr/local/include/opencv -I /usr/local/include/opencv2 -lm /usr/local/lib/libopencv_* -lm //lib/aarch64-linux-gnu/libpthread.so.0 -o maintest

calibration: calibration.cpp main.hpp
	g++ calibration.cpp /usr/local/xclib/lib/xclib_aarch64.so -I /usr/local/xclib/inc/ -I /usr/local/include/opencv -I /usr/local/include/opencv2 -lm /usr/local/lib/libopencv_* -lm //lib/aarch64-linux-gnu/libpthread.so.0 -o calibration

entrewindow: entrewindow.cpp main.hpp function.cpp
	g++ entrewindow.cpp function.cpp /usr/local/xclib/lib/xclib_aarch64.so -I /usr/local/xclib/inc/ -I /usr/local/include/opencv -I /usr/local/include/opencv2 -lm /usr/local/lib/libopencv_* -lm //lib/aarch64-linux-gnu/libpthread.so.0 -o entrewindow
planb: plan_b.cpp main.hpp function.cpp	
	g++ plan_b.cpp function.cpp /usr/local/xclib/lib/xclib_aarch64.so -I /usr/local/xclib/inc/ -I /usr/local/include/opencv -I /usr/local/include/opencv2 -lm /usr/local/lib/libopencv_* -lm //lib/aarch64-linux-gnu/libpthread.so.0 -o planb
demo: demo.cpp main.hpp function.cpp
	g++ demo.cpp function.cpp /usr/local/xclib/lib/xclib_aarch64.so -I /usr/local/xclib/inc/ -I /usr/local/include/opencv -I /usr/local/include/opencv2 -lm /usr/local/lib/libopencv_* -lm //lib/aarch64-linux-gnu/libpthread.so.0 --std=c++11 -o demo
