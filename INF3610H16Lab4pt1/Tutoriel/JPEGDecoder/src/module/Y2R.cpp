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
#include "Y2R.h"
#include "SpaceDisplay.h"
#include "PlatformDefinitions.h"

Y2R::Y2R(sc_core::sc_module_name name, double period, sc_core::sc_time_unit unit, unsigned char id, unsigned char priority, bool verbose)
:SpaceBaseModule(name, period, unit, id, priority, verbose) {
	SC_THREAD(thread_convert);
}

void Y2R::determineOutputAddress() {
    m_imageNo++;
    m_outputAddress = (m_imageNo-1)%2 == 1 ?  BITMAP_IMAGE_SPACER : 0;
}

//////////////////////////////////////////////////////////////////////////////
/// 
/// upon execution, this tyhread reads data from the address given and transforms
/// YUV420 format obtained from JPEG decompression to RGB format, for further
/// processing.
///
//////////////////////////////////////////////////////////////////////////////
void Y2R::thread_convert() {
	space_uint8 i;
	JMSG msg;
	space_uint22 input_address;
	
	m_nb_blocks = 0;
	m_imageNo = 0;
	
	i = 0;
	input_address = 0;

	while (1)
	{

	    if (GetVerbose())
	        SpacePrint("Y2R: waiting for command from IDCT\n");


		// blocking read from space channel - wait for authorisation to process
		// one MCU block
		// authorisation is given on every access to that module, data read is address of pixel block
		ModuleRead(IDCT_ID, SPACE_BLOCKING, &msg);

		if (msg.command != SCMD_CONVERT_MCU)
		{
			SpacePrint("Y2R ERROR : command unsupported - rejected and stoppping");	
			sc_stop();
		}
		
		// end of block conversion - respond to requests for acknowledge if requested
		// this is a request for acknowledge only
		if (msg.param0 == (int)TRUE)
		{
			msg.command = SCMD_CONVERT_MCU_END;
			ModuleWrite(IDCT_ID, SPACE_BLOCKING, &msg);
			computeFor(1);
			continue;
		}

		// when nb parameters is 2 - we receive a start_address indicating a new image
		// first time we process generates some extra processing
		//

		if (GetVerbose())
	        SpacePrint("Y2R: start command received - processing \n");

		computeFor(1);

		if (msg.param1 != 0) 
		{
		
            // collect input_address - where is the YUV data to convert ?
			input_address = msg.param1;

			// based on how many iamges we have processed - generate the address where to output 
			// teh converted RGB pixels
			determineOutputAddress();

			//
			// read data from memory - image resolution
			//
			DeviceRead(BITMAPRAM_ID, m_outputAddress, &width);
			DeviceRead(BITMAPRAM_ID, m_outputAddress + 0x4, &height);
						
			// calculate the number of mcu and the basic increment for data handling
			m_horizontal_mcu	= (width + 15) >> 4;
			m_vertical_mcu		= (height + 15) >> 4;
			//m_increment1		= 8; 
			m_increment2		= ((8 * (width & 0xfffe)) - (m_horizontal_mcu) * 8);

		    // point after IMAGE HEADER
		    m_outputAddress += INFO_IMAGE_HEADER;
		    m_vga_address = 0;

            if (GetVerbose())
            {
                SpacePrint("Y2R : New YUV2RGB conversion - reading from 0x%x\n", msg.param1);
                SpacePrint("Y2R : W:%d; H:%d; HMCU:%d; VMCU:%d\n", width, height, m_horizontal_mcu, m_vertical_mcu);
                SpacePrint("Y2R : output address : 0x%x\n", m_outputAddress);
            }

		} // end of extra processing for first time called


		//
		// process conversion 
		// read data from memory - 6 mCU to properly process pixels
		//

		space_uint22 cb_address = input_address + 512; // each block contains 64 2-byte pixels (128 bytes), and the CB block is the 5th one
		space_uint22 cr_address = input_address + 640;
		space_uint4 cindex;
		UINT32 RGBWord;
		space_uint4 j;
		space_uint22 tempaddr = m_outputAddress;

		for(i=0; i<16; i++) {

			// read 8 CB and 8 CR (for an output of 2 16-pixel lines)
			if(i%2 == 0) {
				DeviceRead(BITMAPRAM_ID, cb_address, &CB[0], 8);
				DeviceRead(BITMAPRAM_ID, cr_address, &CR[0], 8);
				cb_address += 16;
				cr_address += 16;
			}

			cindex = 0;

			for(j=0; j<8; j+= 2) {
				DeviceRead(BITMAPRAM_ID, input_address, &Y[0], 2);
				RGBWord = yuv2rgb(Y[0], CB[cindex], CR[cindex]);
				DeviceWrite(VGA_CONTROLLER_ID, m_vga_address, &RGBWord);
				m_vga_address += 4;
				m_outputAddress += 4;

				RGBWord = yuv2rgb(Y[1], CB[cindex], CR[cindex]);
				DeviceWrite(VGA_CONTROLLER_ID, m_vga_address, &RGBWord);
				m_vga_address += 4;
				m_outputAddress += 4;
	
				cindex++;
				input_address += 4;
			}

			input_address += 112; // (64-8)*2 

			for(j=0; j<8; j+= 2) {
				DeviceRead(BITMAPRAM_ID, input_address, &Y[0], 2);
				RGBWord = yuv2rgb(Y[0], CB[cindex], CR[cindex]);
				DeviceWrite(VGA_CONTROLLER_ID, m_vga_address, &RGBWord);
				m_vga_address += 4;
				m_outputAddress += 4;

				RGBWord = yuv2rgb(Y[1], CB[cindex], CR[cindex]);
				DeviceWrite(VGA_CONTROLLER_ID, m_vga_address, &RGBWord);
				m_vga_address += 4;
				m_outputAddress += 4;
	
				cindex++;
				input_address += 4;
			}

			if(i != 7) {
				input_address -= 128;
			}

			m_outputAddress += (width-16) *0x4ul; // verified;
			m_vga_address += (width-16) *0x4ul;
		}

		input_address += 384;

		if (GetVerbose())
			SpacePrint("Y2R: wrote 4:2:0 format to memory\n");

		
		// data pointer positioning - end of MCU
		m_outputAddress = tempaddr + m_increment1 *4 *2; // *2 added for bitmap conversion
														 // *4 added for memory alignment
		m_vga_address = (tempaddr-0x20) + m_increment1 *4 *2;

		// count in total number of blocks processed
		m_nb_blocks++;

		// position pointer into image

		if (m_nb_blocks == m_horizontal_mcu)
		{
			m_nb_blocks = 0;
			m_outputAddress += m_increment2 *4 *2; // *2 added for bitmap conversion
			m_vga_address += m_increment2 *4 *2;
												   
			// *4 added for memory alignment
		}

	    if (GetVerbose())
			SpacePrint("Y2R : End of Y2R conversion - waiting for next block\n");

		computeFor(1);

	} 
}


//
//  Y2R::ceilRGBValues
//////////////////////////////////////////////////////////////////////////////
/// 
/// part of the YUV2RGB conversion, this function verfies if RGB values are within
/// valid range of 0-255 and corrects them if necessary
/// 
/// @param  =>  *R : pointers to RGB values to verify - these valsues are corrceted
/// @param  =>  *G : 
/// @param  =>  *B : 
///
/// @return =>  None.
///
/// @note   =>  
//////////////////////////////////////////////////////////////////////////////
unsigned int Y2R::yuv2rgb(space_int9 y, space_int9 cb, space_int9 cr) {
	unsigned int RGBWord;
	space_int16 R, G, B;
	
	R = ((y << 10) + 1436 * (cr -128)) >> 10;
	G = ((y << 10) - 352  * (cb -128) - 731 * (cr-128)) >> 10;
	B = ((y << 10) + 1815 * (cb -128)) >> 10;	
	
	if (R < 0)
		R = 0;
	else if (R > 255)
		R = 255;
	
	if (G < 0)
		G = 0;
	else if (G > 255)
		G = 255;
	
	if (B < 0)
		B = 0;
	else if (B > 255)
		B = 255;
		
	//	
	// pixels are stored as such : 0x00BBGGRR
	//
	RGBWord = B;
	RGBWord <<= 8;
	RGBWord |= G;
	RGBWord <<= 8;
	RGBWord |= R;
		
	computeFor(1);
	//SpacePrint("y = %x, cb = %x, cr = %x\n", y, cb, cr);
	//SpacePrint("R = %x, G = %x, B = %x\n", R, G, B);
	
	return RGBWord;
}

/* (c) Space Codesign Systems Inc. 2005-2011 */
