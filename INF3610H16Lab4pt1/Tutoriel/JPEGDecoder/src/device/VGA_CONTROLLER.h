///////////////////////////////////////////////////////////////////////////////
//
// Filename : VGA_CONTROLLER.cpp
//
// Creation date : Tue Sep 30 21:29:09 EDT 2014
//
///////////////////////////////////////////////////////////////////////////////
#define FAST_FORWARD 1

#ifndef VGA_CONTROLLER_H_
#define VGA_CONTROLLER_H_

#include "SpaceBaseDevice.h"
#include "SpaceBaseMasterDevice.h"
#include "BasicVGAController.h"

#include "systemc"

#include <vector>

#ifdef WIN32
	#include <windows.h>
#endif
#include "VGADefinitions.h"

class VGA_CONTROLLER : public SpaceBaseDevice {
	public:
	
		SC_HAS_PROCESS(VGA_CONTROLLER);

		VGA_CONTROLLER(sc_core::sc_module_name name, double period, sc_core::sc_time_unit unit, unsigned char id, bool verbose);
		~VGA_CONTROLLER();

		virtual void SlaveRead(tlm::tlm_generic_payload& trans, sc_core::sc_time& delay);
		virtual void SlaveWrite(tlm::tlm_generic_payload& trans, sc_core::sc_time& delay);

		void end_of_simulation();

	private:
		void play_frames();
		void create_capture();
		void refresh();
		void save_bitmap();
	
		sc_core::sc_time m_refresh_rate;
		bool m_is_ready;
		BasicVGAController m_vga_controller;
		std::vector< rgba** > m_frames;
		rgba** m_current_frame;
		unsigned long m_frame_count;
	
		#if FAST_FORWARD == 1
			sc_core::sc_event  m_frame_completed;
		#endif
};

#endif // VGA_CONTROLLER_H_
