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




#include "SpaceSerialization.h"

using sc_core::sc_module;

Sobel::Sobel(sc_core::sc_module_name name)
:sc_module(name) {
	SC_CTHREAD(thread, ClockPort.pos());
	reset_signal_is( nResetPort, true );
}







inline uint8_t sobel_operator(const int index, const uint8_t Y[3][3])
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

/*
	x_weight = (Y[index - IMG_WIDTH - 1] * -1) + (Y[index - IMG_WIDTH] * -2) + (Y[index - IMG_WIDTH + 1] * -1)
			 + (Y[index + IMG_WIDTH - 1] *  1) + (Y[index + IMG_WIDTH] *  2) + (Y[index + IMG_WIDTH + 1] *  1);

	y_weight = (Y[index - IMG_WIDTH - 1] * -1) + (Y[index -1] * -2) + (Y[index + IMG_WIDTH -1] * -1)
			 + (Y[index - IMG_WIDTH + 1] *  1) + (Y[index +1] *  2) + (Y[index + IMG_WIDTH + 1] *  1);
*/

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

 edge_weight = ((x_weight>0)? x_weight : -x_weight) + ((y_weight>0)? y_weight : -y_weight);

 edge_val = (255-(uint8_t)(edge_weight));

 //Edge thresholding
 if(edge_val > 200)
  edge_val = 255;
 else if(edge_val < 100)
  edge_val = 0;

 return edge_val;
}

void Sobel::thread(void) {

 uint8_t Y[(100 * 100)];
 uint8_t Sob[(100 * 100)];


	MakeSignalsInactive();
	sc_core::wait();


	sc_core::wait();

 while(1) {
  //MyPrint("[Sobel] executing...\n");

  ModuleRead(1, SPACE_BLOCKING, Y, (100 * 100));

  //MyPrint("[Sobel] Applying the filter...\n");
  L1: for(int x = 1; x < 100 - 1; ++x) {
   L2: for(int y = 1; y < 100 -1; ++y) {
   //#pragma HLS pipeline
    int index = y * 100 + x;
    uint8_t tmp[3][3];
    L5: for(int i = 0; i < 3; ++i) {
#pragma HLS unroll
     L6: for(int j = 0; j < 3; ++j) {
#pragma HLS unroll
      tmp[i][j] = Y[index + (j-1)*100 + (i-1)];
     }
    }
    int edge_val = sobel_operator(index, tmp);
    Sob[index] = edge_val;
   }
  }
  //computeFor(441982);
  computeFor(38421);

  //MyPrint("[Sobel] Sending bitmap...\n");
  ModuleWrite(0, SPACE_BLOCKING, Sob, (100 * 100));
 }
}
static unsigned int getNbBeats(const unsigned char* type, unsigned long nb_elements) {
	return 1*nb_elements/4;
}


#define SPACE_BEGIN_DATA_BEAT
#define SPACE_END_DATA_BEAT

	eSpaceStatus ModuleRead_1(Sobel* module, unsigned long timeout,  unsigned char* data, unsigned long nb_elements)

{
	unsigned int nb_iterations = getNbBeats(data, nb_elements);
	unsigned int i;
	eSpaceStatus status;

	bool has_sample = !!module->fifo_in_0.num_available();

	if (timeout == SPACE_WAIT_FOREVER || has_sample) {

		for(i=0;i<nb_iterations;i++) {

		SPACE_BEGIN_DATA_BEAT;
		sc_dt::sc_uint<32> temp = module->fifo_in_0.read();
		GET_CHAR1(temp, *data);
		data++;
		GET_CHAR2(temp, *data);
		data++;
		GET_CHAR3(temp, *data);
		data++;
		GET_CHAR4(temp, *data);
		data++;
		SPACE_END_DATA_BEAT;

		}
		status = SPACE_OK;
	} else
		status = SPACE_EMPTY;
	return status;

}

#undef SPACE_BEGIN_DATA_BEAT
#undef SPACE_END_DATA_BEAT

#define SPACE_BEGIN_DATA_BEAT
#define SPACE_END_DATA_BEAT

	eSpaceStatus ModuleWrite_0(Sobel* module, unsigned long timeout, const unsigned char* data, unsigned long nb_elements)

{
	unsigned int nb_iterations = getNbBeats(data, nb_elements);
	unsigned int i;
	eSpaceStatus status;

	bool has_free_slot = !!module->fifo_out_0.num_free();

	if(timeout == SPACE_WAIT_FOREVER || has_free_slot) {

		for(i=0;i<nb_iterations;i++) {

		sc_dt::sc_uint<32> temp = 0;

		SPACE_BEGIN_DATA_BEAT;
		PUT_CHAR1(temp, *data);
		data++;
		PUT_CHAR2(temp, *data);
		data++;
		PUT_CHAR3(temp, *data);
		data++;
		PUT_CHAR4(temp, *data);
		data++;
		module->fifo_out_0.write(temp);
		SPACE_END_DATA_BEAT;

		}
		status = SPACE_OK;
	} else
		status = SPACE_FULL;

	return status;
}

#undef SPACE_BEGIN_DATA_BEAT
#undef SPACE_END_DATA_BEAT




template <typename T> eSpaceStatus Sobel::ModuleRead (
		unsigned char destination_id, unsigned long timeout, T* data, unsigned long nb_elements) {
	if(destination_id == 1) {
		return ModuleRead_1(this, timeout, data, nb_elements);
	} else
	return SPACE_ERROR;
}


template <typename T> eSpaceStatus Sobel::ModuleWrite (
		unsigned char destination_id, unsigned long timeout, T* data, unsigned long nb_elements) {
	if(destination_id == 0) {
		return ModuleWrite_0(this, timeout, data, nb_elements);
	} else
	return SPACE_ERROR;
}


template <typename T> eSpaceStatus Sobel::DeviceRead (
		unsigned char destination_id, unsigned long offset, T* data,  unsigned long nb_elements) {
	return SPACE_ERROR;
}


template <typename T> eSpaceStatus Sobel::DeviceWrite (
		unsigned char destination_id, unsigned long offset, T* data,  unsigned long nb_elements) {
	return SPACE_ERROR;
}

eSpaceStatus Sobel::RegisterRead(unsigned char register_file_id, unsigned long register_id,  unsigned long* data) {
	return SPACE_ERROR;
}

eSpaceStatus Sobel::RegisterWrite(unsigned char register_file_id, unsigned long register_id, const unsigned long* data) {
	return SPACE_ERROR;
}

void Sobel::MakeSignalsInactive() {





}
