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
#include "HUFF.h"
#include "SpaceDisplay.h"
#include "ApplicationDefinitions.h"
#include "PlatformDefinitions.h"

static const DC_HUFFMAN_STRUCTURE ZERO_DC_HUFFMAN_STRUCTURE = {0};
static const AC_HUFFMAN_STRUCTURE ZERO_AC_HUFFMAN_STRUCTURE = {0};

HUFF::HUFF(sc_core::sc_module_name name, double period, sc_core::sc_time_unit unit, unsigned char id, unsigned char priority, bool verbose)
:SpaceBaseModule(name, period, unit, id, priority, verbose) {
	SC_THREAD(thread_huffman);
}

void HUFF::thread_huffman() {
	JMSG inmsg;
	JMSG outmsg;

	space_uint2 m_nbtable; // add to member variable
	space_uint16 m_nbMCU;
	
    // initiate state of decompression
    m_state = JPEG_STARTED;

	m_nbtable = 0; // add to member variable
	m_nbMCU = 0;

	// reset to 0 structures and data related to jpeg decompression
	initialize();
	
	while (1)
	{
		computeFor(2);

		ModuleRead(EXTR_ID, SPACE_BLOCKING, &inmsg);
		
		switch (inmsg.command)
		{
		case SCMD_HUFFMAN_DHT: // read for huffman tables
			
			m_inputAdd = inmsg.param0;
			
			read_dht_marker();
			
			m_nbtable++;

			if (GetVerbose())
                SpacePrint("HUFF: retreived DHT table no %d sucecssfully!\n", m_nbtable);
			
			// positive ack - return pointer
			outmsg.command = SCMD_ACK;
			outmsg.param0 = m_inputAdd;
			computeFor(1);
			ModuleWrite(EXTR_ID, SPACE_BLOCKING, &outmsg);

			lsize = 0;
			lcode = 0;
			ldc[0] = ldc[1] = ldc[2] = 0;

			m_nbMCU = 0;
			
			break;
			
			//
		case SCMD_HUFFMAN_MCU:

			if (GetVerbose())
					SpacePrint("HUFF: HUFFMAN_MCU received from EXTRACTOR - launching Huffman decompression\n");
			
			m_currentHuffmanComponent = inmsg.param0;
			m_inputAdd = inmsg.param1;
			
			launch_huffman();
			
			m_nbMCU++;
			//SpacePrint("%s: HUFFMAN has retreived MCU block no %d successfully!\n", this->name(), m_nbMCU);
			
			// positive ack - return pointer
			outmsg.command = SCMD_ACK;
			outmsg.param0 = m_inputAdd;
			computeFor(1);
			ModuleWrite(EXTR_ID, SPACE_BLOCKING, &outmsg);

			if (GetVerbose())
				SpacePrint("HUFF: ACK returned to EXTRACTOR\n");
			
			m_nbtable = 0;

			break;
			
		 case SCMD_END_IMAGE:
			computeFor(1);
            ModuleWrite(IQTZ_ID, SPACE_BLOCKING, &inmsg);
            break;
		}
	}
}

/*------------------------------------------------------------------------------
* read_8_bits()
* 
* Read 8 bits of data from jpeg chunck
*
* ret:	8 bits value read
*------------------------------------------------------------------------------*/
UINT8 HUFF::read_8_bits() {
	UINT8 inputValue;
	SPACE_ALIGNED UINT8 inputBuffer[4];

	UINT8 alignment = (m_inputAdd & 0x3);
	DeviceRead(JPEGRAM_ID, m_inputAdd & 0xFFFFFFFC, &inputBuffer[0], 4);
	inputValue = inputBuffer[alignment];
	
	m_inputAdd+=1;

	return inputValue;

}

/*------------------------------------------------------------------------------
* read_16_bits()
* 
* Read 16 bits of data from jpeg chunck
*
* ret:	16 bits value read
*------------------------------------------------------------------------------*/
UINT16 HUFF::read_16_bits() {
	UINT16 value;
	UINT8 reader1;
	UINT8 reader2;


	reader1 = read_8_bits();
	reader2 = read_8_bits();

	value = reader1 << 8;
	value = value | reader2;

	return (value);
}



////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
//////////////////  HUFFMAN THREADS AND OPERATIONS /////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

/*------------------------------------------------------------------------------
* thread_huffman()
* 
* This method extracts the jpeg data from memory and uncompress it using 
* jpeg huffman decompression. Uncompressed data is grouped in blocks of
* 64 pxels and transmitted to inverse_quantize process/module
*
* This was originally a thread which was transformed into a single function
* thread code has been commented out
*
*--------------- ---------------------------------------------------------------*/
UINT8 HUFF::launch_huffman() {
	SPACE_ALIGNED INT16 block[64];
	space_uint7 j;
	space_uint2 selector;
	space_uint2 component_index; // value changes from 0 to 2 ==> 3 possibilities

	space_int12 dc, ac;
	space_uint7 coefficient;
	UINT16 huffcode;
	space_uint8 index;
	space_uint8 huffval;
	space_uint5 huffsize;
	space_uint4 run_length, size;

    // clear buffer we use
    for (j=0; j<64; j++)
        block[j] = 0;
    
    //
    // PROCESS DC COMPONENT
    //
    // 1. READ COMPONENT INDEX EITHER LUMINANCE or CHROMINANCE
    component_index = m_currentHuffmanComponent;
    
    // 2. READ TABLE ELEMENTS HERE 
    if (component_index == 0)
        selector = HUFFTABLE_DCL;
    else
        selector = HUFFTABLE_DCC;
    
    // READ 16 BITS FROM IMAGE FIFO
    if (lsize < 16)
    {
        lcode |= comm_read_16huff_bits()  << (16 - lsize);
        lsize += 16;
    }
    
    huffsize = getHuffmanTableElement(selector, HUFF_HUFFSIZE, lcode >> (32 - NBITS));
    
    if (huffsize)
        huffcode = lcode >> (32 - huffsize);
    else
    {        
		for(huffsize = NBITS + 1; huffsize <= 16; huffsize++) 
        {
            huffcode = lcode >> (32 - huffsize);
			if(huffcode <= getHuffmanTableElement(selector, HUFF_MAXCODE, huffsize)) {
				break;
			}
        }
        
        if (huffsize > 16)
            return ERROR_INVALHUFF_ID_CODE;
    }
    
    index = huffcode + getHuffmanTableElement(selector, HUFF_MINCODE, huffsize);
    huffval = getHuffmanTableElement(selector, HUFF_HUFFVAL, index);
    
    lcode <<= huffsize;
    lsize -= huffsize;
    
    if (lsize < huffval)
    {
        lcode |= comm_read_16huff_bits() << (16 - lsize);
        lsize += 16;
    }
    
    if (huffval == 0)
        dc = 0;
    else
        dc = lcode >> (32 - huffval);
    
    lcode <<= huffval;
    lsize -= huffval;
    
    if (dc < (1 << (huffval - 1)))
        dc += (- 1 << huffval) + 1;
    
    //
    // Write DC element into output block
    //
    
    block [0] = dc + ldc [component_index];
    
    ldc [component_index] = block[0];
    
    
    //
    // PROCESS AC COMPONENT
    //
    
    if (component_index == 0)
        selector = HUFFTABLE_ACL;
    else
        selector = HUFFTABLE_ACC;
    
    for (coefficient =  1; coefficient < 64; coefficient ++)
    {
        if (lsize < 16)
        { 
            lcode |= comm_read_16huff_bits() << (16 - lsize);
            lsize += 16;
        }
        
        huffsize = getHuffmanTableElement(selector, HUFF_HUFFSIZE, lcode >> (32 - NBITS));
        
        if (huffsize)
            huffcode = lcode >> (32 - huffsize);
        else
        {
            for(huffsize = NBITS + 1; huffsize <= 16; huffsize++)
			{
				huffcode = lcode >> (32 - huffsize);
				if(huffcode <= getHuffmanTableElement(selector, HUFF_MAXCODE, huffsize)) {
					break;
				}
			}
            
            if (huffsize > 16)
                return ERROR_INVALHUFF_ID_CODE;
        }
        
        index = huffcode + getHuffmanTableElement(selector, HUFF_MINCODE, huffsize);
        huffval = getHuffmanTableElement(selector, HUFF_HUFFVAL, index);
        
        lcode <<= huffsize;
        lsize -= huffsize;
        
        run_length = huffval >> 4;
        size = huffval & 15;
        
        if (size)
        {
            coefficient += run_length;
            
            if (lsize < size)
            {
                lcode |= comm_read_16huff_bits() << (16 - lsize);
                
                lsize += 16;
            }
            
            ac = lcode >> (32 - size);
            
            lcode <<= size;
            lsize -= size;
            
            if (ac < (1 << (size - 1)))
                ac += (- 1 << size) + 1;
            
            block [coefficient] = ac;
        }
        else
        {
            if (run_length != 15)
                break;
            
            coefficient += 15;
        }
    }
    
    computeFor(25);
    
    communication_write64pixelblock(component_index, block);

	return SUCCESS;
	
}

/*------------------------------------------------------------------------------
* communication_write64pixelblock()
* 
* prepares communication of a 64 element block to IJPEG module for inverse 
* quantization and inverse DCT
*
* input:	component_index is parameter2 to be transferred to IJPEG module
* input:	pblock is a pointer to block of data to be transferred
*
* ret:		SUCCESS
*------------------------------------------------------------------------------*/
void HUFF::communication_write64pixelblock(unsigned int component_index, INT16 *pblock) {
	if (GetVerbose())
		SpacePrint("HUFF: Transferring 64 decompressed pixels to IQTZ\n");

	//
	// send type of table to be transferred
	//

	JMSG myjpegmessage;
	myjpegmessage.command	= SCMD_BEGIN_COMP_DATA_TRANSMISSION;
	myjpegmessage.param0	= component_index;

	computeFor(1);

	ModuleWrite(IQTZ_ID, SPACE_BLOCKING, &myjpegmessage);

	computeFor(1);

	//
	// multiple byte transfer of table
	//	
	for (UINT8 i=0; i<64; i+= BURST_SIZE_SHORT)
	{
		//SpacePrint("OUT = %x, %x\n", pblock[i], pblock[i+1]);
		if (GetVerbose())
			SpacePrint("HUFF: transmitted pixels %d-%d/64 to IQTZ \n", i+1, i+BURST_SIZE_SHORT);
		computeFor(1);
		ModuleWrite(IQTZ_ID, SPACE_BLOCKING, &pblock[i], BURST_SIZE_SHORT);
	}
}


/*------------------------------------------------------------------------------
* comm_read_16huff_bits()
* 
* 
*
* ret:	
*------------------------------------------------------------------------------*/
UINT16 HUFF::comm_read_16huff_bits()
{
	UINT8 val1, val2;

	// read first value 
	
	val1 = read_8_bits(); 
	if (val1 == 0xff)
		read_8_bits();

	val2 = read_8_bits(); 
	if (val2 == 0xff)
		read_8_bits();

	return (val1 << 8) | val2;
}

/*------------------------------------------------------------------------------
* getHuffmanTableElement()
* 
* 
*
* ret:	
*------------------------------------------------------------------------------*/

INT32 HUFF::getHuffmanTableElement(int selector, int id, int index)
{
	int ACDC = 0;
	int MCU_TYPE = 0;

	switch ( selector )
		{
			case HUFFTABLE_ACL :
				ACDC = AC;
				MCU_TYPE = LUMINANCE;
				break;

			case HUFFTABLE_ACC:
				ACDC = AC;
				MCU_TYPE = CHROMINANCE;
				break;
			
			case HUFFTABLE_DCL:
				ACDC = DC;
				MCU_TYPE = LUMINANCE;
				break;
			
			case HUFFTABLE_DCC:
				ACDC = DC;
				MCU_TYPE = CHROMINANCE;
				break;
		}

		switch (id)
		{
			case HUFF_HUFFVAL:
				if(ACDC == DC)
					return ( dc_huffman_structure[MCU_TYPE].huffval_table[index] );
				else
					return ( ac_huffman_structure[MCU_TYPE].huffval_table[index] );
				break;
			case HUFF_HUFFSIZE:
				if(ACDC == DC)
					return( dc_huffman_structure[MCU_TYPE].huffsize_table[index] );
				else
					return( ac_huffman_structure[MCU_TYPE].huffsize_table[index] );
				break;
			case HUFF_MINCODE:
				if(ACDC == DC)
					return( dc_huffman_structure[MCU_TYPE].valptr_minus_mincode_table[index] );
				else
					return( ac_huffman_structure[MCU_TYPE].valptr_minus_mincode_table[index] );
				break;
			case HUFF_MAXCODE:
				if(ACDC == DC)
					return( dc_huffman_structure[MCU_TYPE].maxcode_table[index] );
				else
					return( ac_huffman_structure[MCU_TYPE].maxcode_table[index] );
				break;
		}

		computeFor(1);

		// patch to fix
		return 0;
}




/*------------------------------------------------------------------------------
* initialize()
* 
*
* ret:	
*--------------- ---------------------------------------------------------------*/
void HUFF::initialize()
{

	// initialize to zero arrays and tables
	dc_huffman_structure[0] = dc_huffman_structure[1] = ZERO_DC_HUFFMAN_STRUCTURE;
	ac_huffman_structure[0] = ac_huffman_structure[1] = ZERO_AC_HUFFMAN_STRUCTURE;

	lsize = 0;
	lcode = 0;
	ldc[0] = ldc[1] = ldc[2] = 0;

}


//
// HUFF::read_huffman_table
//////////////////////////////////////////////////////////////////////////////
///
/// Description
///
/// @param  =>  INT32*  valptr_minus_mincode_table : 
/// @param  =>  INT32*  maxcode_table : 
/// @param  =>  UINT8*  huffsize_table : 
/// @param  =>  UINT8*  huffval_table : 
/// @param  =>  UINT16&  huffman_table_definition_length : 
///
/// @return =>  UINT16  : 
///
/// @note   => 
//////////////////////////////////////////////////////////////////////////////
UINT8 HUFF::read_huffman_table(space_int17* valptr_minus_mincode_table, space_int17* maxcode_table, space_uint5* huffsize_table, UINT8* huffval_table, space_uint10& huffman_table_definition_length)
{		
	space_uint9 i;
	space_uint16 code;
	space_uint8 j, k, l, index, bits;
	
	k = 0;

	code = 0;
	index=0;

	for (i = 1; i <= NBITS; i ++)
	{
		bits = read_8_bits();

		for (j = 1; j <= bits; j++)
		{
			k++;

			code++;

			for (l = 0; l < 1 << (NBITS - i); l++)
			{
				huffsize_table [index++] = i;
			}
		}

		if (bits)
		{
			valptr_minus_mincode_table [i] = k - code;
			maxcode_table [i] = code - 1;
		}
		else
		
			maxcode_table [i] = - 1;

		code <<= 1;
	}

	for (i = index; i < (1 << NBITS); i++)
		huffsize_table [i] = 0;

	for (i=NBITS+1; i<=16; i++)
	{
		bits = read_8_bits();

		valptr_minus_mincode_table [i] = k - code;

		k += bits;
		code += bits;

		if (bits)
			maxcode_table [i] = code - 1;
		else
			maxcode_table [i] = - 1;

		code <<= 1;
	}

	//maxcode_table [17] = 0x1ffff;

	huffman_table_definition_length -= 1 + 16;

	if (k > 162 || k > huffman_table_definition_length)
		return ERROR_INVALID_NUMBER_OF_HUFFMAN_CODES;

	for (i=0; i<k; i++)
	{
		huffval_table [i] = read_8_bits();
	}

	huffman_table_definition_length -= k;

	return SUCCESS;	
}


/*------------------------------------------------------------------------------
* read_dht_marker()
* 
* Read Huffman Table  Marker
*
* ret:		
*------------------------------------------------------------------------------*/

UINT8 HUFF::read_dht_marker()
{
	space_uint10 huffman_table_definition_length;
	space_uint4 table_class, huffman_table_destination_identifier;
	space_uint8 tmp;

	// first value is size of table (-2 is the size of field "size" written into file)
	huffman_table_definition_length = read_16_bits() - 2;

	while (huffman_table_definition_length > 0)
	{
		//
		// read table class
		//
		tmp = read_8_bits();
		table_class = tmp >> 4;
		if (table_class > 1)
			return ERROR_INVALID_TABLE_CLASS;

		//
		// read table destination identifier
		//
		huffman_table_destination_identifier = tmp & 0xf;
		if (huffman_table_destination_identifier > 3)
			return ERROR_INVALHUFF_ID_TABLE_DESTINATION_IDENTIFIER;

		//
		// fill HUFFMAN table structure fields
		//	
		if(table_class == DC) {
			UINT8 status = read_huffman_table(
				dc_huffman_structure[huffman_table_destination_identifier].valptr_minus_mincode_table,
				dc_huffman_structure[huffman_table_destination_identifier].maxcode_table,
				dc_huffman_structure[huffman_table_destination_identifier].huffsize_table,
				dc_huffman_structure[huffman_table_destination_identifier].huffval_table,
				huffman_table_definition_length);
			if(status != SUCCESS) return status;
        } else {
			UINT8 status = read_huffman_table(
				ac_huffman_structure[huffman_table_destination_identifier].valptr_minus_mincode_table,
				ac_huffman_structure[huffman_table_destination_identifier].maxcode_table,
				ac_huffman_structure[huffman_table_destination_identifier].huffsize_table,
				ac_huffman_structure[huffman_table_destination_identifier].huffval_table,
				huffman_table_definition_length);
			if(status != SUCCESS) return status;      
		}   	
	}

	computeFor(huffman_table_definition_length+1);

	return SUCCESS;
}


/* (c) Space Codesign Systems Inc. 2005-2011 */
