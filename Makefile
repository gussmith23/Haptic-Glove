haptic_glove: haptic_glove.cpp get_frame.cpp
	g++ -g -o haptic_glove --std=c++0x haptic_glove.cpp get_frame.cpp ../streaming_lib/Common/network/src/*.cpp ../streaming_lib/Client/src/CAPIStreamClient.cpp
