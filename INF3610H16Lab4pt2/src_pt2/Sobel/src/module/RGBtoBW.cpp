///////////////////////////////////////////////////////////////////////////////
//
// Filename : RGBtoBW.cpp
//
// Creation date : Thu Mar 24 11:09:20 EDT 2016
//
///////////////////////////////////////////////////////////////////////////////
#include "RGBtoBW.h"

#include "PlatformDefinitions.h"
#include "ApplicationDefinitions.h"
#include "SpaceDisplay.h"
#include "SpaceTypes.h"
#include <inttypes.h>
#include <cstring> // For memset


// RGB to Y Conversion
// Resulting luminance value used in edge detection

/*** Utils ***/
static inline uint8_t rgb2y(uint8_t R, uint8_t G, uint8_t B) {
	return ((66 * R + 129 * G + 25 * B + 128) >> 8) + 16;
}


RGBtoBW::RGBtoBW(sc_core::sc_module_name name, double period, sc_core::sc_time_unit unit, unsigned char id, unsigned char priority, bool verbose)
:SpaceBaseModule(name, period, unit, id, priority, verbose) {
	SC_THREAD(thread);
	set_stack_size(0x16000+(IMG_SIZE));
}

void RGBtoBW::thread(void) {
	message_t message_in, message_out;
	message_out.command_type = MSG_SOBEL;

	while(1) {
		MyPrint("[RGBtoBW] executing...\n");
		
		for(int i = 0; i < IMG_SIZE; ++i) {
			// Receiving one pixel
			ModuleRead(BITMAPRW_ID, SPACE_BLOCKING, &message_in);
			// TODO Check the message type.
			// Converting
			uint8_t R = (uint8_t)message_in.param0;
			uint8_t G = (uint8_t)message_in.param1;
			uint8_t B = (uint8_t)message_in.param2;
			uint8_t Y = rgb2y(R, G, B);
			// Sending the converted pixel
			message_out.param0 = (cmd_param_t)Y;
			ModuleWrite(SOBEL_ID, SPACE_BLOCKING, &message_out);
		}
	}
}
