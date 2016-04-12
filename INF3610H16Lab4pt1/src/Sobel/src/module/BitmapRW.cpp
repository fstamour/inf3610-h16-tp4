///////////////////////////////////////////////////////////////////////////////
//
// Filename : BitmapRW.cpp
//
// Creation date : Sat Mar 12 14:54:30 EST 2016
//
///////////////////////////////////////////////////////////////////////////////
#include "BitmapRW.h"

#include "PlatformDefinitions.h"
#include "ApplicationDefinitions.h"
#include "SpaceDisplay.h"
#include "SpaceTypes.h"
#include <inttypes.h>
#include "ap_bmp.h"

BitmapRW::BitmapRW(sc_core::sc_module_name name, double period, sc_core::sc_time_unit unit, uint8_t id, uint8_t priority, bool verbose)
:SpaceBaseModule(name, period, unit, id, priority, verbose) {
	SC_THREAD(thread);
	set_stack_size(0x16000+(IMG_SIZE*4));
}


void BitmapRW::thread(void) {
	const char * inName = "../../../../../../import/img/test_1080p.bmp";
	const char * outName = "../../../../../../results/img/result_1080p.bmp";
	const char * goldenModel = "../../../../../../import/img/result_1080p_golden.bmp";

	uint8_t *R = (uint8_t *)malloc(IMG_SIZE * sizeof(uint8_t));
	uint8_t *G = (uint8_t *)malloc(IMG_SIZE * sizeof(uint8_t));
	uint8_t *B = (uint8_t *)malloc(IMG_SIZE * sizeof(uint8_t));
	uint8_t *Y = (uint8_t *)malloc(IMG_SIZE * sizeof(uint8_t));

	// Fill a frame with data
	int read_tmp = BMP_Read(inName, IMG_HEIGHT, IMG_WIDTH, R, G, B);
	if(read_tmp != 0) {
		SpacePrint("%s Loading image failed\n", inName);
		sc_stop();
	}

	/* Communications avec les autres modules */

	// On envoie les array R, G et B au module RGBtoBW.
	message_t message = {
			MSG_RGB_TO_BW,
			(cmd_param_t)R, (cmd_param_t)G, (cmd_param_t)B
	};
	ModuleWrite(RGBTOBW_ID, SPACE_BLOCKING, &message);

	ModuleRead(SOBEL_ID, SPACE_BLOCKING, &message);
	if(message.command_type != MSG_BMP_WRITE) {
		SpacePrint("[BitmapRW] Invalid message type: %d\n", message.command_type);
		sc_stop();
	}
	// SpacePrint("[BitmapRW] About to copy the Y (received from Sobel).\n");
	uint8_t *Sob = (uint8_t *)message.param0;
	memcpy(Y, Sob, IMG_SIZE);

	//Write the image back to disk
	int write_tmp = BMP_Write(outName, IMG_HEIGHT, IMG_WIDTH, Y, Y, Y);
	if(write_tmp != 0){
		SpacePrint("WriteBMP %s failed\n", outName);
		sc_stop();
	}
	free(R);
	free(G);
	free(B);
	free(Y);

	SpacePrint("Simulation Complete\n");
	char diffStr[1000];
	snprintf(diffStr, sizeof(diffStr), "%s %s %s", "diff ", outName, goldenModel);
	int check_results = system(diffStr);
	if(check_results != 0){
		SpacePrint("Output image has mismatches with reference output image!\n");
	} else {
		SpacePrint("Output image matches the reference output image\n");
	}

	//End the simulation
	sc_stop();
}
