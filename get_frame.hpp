#ifndef GET_FRAME_HPP
#define GET_FRAME_HPP

uint8_t get_frame_init(uint16_t height, uint16_t width, uint8_t** data);
uint8_t get_frame_close();
uint8_t get_frame();

#endif