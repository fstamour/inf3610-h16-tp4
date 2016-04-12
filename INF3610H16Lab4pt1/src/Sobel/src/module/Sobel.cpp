///////////////////////////////////////////////////////////////////////////////
//
// Filename : Sobel.cpp
//
// Creation date : Sat Mar 12 14:54:39 EST 2016
//
///////////////////////////////////////////////////////////////////////////////
#include "Sobel.h"

#include "PlatformDefinitions.h"
#include "ApplicationDefinitions.h"
#include "SpaceDisplay.h"
#include "SpaceTypes.h"

#include <cstring> // For memcpy

#define ABS(x)          ((x>0)? x : -x)

Sobel::Sobel(sc_core::sc_module_name name, double period, sc_core::sc_time_unit unit, unsigned char id, unsigned char priority, bool verbose)
:SpaceBaseModule(name, period, unit, id, priority, verbose) {
	SC_THREAD(thread);
	set_stack_size(0x16000+(IMG_SIZE*3));
}


uint8_t Sobel::sobel_operator(const int index)
{
	int x_weight = 0;
	int y_weight = 0;

	unsigned edge_weight;
	uint8_t edge_val;

	/* Noyau de l'algorithme */

	// index = y * width + x; Donc:
	// y = index % IMG_WIDTH;
	// x = index - y * IMG_WIDTH;

	// P/r a index
	// -IMG_WIDTH ==> y - 1
	// +IMG_WIDTH ==> y + 1
	// -1 ==> x -1
	// +1 ==> x +1

	// Compute Gx, Gy
	x_weight = (Y[index - IMG_WIDTH - 1] * -1) + (Y[index - IMG_WIDTH] * -2) + (Y[index - IMG_WIDTH + 1] * -1)
			 + (Y[index + IMG_WIDTH - 1] *  1) + (Y[index + IMG_WIDTH] *  2) + (Y[index + IMG_WIDTH + 1] *  1);

	y_weight = (Y[index - IMG_WIDTH - 1] * -1) + (Y[index -1] * -2) + (Y[index + IMG_WIDTH -1] * -1)
			 + (Y[index - IMG_WIDTH + 1] *  1) + (Y[index +1] *  2) + (Y[index + IMG_WIDTH + 1] *  1);

	// "Compute" |G|
	edge_weight = ABS(x_weight) + ABS(y_weight);
	edge_val = (255-(uint8_t)(edge_weight));

	//Edge thresholding
	if(edge_val > 200)
		edge_val = 255;
	else if(edge_val < 100)
		edge_val = 0;

	return edge_val;
}

void Sobel::thread(void) {

	//We want to have the images on SystemC's stack while being able to access them from other member functions
	uint8_t Ystatic[IMG_SIZE];
	uint8_t SobStatic[IMG_SIZE];
	Y = Ystatic;
	Sob = SobStatic;

	while(1) {
		SpacePrint("[Sobel] executing...\n");

		ModuleRead(RGBTOBW_ID, SPACE_BLOCKING, Y, IMG_SIZE);

		SpacePrint("[Sobel] Applying the filter...\n");
		for(int x = 1; x < IMG_WIDTH - 1; ++x) {
			for(int y = 1; y < IMG_HEIGHT -1; ++y) {
				int index = y * IMG_WIDTH + x;
				int edge_val = sobel_operator(index);
				Sob[index] = edge_val;
			}
		}

		SpacePrint("[Sobel] Sending bitmap...\n");
		ModuleWrite(BITMAPRW_ID, SPACE_BLOCKING, Sob, IMG_SIZE);
	}
}
