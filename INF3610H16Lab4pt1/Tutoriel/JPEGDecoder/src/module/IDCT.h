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
#ifndef IDCT_H
#define IDCT_H

#include "ApplicationDefinitions.h"
#include "SpaceDataTypes.h"
#include "SpaceBaseModule.h"

#include "systemc"

#define JPEG_QUANTTBLSIZE	64

class IDCT: public SpaceBaseModule {
	public:

		SC_HAS_PROCESS(IDCT);

		IDCT(sc_core::sc_module_name name, double period, sc_core::sc_time_unit unit, unsigned char id, unsigned char priority, bool verbose);
		
		void thread_inverse_dct();

	private:

		bool m_endFlagReceived;
	
		space_uint22 m_currentTempBufferAddress;
		space_uint22 m_lastMCUStartAddress;
	
		// QUANT TABLE ELEMENTSt
		short m_interblock[64];	// F(u,v) block
};

#endif

/* (c) Space Codesign Systems Inc. 2005-2011 */
