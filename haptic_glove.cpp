#include "../streaming_lib/CAPIStreamClient/CAPINetworkUtility/CAPINetworkUtility.hpp"
#include "../streaming_lib/CAPIStreamClient/SocketData/SocketData.hpp"
#include "../streaming_lib/CAPIStreamClient/CAPIStreamClient.hpp"
#include "get_frame.hpp"
//#include "mraa.hpp"					// For motor control.

#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <cstdint>
#include <unistd.h> 			// sleep.

/////////////////////////
// Enums

// Vibration direction.
typedef enum{
	UP, LEFT, RIGHT, DOWN
} Direction;


////////////////////////
// Fields

// Connection fields
CAPINetworkUtility *myClient;
char* addr = "192.168.82.186";
const uint32_t port = 2275;

// The height and width of the current frame.
const uint32_t HEIGHT = 480, WIDTH = 640;
const uint32_t HALF_HEIGHT = HEIGHT/2, HALF_WIDTH = WIDTH/2;
const float SLOPE = (float) HEIGHT/ (float) WIDTH;


// mraa fields.
/*
mraa::Gpio *left, *right, *up, *down;
const uint32_t LEFT_PIN = 31,   //GP44
								RIGHT_PIN = 45,	//GP45
								UP_PIN = 32,		//GP46
								DOWN_PIN = 46;	//GP47*/


/////////////////////////
// Function prototypes
uint8_t process_return_packet(SocketData* socketData);
uint8_t vibrate_motor(Direction direction);
void sig_handler(int signo);


int main()
{
	
	////////////////////////
	// Step 0: Setup.
	
	// Set sig handler.
	signal(SIGINT, sig_handler);
	
	// Set up mraa.
	//printf("Initializing connection to motors...\n");
	//mraa_init();
	
	// Init.
	/*left = new mraa::Gpio(LEFT_PIN);
	right = new mraa::Gpio(RIGHT_PIN);
	up = new mraa::Gpio(UP_PIN);
	down = new mraa::Gpio(DOWN_PIN);
	if (left == NULL || right == NULL || up == NULL || down == NULL)
	{
		fprintf(stderr, "Error initializing connection to motors.\n");
		return -1;
	}
	
	// Set to output.
	mraa::Result l = mraa::SUCCESS,
								r = mraa::SUCCESS,
								u = mraa::SUCCESS,
								d = mraa::SUCCESS;
							
	l = left->dir(mraa::DIR_OUT);
	r = right->dir(mraa::DIR_OUT);
	u = up->dir(mraa::DIR_OUT);
	d = down->dir(mraa::DIR_OUT);
	if (l != mraa::SUCCESS || r != mraa::SUCCESS || u != mraa::SUCCESS || d != mraa::SUCCESS)
	{
		fprintf(stderr, "Error setting gpio direction.\n");
		return -1;
	}
	
	// TEST DEBUG TEST
	while(true)
	{
		vibrate_motor(UP);
	}
	// END DEBUG
	*/
	
	// Set up connection.
	
	printf("Connecting to %s:%d.\n", addr, port);
	
	myClient = new CAPINetworkUtility();
	int err = myClient->openSocket(addr, port);
	if(err != 0){
		fprintf(stderr, "Failed to connect to server.");
		return -1;
	}	
	
	// Set up webcam.
	uint8_t* buffer;
	if (get_frame_init(HEIGHT, WIDTH, &buffer) != 0)
	{
		fprintf(stderr,"get_frame_init failed.\n");
		return -1;
	}
	
	while(true)
	{
		////////////////////////
		// Step 1: Get frame data from webcam.
		
		printf("Getting frame from webcam.\n");
		
		get_frame();
		
		////////////////////////
		// Step 2: Send frame data.
		
		uint32_t buffer_size = get_buffer_size();
		if (buffer_size == 0)
		{
			fprintf(stderr, "buffer size is zero! no data to send!");
			return -1;
		}
		
		printf("Sending packet of length %d.\n", buffer_size);	
		
		// Create new packet.
		SocketData * myPacket = new SocketData(VIDEO_FRAME, 0, 0, 0, 0, NULL);
		
		// Copy data into packet
		myPacket->data = (uint8_t *)malloc(buffer_size);
		memcpy(myPacket->data, buffer, buffer_size);
		myPacket->message_length = buffer_size;
		
		myClient->sendDataPacket(myPacket);
		
		
		////////////////////////
		// Step 3: Receive response.
	}
	
	
	
	////////////////////////
	// Step 4: Send to GPIO.
	
	
	
	////////////////////////
	// Close.
	
	printf("Closing webcam.\n");
	
		if (get_frame_close() != 0)
	{
		fprintf(stderr,"get_frame_close failed.\n");
		return -1;
	}
	
	return 0;
}

/**
 * process_return_packet
 *
 * Description:
 * The server is expected to return an (x,y) pair representing the location
 * of the tracked object in the frame. This function translates that pair
 * into one of four quadrants of the screen. We then vibrate the appropriate 
 * motor for the quadrant, as seen in this diagram:
 * _____________
 * |\    UP   /|
 * |  \     /  |
 * |LF  \ /  RT|
 * |    / \    |
 * |  /     \  |
 * |/____DN___\|
 * 
 */
uint8_t process_return_packet(SocketData* socketData)
{

	/////////////////////
	// Step 1: Get the actual data.
	uint8_t *data = socketData->data;
	uint32_t x = ((((((data[3] << 8) | data[2]) << 8) | data[1]) << 8) | data[0]);
	uint32_t y = ((((((data[7] << 8) | data[6]) << 8) | data[5]) << 8) | data[4]);
	
	//////////////////////
	// Step 2: Check which motor to vibrate.
	
	Direction direction;
	
	// Check if it's above the midlines.
	uint8_t above_bottom_left_to_top_right = 0;
	uint8_t above_top_left_to_bottom_right = 0;
	if (y > x*SLOPE) above_bottom_left_to_top_right = 1;		
	if (y > (WIDTH-x)*SLOPE) above_top_left_to_bottom_right = 1;
	
	// Now break into cases:
	if (above_bottom_left_to_top_right && above_top_left_to_bottom_right)
		direction = UP;
	if (above_bottom_left_to_top_right && !above_top_left_to_bottom_right)
		direction = LEFT;
	if (!above_bottom_left_to_top_right && above_top_left_to_bottom_right)
		direction = RIGHT;
	else
		direction = DOWN;
	
	///////////////////////
	// Step 3: vibrate motor.
	return vibrate_motor(direction);
	
}

uint8_t vibrate_motor(Direction direction)
{/*
	
	switch(direction)
	{
		case UP:
			up->write(1);
			sleep(1);
			up->write(0);
			break;
		case DOWN:
			down->write(1);
			sleep(1);
			down->write(0);
			break;
		case LEFT:
			left->write(1);
			sleep(1);
			left->write(0);
			break;
		case RIGHT:
			right->write(1);
			sleep(1);
			right->write(0);
			break;
	}
	
	//test
	sleep(1);
	*/
	return 0;
}

void sig_handler(int signo)
{
	if (signo == SIGINT)
	{
		get_frame_close();
		exit(0);
	}
}
