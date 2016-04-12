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
#ifndef HUFF_H
#define HUFF_H

#include "ApplicationDefinitions.h"
#include "SpaceDataTypes.h"
#include "SpaceBaseModule.h"
#include "definition_jpeg.h"

#include "systemc"

typedef struct DC_HUFFMAN_STRUCTURE {
	space_int17	valptr_minus_mincode_table [17];
	space_int17	maxcode_table [17];
	space_uint5	huffsize_table [256];
	UINT8	huffval_table [12];
} DC_HUFFMAN_STRUCTURE;


typedef struct AC_HUFFMAN_STRUCTURE {
	space_int17	valptr_minus_mincode_table [17];
	space_int17	maxcode_table [17];
	space_uint5	huffsize_table [256];
	UINT8	huffval_table [162];
} AC_HUFFMAN_STRUCTURE;

class HUFF: public SpaceBaseModule {
	public:

		SC_HAS_PROCESS(HUFF);

		HUFF(sc_core::sc_module_name name, double period, sc_core::sc_time_unit unit, unsigned char id, unsigned char priority, bool verbose);

		void thread_huffman();

	private:

		void communication_write64pixelblock(unsigned int, INT16*);
		void initialize();

										// second field is for validity of address
		space_uint3  m_state;				// state of the jpeg decompression
	
		space_uint22	m_inputAdd;				// latest jpeg data address, multiple of 4 and 0x20 afetr header
	
		UINT8 launch_huffman();
		//
		//  Huffman
		//

		DC_HUFFMAN_STRUCTURE dc_huffman_structure[2];
		AC_HUFFMAN_STRUCTURE ac_huffman_structure[2];
		UINT32 lcode;
		space_uint5 lsize;
		INT16 ldc[3];
		INT32		getHuffmanTableElement(int selector, int id, int index);
		UINT16		comm_read_16huff_bits();
	
		space_uint2		m_currentHuffmanComponent;

	
		//
		// JPEG related
		//
		UINT8		read_dht_marker();
		UINT8		read_8_bits();
		UINT16		read_16_bits();
		UINT8 		read_huffman_table(space_int17* valptr_minus_mincode_table, space_int17* maxcode_table, space_uint5* huffsize_table, UINT8* huffval_table, space_uint10& huffman_table_definition_length);
};

#endif

/* (c) Space Codesign Systems Inc. 2005-2011 */
