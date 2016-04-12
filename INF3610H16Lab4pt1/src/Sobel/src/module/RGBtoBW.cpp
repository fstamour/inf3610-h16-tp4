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
	message_t message;
	uint8_t *Y = (uint8_t *)malloc(IMG_SIZE * sizeof(uint8_t));

	while(1) {
		// Add your code here...

		ModuleRead(BITMAPRW_ID, SPACE_BLOCKING, &message);

		SpacePrint( "RGBtoBW executing...\n" );
		
		if(message.command_type != MSG_RGB_TO_BW) {
			SpacePrint("[RGBtoBW] Invalid message type: %d\n", message.command_type);
			sc_stop();
		}

		SpacePrint( "[RGBtoBW] About to convert the bitmap...\n" );

		uint8_t *R = (uint8_t *)message.param0;
		uint8_t *G = (uint8_t *)message.param1;
		uint8_t *B = (uint8_t *)message.param2;

		// Converting the image from RBG to Y
		for(int x = 0; x < IMG_WIDTH; ++x) {
			for(int y = 0; y < IMG_HEIGHT; ++y) {
				int index = y * IMG_WIDTH + x;
				int luma = rgb2y(R[index], G[index], B[index]);
				Y[index] = luma;
			}
		}

		SpacePrint( "[RGBtoBW] Sending bitmap...\n" );
		memset(&message, 0, sizeof(message));
		message.command_type = MSG_SOBEL;
		message.param0 = (cmd_param_t)Y;
		ModuleWrite(SOBEL_ID, SPACE_BLOCKING, &message);
	}
}
