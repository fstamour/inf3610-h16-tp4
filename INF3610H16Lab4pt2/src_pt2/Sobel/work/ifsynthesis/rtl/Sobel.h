///////////////////////////////////////////////////////////////////////////////
//
// Filename : Sobel.h
//
// Creation date : Sat Mar 12 14:54:39 EST 2016
//
///////////////////////////////////////////////////////////////////////////////
#ifndef SOBEL_H
#define SOBEL_H

#include <inttypes.h>

#include "SpaceBaseModule.h"
#include "systemc"


class Sobel: public sc_core::sc_module {
	public:
		
	    SC_HAS_PROCESS(Sobel);
		
	sc_core::sc_in< bool > nResetPort;
	sc_core::sc_in< bool > ClockPort;
	sc_core::sc_fifo_in< sc_dt::sc_lv<32> > fifo_in_0;
	sc_core::sc_fifo_out< sc_dt::sc_lv<32> > fifo_out_0;

public:

	Sobel(sc_core::sc_module_name name);

	// Timing Annotation added for monitoring of computation time
    void computeFor(int nbCycles) {}

    /// Returns the verbose module status.
    bool GetVerbose() {return false;}

    ///
	void sc_stop(void) {}

	template <typename T> eSpaceStatus ModuleRead(unsigned char destination_id, unsigned long timeout, T* data, unsigned long nb_elements = 1);
	template <typename T> eSpaceStatus ModuleWrite(unsigned char destination_id, unsigned long timeout, T* data, unsigned long nb_elements = 1);

	template <typename T> eSpaceStatus DeviceRead(unsigned char destination_id, unsigned long ulOffset, T* data, unsigned long nb_elements = 1);
    template <typename T> eSpaceStatus DeviceWrite(unsigned char destination_id, unsigned long offset, T* data, unsigned long nb_elements = 1);

	eSpaceStatus RegisterRead(unsigned char register_file_id, unsigned long register_id, unsigned long* data);
    eSpaceStatus RegisterWrite(unsigned char register_file_id, unsigned long register_id, const unsigned long* data);

	void MakeSignalsInactive();


	    void thread(void);
	private:
		//uint8_t sobel_operator(const int index, const uint8_t * Y);
};

#endif