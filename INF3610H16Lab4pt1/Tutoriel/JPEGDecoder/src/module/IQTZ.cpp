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
#include "IQTZ.h"
#include "SpaceDisplay.h"
#include "ApplicationDefinitions.h"
#include "PlatformDefinitions.h"

static const space_uint6 zigzag_table [] = {
	 0,  1,  8, 16,  9,  2,  3, 10,
	17, 24, 32, 25, 18, 11,  4,  5,
	12, 19, 26, 33, 40, 48, 41, 34,
	27, 20, 13,  6,  7, 14, 21, 28,
	35, 42, 49, 56, 57, 50, 43, 36,
	29, 22, 15, 23, 30, 37, 44, 51,
	58, 59, 52, 45, 38, 31, 39, 46,
	53, 60, 61, 54, 47, 55, 62, 63
}; 

IQTZ::IQTZ(sc_core::sc_module_name name, double period, sc_core::sc_time_unit unit, unsigned char id, unsigned char priority, bool verbose)
:SpaceBaseModule(name, period, unit, id, priority, verbose) {
	SC_THREAD(thread_inverse_quantize);
}


/*------------------------------------------------------------------------------
* thread_inverse_quantize()
* 
* inverse Inverse quantization processes Pixels retreived 
* from the channel. When 64 pixels are ready, they are transferred to an 
* out buffer for inverse DCT to start decompression
*
*
*------------------------------------------------------------------------------*/
void IQTZ::thread_inverse_quantize() {
	bool startTagReceived;
	space_uint22 infoImageHeaderAdd = 0;
	
	// to receive messages from other modules and to send messages to them
	JMSG input_message;
	JMSG output_message;

	// variables for INVERSE QUANTIFICATION computation
	space_uint6 index;
	space_uint16 blockCounter;
	
	space_uint2 componentindex = 0;

	startTagReceived = false;

	//	variables for INVERSE QUANTIFICATION computation
	blockCounter = 0;
	
	while(true)
    {
        // waiting for a command to be received
        
        if(!startTagReceived) {
			//
			// MESSAGES FROM EXTRACTOR
			//
			computeFor(5);
			ModuleRead(EXTR_ID, SPACE_BLOCKING, &input_message);
						
            //
            // process input command from the message
            //
            switch (input_message.command)
            {
      
            //////////////////////////////////////////////////////////////////////////////////////////
            //	2.	SCMD_NEW_IMAGE
            //
            case SCMD_NEW_IMAGE:

                //
                // calculate buffer and image memory address
                //
                //infoImageHeaderAdd =  ((input_message.param1-1)*BITMAP_IMAGE_SPACER);
				infoImageHeaderAdd = (input_message.param1-1)%2 == 1 ?  BITMAP_IMAGE_SPACER : 0; 
                        
                //
                // get width and height of image to generate and write to this image info header
                //
                int t1, t2;
				 
				DeviceRead(JPEGRAM_ID, input_message.param0 , &t1);
                DeviceWrite(BITMAPRAM_ID, infoImageHeaderAdd, &t1);
                DeviceRead(JPEGRAM_ID, input_message.param0+0x4ul, &t2);
                DeviceWrite(BITMAPRAM_ID, infoImageHeaderAdd+0x4ul, &t2);
                
                // get quantization table
                for (unsigned int i=0; i<JPEG_QUANTTBLSIZE/2; i++)
				{
					unsigned long ulQuantTblRegister;
					SPACE_REGISTER_RANGE(0,31);
					RegisterRead(REGISTERFILE1_ID, i, &ulQuantTblRegister);
					unsigned int uiTableID = i / (JPEG_QUANTTBLSIZE/4);
					unsigned int uiTableIndex = 4*i % (JPEG_QUANTTBLSIZE);
					QuantTable[uiTableID][uiTableIndex    ] = (ulQuantTblRegister >> 24) & 0xFF;
					QuantTable[uiTableID][uiTableIndex + 1] = (ulQuantTblRegister >> 16) & 0xFF;
					QuantTable[uiTableID][uiTableIndex + 2] = (ulQuantTblRegister >> 8 ) & 0xFF;
					QuantTable[uiTableID][uiTableIndex + 3] = (ulQuantTblRegister      ) & 0xFF;
					computeFor(1);
				}
                
                // print message
                if (GetVerbose())
                    SpacePrint("IQTZ: Request for new image (%dx%d) received - working at 0x%x\n", t1, t2, BITMAP_TEMP_ADDRESS);

                //
                // returning ack
                //
                input_message.command	= SCMD_NEW_IMAGE_ACK; 
                input_message.param0	= infoImageHeaderAdd;
                
				ModuleWrite(EXTR_ID, SPACE_BLOCKING, &input_message);
				computeFor(1);
				if (GetVerbose())
                    SpacePrint("IQTZ: ACK returned to EXTRACTOR after NEW_IMAGE command\n"); 

				// tag telling begin of transmission is received and 
                //command can be processed
                startTagReceived = true; 

                break;
            }
		}    
		else            
		{
			///
			//	MESSAGES FROM HUFFMAN
			//
			computeFor(5);
			ModuleRead(HUFF_ID, SPACE_BLOCKING, &input_message);
            
            //
            // process input command from the message
            //
            switch (input_message.command)
            {
                
                
            //////////////////////////////////////////////////////////////////////////////////////////
            //	3.	SCMD_BEGIN_COMP_DATA_TRANSMISSION
            //
            case SCMD_BEGIN_COMP_DATA_TRANSMISSION:	
                             
				if (GetVerbose())
					SpacePrint("IQTZ: BEGIN_COMP_DATA_TRANSMISSION received - ready to receive 64 pixels\n"); 

				// selection of proper table (LIGHT or COLOR table)
                componentindex = input_message.param0;
                if(componentindex > CHROMINANCE) componentindex = CHROMINANCE;
                
                //receiving pixels decompressed with Huffman
                for (int i=0; i<64; i+= BURST_SIZE_SHORT) {
	                ModuleRead(HUFF_ID, SPACE_BLOCKING, &m_inblock[i], BURST_SIZE_SHORT);
					if (GetVerbose())
						SpacePrint("IQTZ: COMP_DATA_TRANSMISSION received - collecting pixel %d-%d/64\n", i+1, i+BURST_SIZE_SHORT); 
                }
               
                blockCounter++;		//count the number of blocks received from EXTRACTOR       

				//------------------------------------------------------------
				//-------------- INVERSE QUANTIZE computation ----------------
				//-----------------------------------------------------------
				
				for (int k=0; k<64; k++)
                {
                    index = zigzag_table[k];
                    //SpacePrint("IN = %x\n", m_inblock[k]);    
                    // compute unquantized value 
                    m_interblock[index] = (m_inblock[k] * QuantTable[componentindex][k] );
                    //SpacePrint("OUT = %x\n", m_interblock[index]);    
                }
                                        
                // transmit SCMD_START_IDCT to begin inverse DCT operation
                output_message.command = SCMD_START_IDCT;
                output_message.param1 = blockCounter;
                
                if (blockCounter == 1)	//we transmit to IDCT the tempory address 
                    //for use with the storage of the block processed with inverse DCT
                {
                    output_message.param0 = BITMAP_TEMP_ADDRESS;
                }
                else	//for other blocks, don't send the tempory address
                {
					output_message.param0 = 0;
                }
                   
				if (GetVerbose())
					SpacePrint("IQTZ: 64 pixels processed - awaking IDCT\n"); 

				computeFor(5);
				
				ModuleWrite(IDCT_ID, SPACE_BLOCKING, &output_message);
                

                if (GetVerbose())
					SpacePrint("IQTZ: Ready to send 64 pixels to IDCT\n"); 

				//send the 64-elements block for inverse DCT
                for (int j=0; j<64; j+= BURST_SIZE_SHORT) {
					computeFor(1);
                    ModuleWrite(IDCT_ID, SPACE_BLOCKING, &m_interblock[j], BURST_SIZE_SHORT);
                }

				if (GetVerbose())
					SpacePrint("IQTZ: 64 pixels sucessfully sent\n"); 
                
                break;
             //////////////////////////////////////////////////////////////////////////////////////////
            //	5.	SCMD_END_IMAGE
            //
            case SCMD_END_IMAGE:
                
                if (GetVerbose())
                    SpacePrint("IQTZ: End of Image - Reseting for next image\n");

				//
				//waiting for last block to be processed before starting a new one
				//
				output_message.command = SCMD_END_IMAGE;
            
				// REQUEST FOR ACK

                ModuleWrite(IDCT_ID, SPACE_BLOCKING, &output_message);
		
				computeFor(1);

			    ModuleRead(IDCT_ID, SPACE_BLOCKING, &input_message);
            
				blockCounter = 0;

				output_message.command = SCMD_END_IMAGE_ACK;
                ModuleWrite(EXTR_ID, SPACE_BLOCKING, &output_message);
				computeFor(1);

                if (GetVerbose())
                    SpacePrint("IQTZ: ACK returned to EXTRACTOR for END_IMAGE command\n"); 

				// wait for next image
                startTagReceived = false; 

				break;
            
            default:
                break;             
            } // switch                
        } 
    } // while
}

/* (c) Space Codesign Systems Inc. 2005-2011 */
