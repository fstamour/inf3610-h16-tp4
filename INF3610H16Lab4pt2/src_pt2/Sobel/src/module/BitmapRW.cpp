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
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

BitmapRW::BitmapRW(sc_core::sc_module_name name, double period, sc_core::sc_time_unit unit, uint8_t id, uint8_t priority, bool verbose)
:SpaceBaseModule(name, period, unit, id, priority, verbose) {
	SC_THREAD(thread);
#ifdef ELIX
	set_stack_size(0x16000+(IMG_SIZE*4));
#endif
}

void BitmapRW::thread(void) {
#ifdef SIMTEK
	#define PATH_TO_FILES "/home/root/tb/"
	#define PATH_TO_RESULT PATH_TO_FILES
#else
	#define PATH_TO_FILES "../../../../../../import/img/"
	#define PATH_TO_RESULT "../../../../../../results/img/"
#endif

	const char * outName = PATH_TO_RESULT "result.bmp";

	const char * inputImage = PATH_TO_FILES "test_1080p.bmp";
	//const char * inputImage = PATH_TO_FILES "test_500x500.bmp";
	//const char * inputImage = PATH_TO_FILES "test_100x100.bmp";
	//const char * inputImage = PATH_TO_FILES "test_36x36.bmp";
	const char * goldenModel = PATH_TO_FILES "result_1080p_golden.bmp";
	//const char * goldenModel = PATH_TO_FILES "result_500x500_golden.bmp";
	//const char * goldenModel = PATH_TO_FILES "result_100x100_golden.bmp";
	//const char * goldenModel = PATH_TO_FILES "result_36x36_golden.bmp";


	uint8_t *R = (uint8_t *)malloc(IMG_SIZE * sizeof(uint8_t));
	uint8_t *G = (uint8_t *)malloc(IMG_SIZE * sizeof(uint8_t));
	uint8_t *B = (uint8_t *)malloc(IMG_SIZE * sizeof(uint8_t));
	uint8_t *Y = (uint8_t *)malloc(IMG_SIZE * sizeof(uint8_t));

	// Fill a frame with data
	int read_tmp = BMP_Read(inputImage, IMG_HEIGHT, IMG_WIDTH, R, G, B);
	if(read_tmp != 0) {
		SpacePrintIfNotMonitoring("Loading image failed\n");
		sc_stop();
	}

	/*À COMPLÉTER: Communications avec les autres modules */
	message_t message = {0};

	// On envoie les array R, G et B au module RGBtoBW.
	MyPrint("\nAbout to send RGB to RGBtoBW.\n");
	for(int i = 0; i < IMG_SIZE; ++i) {
		message.command_type = MSG_RGB_TO_BW;
		message.param0 = (cmd_param_t)R[i];
		message.param1 = (cmd_param_t)G[i];
		message.param2 = (cmd_param_t)B[i];
		ModuleWrite(RGBTOBW_ID, SPACE_BLOCKING, &message);
	}

	for(int i = 0; i < IMG_SIZE; ++i) {
		ModuleRead(SOBEL_ID, SPACE_BLOCKING, &message);
		// TODO Check the message type.
		Y[i] = (uint8_t)message.param0;
	}
	if(message.command_type != MSG_BMP_WRITE) {
		SpacePrintIfNotMonitoring("[BitmapRW] Invalid message type: %d\n", message.command_type);
		sc_stop();
	}
	// MyPrint("[BitmapRW] About to copy the Y (received from Sobel).\n");
	uint8_t *Sob = (uint8_t *)message.param0;
	memcpy(Y, Sob, IMG_SIZE);


#if !defined(SIMTEK) || !defined(SPACE_SIMULATION_MONITORING)
	//Write the image back to disk
	int write_tmp = BMP_Write(outName, IMG_HEIGHT, IMG_WIDTH, Y, Y, Y);
	if(write_tmp != 0){
		SpacePrintIfNotMonitoring("WriteBMP %s failed\n", outName);
		sc_stop();
	}

	char diffStr[1000];
	snprintf(diffStr, sizeof(diffStr), "diff %s %s", outName, goldenModel);
	int check_results = system(diffStr);
	if(check_results != 0){
		SpacePrintIfNotMonitoring("Output image has mismatches with reference output image!\n");
	}
	else{
		SpacePrintIfNotMonitoring("Output image matches the reference output image\n");
	}
#endif

	free(R);
	free(G);
	free(B);
	free(Y);


	SpacePrintIfNotMonitoring("Simulation Complete\n");
	waitIfNotMonitoring(100);

	//End the simulation
	sc_stop();
}
