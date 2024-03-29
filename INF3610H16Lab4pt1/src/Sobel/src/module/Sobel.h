///////////////////////////////////////////////////////////////////////////////
//
// Filename : Sobel.h
//
// Creation date : Sat Mar 12 14:54:39 EST 2016
//
///////////////////////////////////////////////////////////////////////////////
#ifndef SOBEL_H
#define SOBEL_H

#include "SpaceBaseModule.h"
#include <inttypes.h>
#include "systemc"
#include "ApplicationDefinitions.h"

class Sobel: public SpaceBaseModule {
	public:
		
	    SC_HAS_PROCESS(Sobel);
		
		Sobel(sc_core::sc_module_name name, double period, sc_core::sc_time_unit unit, unsigned char id, unsigned char priority, bool verbose);
	   	
	    void thread(void);
	private:
		uint8_t sobel_operator(const int index);
		uint8_t * Y;
		uint8_t * Sob;
};

#endif
