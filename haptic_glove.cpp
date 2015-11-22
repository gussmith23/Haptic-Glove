#include "CAPINetworkUtility.hpp"
#include "SocketData.hpp"

#include <cstdint>

/////////////////////////
// Enums

// Vibration direction.
typedef enum{
	UP, LEFT, RIGHT, DOWN
} Direction;


////////////////////////
// Fields

// Connection fields
CAPIStreamClient *myClient;
char* inet_addr = "192.168.1.3";

// The height and width of the current frame.
const uint32_t HEIGHT = 720, WIDTH = 1280;
const uint32_t HALF_HEIGHT = HEIGHT/2, HALF_WIDTH = WIDTH/2;
const float SLOPE = (float) HEIGHT/ (float) WIDTH;


/////////////////////////
// Function prototypes
int process_return_packet(SocketData* socketData);
int vibrate_motor (Direction direction);


int main()
{
	
	////////////////////////
	// Step 0: Setup.
	
	// Set up connection.
	myClient = new CAPIStreamClient();
	int err = myClient->connect(, 1000);
	if(err != 0){
		fprintf(stderr, "Failed to connect to server.")
	}	
	
	// Set up webcam.
	
	////////////////////////
	// Step 1: Get frame data from webcam.
	
	////////////////////////
	// Step 2: Send frame data.
	
	////////////////////////
	// Step 3: Receive response.
	
	////////////////////////
	// Step 4: Send to GPIO.
	
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
int process_return_packet(SocketData* socketData)
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
	if (y > x*slope) above_bottom_left_to_top_right = 1;		
	if (y > (WIDTH-x)*slope) above_top_left_to_bottom_right = 1;
	
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