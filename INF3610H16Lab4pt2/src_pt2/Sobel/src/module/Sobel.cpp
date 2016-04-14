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


inline uint8_t sobel_operator(const uint8_t Y[3][3])
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

	const int8_t coefs_x[3][3] =
			{{-1,0,1},
             {-2,0,2},
             {-1,0,1}};
	const int8_t coefs_y[3][3] =
			{{-1,-2,-1},
             {0,0,0},
             {1,2,1}};


	L3: for(int i = 0; i < 3; ++i) {
	#pragma HLS unroll
		L4: for(int j = 0; j < 3; ++j) {
	#pragma HLS unroll
			// DEBUG printf("x: %d y: %d   cx: %d cy: %d\n", i-1, j-1, coefs_x[i][j], coefs_y[i][j]);
			/*x_weight += Y[index + (j-1)*IMG_WIDTH + (i-1)] * coefs_x[i][j];
			y_weight += Y[index + (j-1)*IMG_WIDTH + (i-1)] * coefs_y[i][j];*/
			x_weight += Y[i][j] * coefs_x[i][j];
			y_weight += Y[i][j] * coefs_y[i][j];
		}
	}


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

	while(1) {
		//MyPrint("[Sobel] executing...\n");

		ModuleRead(RGBTOBW_ID, SPACE_BLOCKING, Y, IMG_SIZE);

		//MyPrint("[Sobel] Applying the filter...\n");
		L1: for(int x = 1; x < IMG_WIDTH - 1; ++x) {
			L2: for(int y = 1; y < IMG_HEIGHT -1; ++y) {
				int index = y * IMG_WIDTH + x;

				uint8_t tmp[3][3];
				L5: for(int i = 0; i < 3; ++i) {
					#pragma HLS unroll
					L6: for(int j = 0; j < 3; ++j) {
					#pragma HLS unroll
						tmp[i][j] = Y[index + (j-1)*IMG_WIDTH + (i-1)];
					}
				}

				int edge_val = sobel_operator(index, tmp);
				Sob[index] = edge_val;
			}
		}
		//computeFor(441982);
		computeFor(38421);

		//MyPrint("[Sobel] Sending bitmap...\n");
		ModuleWrite(BITMAPRW_ID, SPACE_BLOCKING, Sob, IMG_SIZE);
	}
}
