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
#ifndef IQTZ_H
#define IQTZ_H

#include "ApplicationDefinitions.h"
#include "SpaceDataTypes.h"
#include "SpaceBaseModule.h"

#include "systemc"

#define JPEG_QUANTTBLSIZE	64

class IQTZ: public SpaceBaseModule {
	public:

		SC_HAS_PROCESS(IQTZ);

		IQTZ(sc_core::sc_module_name name, double period, sc_core::sc_time_unit unit, unsigned char id, unsigned char priority, bool verbose);
	
		void thread_inverse_quantize();
	
	private:

		short m_inblock[64];		// qF(u,v) block
		short m_interblock[64];	// F(u,v) block

		UINT8 QuantTable[2][JPEG_QUANTTBLSIZE];	// Luminance and Chrominance Matrix Quantization Table
};

#endif

/* (c) Space Codesign Systems Inc. 2005-2011 */
