haptic_glove: haptic_glove.cpp get_frame.cpp
	g++ -g -o haptic_glove --std=c++0x haptic_glove.cpp get_frame.cpp ../streaming_lib/CAPIStreamClient/CAPINetworkUtility/CAPINetworkUtility.cpp ../streaming_lib/CAPIStreamClient/CAPIStreamClient.cpp ../streaming_lib/CAPIStreamClient/SocketData/SocketData.cpp -lpthread
	# removed -lmraa