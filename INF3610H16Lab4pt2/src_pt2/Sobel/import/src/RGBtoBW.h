///////////////////////////////////////////////////////////////////////////////
//
// Filename : RGBtoBW.h
//
// Creation date : Thu Mar 24 11:09:20 EDT 2016
//
///////////////////////////////////////////////////////////////////////////////
#ifndef RGBTOBW_H
#define RGBTOBW_H

#include "SpaceBaseModule.h"

#include "systemc"

class RGBtoBW: public SpaceBaseModule {
	public:
		
	    SC_HAS_PROCESS(RGBtoBW);
		
		RGBtoBW(sc_core::sc_module_name name, double period, sc_core::sc_time_unit unit, unsigned char id, unsigned char priority, bool verbose);
	   	
	    void thread(void);
};

#endif
