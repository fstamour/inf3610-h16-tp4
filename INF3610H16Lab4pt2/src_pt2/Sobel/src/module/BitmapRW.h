///////////////////////////////////////////////////////////////////////////////
//
// Filename : BitmapRW.h
//
// Creation date : Sat Mar 12 14:54:30 EST 2016
//
///////////////////////////////////////////////////////////////////////////////
#ifndef BitmapRW_H
#define BitmapRW_H

#include "SpaceBaseModule.h"

#include "systemc"

class BitmapRW: public SpaceBaseModule {
	public:
		
	    SC_HAS_PROCESS(BitmapRW);
		
		BitmapRW(sc_core::sc_module_name name, double period, sc_core::sc_time_unit unit, unsigned char id, unsigned char priority, bool verbose);
	   	
	    void thread(void);
};

#endif
