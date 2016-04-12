///////////////////////////////////////////////////////////////////////////////
///
///         Space Libraries - Space Codesign Systems Inc. (http://www.spacecodesign.com)
///         (c) All Rights Reserved. 2005-2014
///                                                                       
///         This file contains proprietary, confidential information of Space Codesign 
///         Systems Inc. and may be used only pursuant to the terms of a valid license 
///         agreement with Space Codesign Systems Inc. For more information about licensing,
///         please contact your Space Codesign representative. 
/// 
///////////////////////////////////////////////////////////////////////////////
#ifndef Y2R_H
#define Y2R_H

#include "ApplicationDefinitions.h"
#include "SpaceDataTypes.h"
#include "SpaceBaseModule.h"

#include "systemc"

class Y2R: public SpaceBaseModule {
public:
	
    SC_HAS_PROCESS(Y2R);
	
	Y2R(sc_core::sc_module_name name, double period, sc_core::sc_time_unit unit, unsigned char id, unsigned char priority, bool verbose);

    void thread_convert();

private:
	
	space_uint22 m_outputAddress;
	unsigned long m_vga_address;

	// internal methods
	void write_420_format();

	void determineOutputAddress();
	unsigned int yuv2rgb(space_int9, space_int9, space_int9);

	space_uint8 m_nb_blocks;
	space_uint4 m_imageNo;

	space_uint8 m_horizontal_mcu;
	space_uint8 m_vertical_mcu;
	#define m_increment1 8
	//INT32 m_increment1;		
	space_uint11 m_increment2;

	INT32 width;
	INT32 height;

	INT16 Y[2];
	INT16 CB[8];
	INT16 CR[8];
};

#endif

/* (c) Space Codesign Systems Inc. 2005-2011 */
