#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>   // For cap query.
#include <stdio.h>
#include <linux/videodev2.h>
#include <errno.h>
#include <stdint.h>
#include <string.h>  			//
#include <sys/mman.h>
#include <unistd.h>

/**
 * The following code is adapted from 
 * http://jwhsmith.net/2014/12/capturing-a-webcam-stream-using-v4l2/
 */

int fd;
const unsigned int PIX_FORMAT = V4L2_PIX_FMT_MJPEG;
struct v4l2_buffer bufferinfo = NULL;
uint8_t* buffer;


uint8_t get_frame_init(uint16_t height, uint16_t width, uint8_t** data)
{
	
	// Open camera.
	fd = open("/dev/video0", O_RDWR);
	if (fd == -1) 
	{
		perror("Opening video device");
		return -1;
	}
	
	// Set format.
	struct v4l2_format format;
	format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	format.fmt.pix.pixelformat = PIX_FORMAT;
	format.fmt.pix.width = width;
	format.fmt.pix.height = height;
	
	if(ioctl(fd, VIDIOC_S_FMT, &format) < 0){
		perror("VIDIOC_S_FMT");
		return -1;
	}
	
	// Request a buffer
	struct v4l2_requestbuffers bufrequest;
	bufrequest.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	bufrequest.memory = V4L2_MEMORY_MMAP;
	bufrequest.count = 1;
	 
	if(ioctl(fd, VIDIOC_REQBUFS, &bufrequest) < 0){
		perror("VIDIOC_REQBUFS");
		return -1;
	}
	
	
	// Allocate buffer.
	memset(&bufferinfo, 0, sizeof(bufferinfo));
	 
	bufferinfo.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	bufferinfo.memory = V4L2_MEMORY_MMAP;
	bufferinfo.index = 0;
	 
	if(ioctl(fd, VIDIOC_QUERYBUF, &bufferinfo) < 0){
		perror("VIDIOC_QUERYBUF");
		return -1;
	}
	
	buffer = (uint8_t*) mmap(
    NULL,
    bufferinfo.length,
    PROT_READ | PROT_WRITE,
    MAP_SHARED,
    fd,
    bufferinfo.m.offset
	);
	 
	if(buffer == MAP_FAILED){
		perror("mmap");
		return -1;
	}
	
	
	
	// Set up returns
	*data = buffer;
	
	
	// Activate streaming
	int type = bufferinfo.type;
	if(ioctl(fd, VIDIOC_STREAMON, &type) < 0){
			perror("Stream on");
			return -1;
	}
	
	return 0;
	
	
}

uint8_t get_frame_close()
{
		
	// Deactivate streaming	
	int type = bufferinfo.type;
	if(ioctl(fd, VIDIOC_STREAMOFF, &type) < 0){
			perror("Stream off");
			return -1;
	}
	
	close(fd);
	fd = -1;
	
	
	munmap(buffer, bufferinfo.length);
	buffer = NULL;
	
	return 0;

}

uint32_t get_buffer_size()
{
	if (bufferinfo == NULL) return 0;
	else return bufferinfo.length;
}

uint8_t get_frame() 
{
	
	memset(buffer, 0, bufferinfo.length);
	
	 
	// Get a frame.
	// Put the buffer in the incoming queue.
	if(ioctl(fd, VIDIOC_QBUF, &bufferinfo) < 0){
			perror("Enqueue");
			return -1;
	}
	 
	// The buffer's waiting in the outgoing queue.
	if(ioctl(fd, VIDIOC_DQBUF, &bufferinfo) < 0){
			perror("Dequeue");
			return -1;
	}
	
	
	return 0;
	
}