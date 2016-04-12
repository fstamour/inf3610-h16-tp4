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
#include "IDCT.h"
#include "SpaceDisplay.h"
#include "ApplicationDefinitions.h"
#include "PlatformDefinitions.h"

static const space_uint8 ic1 = 251;
static const space_uint6 is1 = 50;
static const space_uint8 ic3 = 213;
static const space_uint7 is3 = 142;
static const space_uint9 ir2c6 = 277;
static const space_uint10 ir2s6 = 669;
static const space_uint8 ir2 = 181;
	
IDCT::IDCT(sc_core::sc_module_name name, double period, sc_core::sc_time_unit unit, unsigned char id, unsigned char priority, bool verbose)
:SpaceBaseModule(name, period, unit, id, priority, verbose) {
	SC_THREAD(thread_inverse_dct);

}

void IDCT::thread_inverse_dct() {
	JMSG input_message, output_message;
	
	space_uint16 blockCounter;

	//variables for INVERSE DCT computation
	INT16* dataptr;

	// maximum intermediate bit width is COL_SHIFT (11) + SAMPLE_SIZE(8) + 3
	// "Systematic Approach of Fixed Point 8x8 IDCT and DCT Design and Implementation"
	space_int22 x0, x1, x2, x3, x4, x5, x7, x8;
	space_int29 x6;
	
	// private member initailization
	space_uint3 m_mcuTransferred;
	
	m_mcuTransferred		= 0;
	
	blockCounter = 0;

	while (1)
	{

		if (GetVerbose())
	        SpacePrint("IDCT: waiting for command from IQTZ\n");

		
		//
		// wait for a block to be ready from IQUANT
		//
		ModuleRead(IQTZ_ID, SPACE_BLOCKING, &input_message);
	    
	    computeFor(1);
	    
		// this is the first time - store working address
        if ( input_message.param0 != 0 ) 
		{
			m_lastMCUStartAddress = input_message.param0;		//Stored for communicating to Y2R module
			m_currentTempBufferAddress = input_message.param0;	//Local Working pointer

			// reset members
			blockCounter = 0;
            
            if (GetVerbose())
                SpacePrint("IDCT: Starting New Image at %x\n", m_lastMCUStartAddress);
		}

		if(input_message.command == SCMD_START_IDCT)
		{

		    if (GetVerbose())
		        SpacePrint("IDCT: START_IDCT received - reading 64 pixels from IQTZ\n");

            //receiving of the 64-elements dequantized block from the IQUANT module
            for (unsigned char i=0; i<64; i+= BURST_SIZE_SHORT)
			{
				//SpacePrint("IN = %x, %x\n", m_interblock[i], m_interblock[i+1]);
				ModuleRead(IQTZ_ID, SPACE_BLOCKING, &m_interblock[i], BURST_SIZE_SHORT);
			}


			if (GetVerbose())
		        SpacePrint("IDCT: 64 pixels received - processing IDCT\n");

			//--------------------------------------------------
			//--------- INVERSE DCT computation --------------//
			//--------------------------------------------------

			// setting ptr to beginning of data array prepared by inverse_quantize
			dataptr = &m_interblock[0];
		
			for (UINT8 k = 0; k < 8; k ++)
			{
				x0 = (space_int11)(dataptr [0]) << 9;	// 20
				x1 = (space_int11)(dataptr [1]) << 7;	// 18
				x2 = (space_int11)(dataptr [2]);		// 11
				x3 = (space_int11)(dataptr [3]) * ir2;	// 19
				x4 = (space_int11)(dataptr [4]) << 9;	// 20
				x5 = (space_int11)(dataptr [5]) * ir2;	// 19
				x6 = (space_int11)(dataptr [6]);		// 11
				x7 = (space_int11)(dataptr [7]) << 7;	// 18
				x8 = x7 + x1;							// 19
				x1 -= x7;								// 19
				
				x7 = x0 + x4;							// 21
				x0 -= x4;								// 21
				x4 = x1 + x5;							// 20
				x1 -= x5;								// 20
				x5 = x3 + x8;							// 20
				x8 -= x3;								// 20
				x3 = ir2c6 * (x2 + x6);					// 21
				x6 = x3 + (- ir2c6 - ir2s6) * x6;		// 22
				x2 = x3 + (- ir2c6 + ir2s6) * x2;		// 22
				
				x3 = x7 + x2;							// 23
				x7 -= x2;								// 23
				x2 = x0 + x6;							// 23
				x0 -= x6;								// 23
				x6 = ic3 * (x4 + x5);					// 29
				x5 = (x6 + (- ic3 - is3) * x5) >> 6;	// 24
				x4 = (x6 + (- ic3 + is3) * x4) >> 6;	// 22
				x6 = ic1 * (x1 + x8);					// 28
				x1 = (x6 + (- ic1 - is1) * x1) >> 6;	// 24
				x8 = (x6 + (- ic1 + is1) * x8) >> 6;	// 23
				
				x7 += 512;								// 24
				x2 += 512;								// 24
				x0 += 512;								// 24
				x3 += 512;								// 24
				dataptr [0] = (INT16) ((x3 + x4) >> 10);
				dataptr [1] = (INT16) ((x2 + x8) >> 10);
				dataptr [2] = (INT16) ((x0 + x1) >> 10);
				dataptr [3] = (INT16) ((x7 + x5) >> 10);
				dataptr [4] = (INT16) ((x7 - x5) >> 10);
				dataptr [5] = (INT16) ((x0 - x1) >> 10);
				dataptr [6] = (INT16) ((x2 - x8) >> 10);
				dataptr [7] = (INT16) ((x3 - x4) >> 10);
				
				dataptr += 8;
			}
			
			dataptr -= 64;
			
			for (UINT8 l = 0; l < 8; l ++) 
			{
				x0 = dataptr [0] << 9;
				x1 = dataptr [8] << 7;
				x2 = dataptr [16];
				x3 = dataptr [24] * ir2;
				x4 = dataptr [32] << 9;
				x5 = dataptr [40] * ir2;
				x6 = dataptr [48];
				x7 = dataptr [56] << 7;
				x8 = x7 + x1;
				x1 -= x7;
				
				x7 = x0 + x4;
				x0 -= x4;
				x4 = x1 + x5;
				x1 -= x5;
				x5 = x3 + x8;
				x8 -= x3;
				x3 = ir2c6 * (x2 + x6);
				x6 = x3 + (- ir2c6 - ir2s6) * x6;
				x2 = x3 + (- ir2c6 + ir2s6) * x2;
				
				x3 = x7 + x2;
				x7 -= x2;
				x2 = x0 + x6;
				x0 -= x6;
				x4 >>= 6;
				x5 >>= 6;
				x1 >>= 6;
				x8 >>= 6;
				x6 = ic3 * (x4 + x5);
				x5 = (x6 + (- ic3 - is3) * x5);
				x4 = (x6 + (- ic3 + is3) * x4);
				x6 = ic1 * (x1 + x8);
				x1 = (x6 + (- ic1 - is1) * x1);
				x8 = (x6 + (- ic1 + is1) * x8);
				
				x7 += 1024;
				x2 += 1024;
				x0 += 1024;
				x3 += 1024;
				dataptr [0] = (INT16) ((x3 + x4) >> 11);
				dataptr [8] = (INT16) ((x2 + x8) >> 11);
				dataptr [16] = (INT16) ((x0 + x1) >> 11);
				dataptr [24] = (INT16) ((x7 + x5) >> 11);
				dataptr [32] = (INT16) ((x7 - x5) >> 11);
				dataptr [40] = (INT16) ((x0 - x1) >> 11);
				dataptr [48] = (INT16) ((x2 - x8) >> 11);
				dataptr [56] = (INT16) ((x3 - x4) >> 11);
				
				dataptr ++;
			}
				
			// values must be shifted and copied to outblock before written to memory
			for (UINT8 m=0; m<64; m++)
			{
				m_interblock[m] = m_interblock[m] + 128;

				if (m_interblock[m] < 0)
					m_interblock[m] = 0;
				else if (m_interblock[m] > 255)
					m_interblock[m] = 255;
			}

			computeFor(100);

			if (GetVerbose())
				SpacePrint("IDCT: outputing 64 processed picels to BITMAPRAM\n");


			// write inversed DCT data to memory at tempory address
			// data are transformed from 2 bytes to 4 bytes to correspond to the UTF_Channel operation
			for (unsigned char i=0; i<64; i+= BURST_SIZE_SHORT)
			{
				//SpacePrint("OUT = %x, %x\n", m_interblock[i], m_interblock[i+1]);
				DeviceWrite(BITMAPRAM_ID,  m_currentTempBufferAddress, &m_interblock[i], BURST_SIZE_SHORT);
    			m_currentTempBufferAddress+=2*BURST_SIZE_SHORT;
			}

			if (GetVerbose())
				SpacePrint("IDCT: 64 pixels written to BITMAPRAM\n");


			m_mcuTransferred++;		//to count the number of inverse DCT blocks
			//m_mcuTransferred = 6;		//to count the number of inverse DCT blocks

			computeFor(2);
		
			// COMMUNICATION TO Y2R MODULE
			// process total mcu count - when 6 blocks are process, 
            // Y1Y2Y3Y4CbCr can be processed
			if (m_mcuTransferred == 6)
			{
				m_mcuTransferred = 0;
				
				output_message.command = SCMD_CONVERT_MCU;
				output_message.param0 = false;
				
				//Collect the total number of dequantified blocks (received in control message)
				blockCounter = input_message.param1;				
				// if this is the first block of a series, give Y2V module the address
				if (blockCounter == 6) 
				{
					output_message.param1 = m_lastMCUStartAddress;
				}
				else
				{
					output_message.param1 = 0;
				}

				if (GetVerbose())
					SpacePrint("IDCT: total of 6 blocks processed - starting Y2R with request for ack:%d and blockCounter:%d\n", output_message.param0, blockCounter);

				// begin a YUV2RGB conversion 
				ModuleWrite(Y2R_ID, SPACE_BLOCKING, &output_message);

			}	//END IF(input_message.command == SCMD_START_IDCT)

	
		}
        else if (input_message.command == SCMD_END_IMAGE)
		{
			
            if (GetVerbose())
                SpacePrint("IDCT: End of Image\n", m_lastMCUStartAddress);

            // this is the last block the same series we process
			// request for sync with conv before we start anythign else
			output_message.command = SCMD_CONVERT_MCU;
            output_message.param0 = true; // this is request for ack
            
			// sync with Y2R module
			ModuleWrite(Y2R_ID, SPACE_BLOCKING, &output_message);

			computeFor(1);

			ModuleRead(Y2R_ID, SPACE_BLOCKING, &input_message);
		
			if (GetVerbose())
				SpacePrint("IDCT: ACK received from Y2R for END_OF_IMAGE command \n");
		}

		computeFor(1);

		// if a request for ack was requested - process here
		// notiufy IQUANT that we have finished processing the last MCU block
		if (input_message.command == SCMD_CONVERT_MCU_END)					
		{
			if (GetVerbose())
				SpacePrint("IDCT: CONVERT_END_MCU received - responding to IQTZ\n");
			
			
			output_message.command = SCMD_CONVERT_MCU_END;	
			ModuleWrite(IQTZ_ID, SPACE_BLOCKING, &output_message);
			computeFor(1);
		}


	}
}

/* (c) Space Codesign Systems Inc. 2005-2011 */
