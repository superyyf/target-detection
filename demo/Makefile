function: function.cpp main.hpp
	g++ function.cpp -I /usr/local/xclib/inc/ -I /usr/local/include/opencv -I /usr/local/include/opencv2 -lm /usr/local/lib/libopencv_* -c function.o
demoMain: demo_main.cpp main.hpp queue.hpp function.cpp
	g++ demo_main.cpp function.cpp /usr/local/xclib/lib/xclib_aarch64.so -I /usr/local/xclib/inc/ -I /usr/local/include/opencv -I /usr/local/include/opencv2 -lm /usr/local/lib/libopencv_* -lm //lib/aarch64-linux-gnu/libpthread.so.0 --std=c++11 -o demoMain
demoWithBDS: demoWithBDS.cpp main.hpp function.cpp queue.hpp
	g++ demoWithBDS.cpp function.cpp /usr/local/xclib/lib/xclib_aarch64.so -I /usr/local/xclib/inc/ -I /usr/local/include/opencv -I /usr/local/include/opencv2 -lm /usr/local/lib/libopencv_* -lm //lib/aarch64-linux-gnu/libpthread.so.0 --std=c++11 -o demoWithBDS

