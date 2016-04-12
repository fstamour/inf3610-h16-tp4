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

	MyPrint("\n[BitmapRW] Executing.\n");

	const char * outName = PATH_TO_RESULT "result.bmp";

	//const char * inputImage = PATH_TO_FILES "test_1080p.bmp";
	//const char * inputImage = PATH_TO_FILES "test_500x500.bmp";
	const char * inputImage = PATH_TO_FILES "test_100x100.bmp";
	//const char * inputImage = PATH_TO_FILES "test_36x36.bmp";
	//const char * goldenModel = PATH_TO_FILES "result_1080p_golden.bmp";
	//const char * goldenModel = PATH_TO_FILES "result_500x500_golden.bmp";
	const char * goldenModel = PATH_TO_FILES "result_100x100_golden.bmp";
	//const char * goldenModel = PATH_TO_FILES "result_36x36_golden.bmp";


	MyPrint("[BitmapRW] About to malloc things.\n");
	uint8_t *R = (uint8_t *)malloc(IMG_SIZE * sizeof(uint8_t));
	uint8_t *G = (uint8_t *)malloc(IMG_SIZE * sizeof(uint8_t));
	uint8_t *B = (uint8_t *)malloc(IMG_SIZE * sizeof(uint8_t));
	uint8_t *Y = (uint8_t *)malloc(IMG_SIZE * sizeof(uint8_t));

	// Fill a frame with data
	MyPrint("[BitmapRW] About to load the image.\n");
	int read_tmp = BMP_Read(inputImage, IMG_HEIGHT, IMG_WIDTH, R, G, B);
	if(read_tmp != 0) {
		SpacePrintIfNotMonitoring("Loading image failed\n");
		sc_stop();
	}

	/*À COMPLÉTER: Communications avec les autres modules */

	// On envoie les array R, G et B au module RGBtoBW.
	MyPrint("[BitmapRW] Sending...\n");
	ModuleWrite(RGBTOBW_ID, SPACE_BLOCKING, R, IMG_SIZE);
	ModuleWrite(RGBTOBW_ID, SPACE_BLOCKING, G, IMG_SIZE);
	ModuleWrite(RGBTOBW_ID, SPACE_BLOCKING, B, IMG_SIZE);

	MyPrint("[BitmapRW] Receiving...\n");
	ModuleRead(SOBEL_ID, SPACE_BLOCKING, Y, IMG_SIZE);


#if !defined(SIMTEK) || !defined(SPACE_SIMULATION_MONITORING)
	//Write the image back to disk
	MyPrint("[BitmapRW] About to write the resulting bitmap.\n");
	int write_tmp = BMP_Write(outName, IMG_HEIGHT, IMG_WIDTH, Y, Y, Y);
	if(write_tmp != 0){
		SpacePrintIfNotMonitoring("WriteBMP %s failed\n", outName);
		sc_stop();
	}

	MyPrint("[BitmapRW] Diff'ing...\n");
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

	MyPrint("[BitmapRW] Done, about to free stuff.\n");
	free(R);
	free(G);
	free(B);
	free(Y);


	SpacePrintIfNotMonitoring("Simulation Complete\n");
	waitIfNotMonitoring(100);

	//End the simulation
	sc_stop();
}
