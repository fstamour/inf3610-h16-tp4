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

#define ABS(x)          ((x>0)? x : -x)

Sobel::Sobel(sc_core::sc_module_name name, double period, sc_core::sc_time_unit unit, unsigned char id, unsigned char priority, bool verbose)
:SpaceBaseModule(name, period, unit, id, priority, verbose) {
	SC_THREAD(thread);
	set_stack_size(0x16000+(IMG_SIZE*2));
}


uint8_t Sobel::sobel_operator(const int index, const uint8_t * Y)
{
	int x_weight = 0;
	int y_weight = 0;

	unsigned edge_weight;
	uint8_t edge_val;

	/* À COMPLÉTER: noyau de l'algorithme */

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

	uint8_t Y[IMG_SIZE];
	uint8_t Sob[IMG_SIZE];

	message_t message;

	while(1) {
		MyPrint("[Sobel] executing...\n");

		for(int i = 0; i < IMG_SIZE; ++i) {
			ModuleRead(RGBTOBW_ID, SPACE_BLOCKING, &message);
			// TODO Check the message type.
			Y[i] = (uint8_t)message.param0;
		}

		MyPrint("[Sobel] Applying the filter...\n");
		for(int x = 1; x < IMG_WIDTH - 1; ++x) {
			for(int y = 1; y < IMG_HEIGHT -1; ++y) {
				int index = y * IMG_WIDTH + x;
				int edge_val = sobel_operator(index, Y);
				Sob[index] = edge_val;
			}
		}

		MyPrint("[Sobel] Sending bitmap...\n");
		memset(&message, 0, sizeof(message));
		message.command_type = MSG_BMP_WRITE;
		for(int i = 0; i < IMG_SIZE; ++i) {
			message.param0 = (cmd_param_t)Sob[i];
			ModuleWrite(BITMAPRW_ID, SPACE_BLOCKING, &message);
		}

		computeFor(1);
	}
}
