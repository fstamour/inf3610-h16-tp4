///////////////////////////////////////////////////////////////////////////////
//
// Filename : VGA_CONTROLLER.cpp
//
// Creation date : Tue Sep 30 21:29:09 EDT 2014
//
///////////////////////////////////////////////////////////////////////////////
#include "VGA_CONTROLLER.h"
#include "SpaceDisplay.h"
#include "PlatformDefinitions.h"
#include "ApplicationDefinitions.h"

#include <iostream>

#include "tlm"

#define REFRESH_RATE 1
#define HEIGHT_VGA 128
#define WIDTH_VGA 128

using namespace std;

VGA_CONTROLLER::VGA_CONTROLLER(sc_core::sc_module_name name, double period, sc_core::sc_time_unit unit, unsigned char id, bool verbose)
:SpaceBaseDevice(name, sc_core::sc_time(period, unit), id, verbose), m_is_ready(true), m_frame_count(0), m_vga_controller(WIDTH_VGA, HEIGHT_VGA, REFRESH_RATE) {
	m_current_frame = new rgba*[HEIGHT_VGA];

	for(int i=0; i<HEIGHT_VGA; i++)
		m_current_frame[i] = new rgba[WIDTH_VGA];

	// Screen is black by default
	for (int i=0; i<HEIGHT_VGA; i++)
		for (int j=0; j<WIDTH_VGA; j++) {
			m_current_frame[i][j].red = 0;
			m_current_frame[i][j].green = 0;
			m_current_frame[i][j].blue = 0;
			m_current_frame[i][j].alpha = 0;
		}

	m_refresh_rate = sc_core::sc_time(double((double)1/(double)REFRESH_RATE), sc_core::SC_SEC);

	SC_THREAD(refresh);
	
	set_stack_size(0x64000);
}

void VGA_CONTROLLER::end_of_simulation() {
	play_frames();

	if (GetVerbose())
		save_bitmap();
}

VGA_CONTROLLER::~VGA_CONTROLLER() {
	vector< rgba** >::iterator iter;

	if (m_current_frame != 0) {
		for(int i=0; i<HEIGHT_VGA; i++)
			delete [] m_current_frame[i];
		delete [] m_current_frame;
	}

	for (iter=m_frames.begin(); iter < m_frames.end(); iter++) {
		for(int i=0; i<HEIGHT_VGA; i++)
				delete [] (*(iter))[i];
			delete [] *(iter);
	}
}

void VGA_CONTROLLER::save_bitmap() {
	int frame = 0;
	for (vector< rgba** >::iterator iter=m_frames.begin(); iter < m_frames.end(); iter++)
		m_vga_controller.save_bitmap(*iter, frame++);
}

void VGA_CONTROLLER::play_frames() {
	create_capture();
/*#ifndef SPACE_SIMULATION_MONITORING
	while(m_vga_controller.can_continue()) {
		if (m_vga_controller.can_animate()) {
			for (vector< rgba** >::iterator iter=m_frames.begin(); iter < m_frames.end() && m_vga_controller.can_continue(); iter++) {
				m_vga_controller.display_frame(*iter);
				m_vga_controller.synchronize();
			}
		}
		Sleep(1);
	}
#endif*/
}

void VGA_CONTROLLER::SlaveRead(tlm::tlm_generic_payload& trans, sc_core::sc_time& delay) {
	unsigned long* data = (unsigned long*)trans.get_data_ptr();
	
	if (GetRelativeAddress(trans) == 0)
		*data = m_is_ready ? 1 : 0;
	else
		*data = m_frame_count;
	
	trans.set_response_status(tlm::TLM_OK_RESPONSE);
	delay += GetClockPeriod();
}

void VGA_CONTROLLER::SlaveWrite(tlm::tlm_generic_payload& trans, sc_core::sc_time& delay) {
	unsigned int row = 0;
	unsigned int column = 0;
	unsigned long data = *(unsigned long*)trans.get_data_ptr();
	unsigned long offset = GetRelativeAddress(trans);
	unsigned long current_pixel = 0;

	// Are we done receiving the frame ?
	if (offset == FRAME_DONE) {
		m_is_ready = false;
		m_frame_count++;
		
		#if FAST_FORWARD == 1
			m_frame_completed.notify();
		#endif
	}
	else {
		current_pixel = offset / 4;
		row = current_pixel / WIDTH_VGA;
		column = current_pixel - (row * WIDTH_VGA);

		// Assign current pixel
		m_current_frame[row][column].red = data & 0xFF;
		m_current_frame[row][column].green = (data >> 8) & 0xFF;
		m_current_frame[row][column].blue = (data >> 16) & 0xFF;
		m_current_frame[row][column].alpha = (data >> 24) & 0xFF;
	
		if (GetVerbose() && column == 0 && (row % 8) == 0)
			m_vga_controller.display_frame(m_current_frame);
	}
			
	trans.set_response_status(tlm::TLM_OK_RESPONSE);
	delay += GetClockPeriod();
}

void VGA_CONTROLLER::refresh() {
	while(1) {
		#if FAST_FORWARD == 1
			wait(m_frame_completed);
		#else
			wait(m_refresh_rate);
		#endif

		create_capture();
	}
}

void VGA_CONTROLLER::create_capture() {
	rgba** tblPixel;
	tblPixel = new rgba*[HEIGHT_VGA];

	for(int i=0; i<HEIGHT_VGA; i++)
		tblPixel[i] = new rgba[WIDTH_VGA];

	for (int i=0; i<HEIGHT_VGA; i++)
		for (int j=0; j<WIDTH_VGA; j++) {
			tblPixel[i][j].red = m_current_frame[i][j].red;
			tblPixel[i][j].green = m_current_frame[i][j].green;
			tblPixel[i][j].blue = m_current_frame[i][j].blue;
			tblPixel[i][j].alpha = m_current_frame[i][j].alpha;
		}

	m_frames.push_back( tblPixel );
	m_is_ready = true;
}
