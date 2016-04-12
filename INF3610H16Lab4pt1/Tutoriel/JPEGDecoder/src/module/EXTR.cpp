///////////////////////////////////////////////////////////////////////////////
///
///        SPACE Libraries - Space Codesign Systems Inc. (http://www.spacecodesign.com)
///         (c) All Rights Reserved. 2005-2014
///                                                                       
///         This file contains proprietary, confidential information of Space Codesign 
///         Systems Inc. and may be used only pursuant to the terms of a valid license 
///         agreement with Space Codesign Systems Inc. For more information about licensing,
///         please contact your Space Codesign representative. 
/// 
///////////////////////////////////////////////////////////////////////////////
#include "EXTR.h"
#include "SpaceDisplay.h"
#include "PlatformDefinitions.h"

static const JPEG_DECODER_STRUCTURE ZERO_JPEG_DECODER_STRUCTURE = {0};

EXTR::EXTR(sc_core::sc_module_name name, double period, sc_core::sc_time_unit unit, unsigned char id, unsigned char priority, bool verbose)
:SpaceBaseModule(name, period, unit, id, priority, verbose) {
	SC_THREAD(thread_extract_jpeg);
}

void EXTR::thread_extract_jpeg() {
	UINT8 last_marker;
	UINT8 last_returned;
	
    bool end_of_jpeg;
    bool has_at_least_one_jpeg_succeeded;
	
	JMSG outmsg;
	JMSG inmsg;
	
	m_currentJPEGImageNo = 0;
	m_currentBitmapImage = 0;
	m_bufferValid  = false;

	// represents the distance between each jpeg image
	m_jpegDist	= JPEG_IMAGE_SPACER;
	
    has_at_least_one_jpeg_succeeded = false;

	// reset to 0 structures and data related to jpeg decompression
	initialize();

	while(1) {
       end_of_jpeg = false;

		//
		// selection of jpeg preloop - sets pointer on proper image 
		//
        m_currentJPEGImageNo++; 
		if (m_currentJPEGImageNo > MAX_NB_IMAGE)
        {
        	// either we stop when we finish processing the images...
        	launch_face_detection();
        	sc_stop();
			
			// ...or we loop forever (comment out the previous line)
			m_currentJPEGImageNo = 1; 
            if (has_at_least_one_jpeg_succeeded == false)
            {
               SpacePrint("........................................................\n");
               SpacePrint("JPEG DECODER : ALL IMAGES SET FOR DECOMPRESSION\n");
               SpacePrint("HAVE FAILED -- STOPPING\n");
               SpacePrint("........................................................\n");
                sc_stop();
            }
        }

		m_infoImageHeaderAdd = (m_currentJPEGImageNo-1) * m_jpegDist; 	
		m_inputAdd = m_infoImageHeaderAdd + INFO_IMAGE_HEADER;
		m_bufferValid = false;
		computeFor(1);

        if (GetVerbose())
        {
           SpacePrint("EXTR: ********************************************** \n");
           SpacePrint("EXTR: Decompressing JPEG IMAGE no %d\n", m_currentJPEGImageNo);
           SpacePrint("EXTR: Input Address: 0x%x\n", m_inputAdd);
           SpacePrint("EXTR: ********************************************** \n");
        }
        else
        {
           SpacePrint("***JPEG %d\n",m_currentJPEGImageNo);
        }

		//
		// decode jpeg!
		//
		while (!end_of_jpeg) 
		{
			
			//
			// JPEG information is stored in between MARKERS. Each marker tells about 
			// the upcoming information to be read from the data
			//
			// this section scans markers and execute appropriate code according
			// to last marker read
			
			last_marker = read_next_marker();
			computeFor(1);
			
			switch (last_marker)
			{
				
			//
			// START OF FRAME MARKER
			//
			case SOF0:
			case SOF1:
			case SOF2:

                has_at_least_one_jpeg_succeeded = true;
				m_jpeg_decoder_structure.sof = last_marker;

				last_returned = read_sof_marker ();
				if (last_returned != SUCCESS)
				{
					SpacePrint("----------------------------------------\n");
					SpacePrint("EXTRACTOR: JPEG SOF DECODING ERROR no %d - skipping JPEG\n", last_returned); //, this->name(), last_returned);
					initialize();
					end_of_jpeg = true;
				}
					// write image header into memory
					// WORD 1 is X width
					// WORD 2 is Y height
					DeviceWrite(JPEGRAM_ID, m_infoImageHeaderAdd, &m_jpeg_decoder_structure.number_of_samples_per_line);
					DeviceWrite(JPEGRAM_ID, m_infoImageHeaderAdd+0x4ul, &m_jpeg_decoder_structure.number_of_lines);

	                if (GetVerbose())
                    {
					   SpacePrint("EXTR: JPEG STARTING DECOMPRESSION OF IMAGE no%d\n", m_currentJPEGImageNo);
                       SpacePrint("EXTR: Image size: %dx%d\n",
                                      m_jpeg_decoder_structure.number_of_samples_per_line, 
                                      m_jpeg_decoder_structure.number_of_lines);
                    }
                    else
                    {
					   SpacePrint("EXTR:JPEG%d\n", m_currentJPEGImageNo);
                    }


                    communicate_IJPEG(SCMD_NEW_IMAGE);
				//}
				
				break;
				

			//
			// START OF HUFFMAN TABLE MARKER AT 0xffc0
			//	
			case DHT: 
				
				outmsg.command = SCMD_HUFFMAN_DHT;
				outmsg.param0 = m_inputAdd;

				if (GetVerbose())
					SpacePrint("EXTR: Huffman Table found; sending address to HUFF module\n");

				ModuleWrite(HUFF_ID, SPACE_BLOCKING, &outmsg);

				computeFor(1);

				// wait for acknowledgement
				ModuleRead(HUFF_ID, SPACE_BLOCKING, &inmsg);

				if (GetVerbose())
					SpacePrint("EXTR: Return from blocking read - waiting Huffman ACK\n");

				
				// ack received with update with memory pointer
				if (inmsg.command == SCMD_ACK)
				{
					m_inputAdd = inmsg.param0;
					m_bufferValid = false;
				}
				else
				{
					SpacePrint("----------------------------------------\n");
					SpacePrint("EXTRACTOR: JPEG DHT DECODING ERROR no %d - skipping JPEG\n", inmsg.param0); //this->name(), inmsg.param0);
					initialize();
					end_of_jpeg = true;
				}
				break;

			//
			// START OF SCAN MARKER (DATA) AT 0xffda
			//
			case SOS:
				skip_marker ();	
				// extract file information
				/*
				last_returned = read_sos_marker ();
				if (last_returned != SUCCESS)
				{
					SpacePrint("----------------------------------------\n");
					SpacePrint("EXTRACTOR: JPEG SOS DECODING ERROR no %d - skipping JPEG\n", last_returned); //this->name(), last_returned);
					initialize();
					end_of_jpeg = true;
				}
				*/
				
				// this flys through the MCU and keep tracks of the current type of 
				// block being processed - for HUffman_thread. This should be 
				// optimised since it is always 4Y+CB+CR- TODO

				if (GetVerbose())
					SpacePrint("EXTR: Start of Scan Marker found... interleaving scan decoding can start\n");

				decode_interleaved_scan ();

				if (GetVerbose())
				{
					SpacePrint("EXTR: End of Scan Marker... waiting for other modules to finish\n");
				}


				// This is the end of the file, we terminate
				
				communicate_IJPEG(SCMD_END_IMAGE);


				
				// reinitialize buffers for jpeg decompression
				initialize();
				
				// we get off this loop and prepare for next image
				end_of_jpeg = true;

                // we launch execution of face detection - 
                // which emulates somne filters to render
                // new images
                launch_face_detection();
				
				computeFor(1);

				break;
				
			//
			// START OF QUANTIZATION TABLE MARKER AT 0xffdb
			//
			case DQT: 		
				
				// ack received with update with memory pointer
				if (read_dqt_marker() != SUCCESS)
				{
					SpacePrint("----------------------------------------\n");
					SpacePrint("EXTRACTOR: IQTZ DQT DECODING ERROR no %d - skipping JPEG\n", inmsg.param0); //this->name(), inmsg.param0);
					initialize();
					end_of_jpeg = true;
				}
				break;
				
				
			//
			// START OF IMAGE && END OF IMAGE MARKERS
			//
			case SOI:

				if (GetVerbose())
                   SpacePrint("EXTR: START OF IMAGE marker detected - reading image\n");

				break;
				
			//
			// START OF DEFINE RESTORE INTERVAL TABLE MARKER AT 0xffdb
			//
			/*
			case DRI:
				last_returned = read_dri_marker (); // read info and skip
				if (last_returned != SUCCESS)
				{
					SpacePrint("----------------------------------------\n");
					SpacePrint("EXTRACTOR: JPEG DRI DECODING ERROR no %d - skipping JPEG\n", last_returned); //this->name(), last_returned);
					initialize();
					end_of_jpeg = true;
				}
				break;
			*/	
			//
			// END OF IMAGE MARKER - nothing to do
			//
			case MARKER_EOI: // END OF IMAGE MARKER
				break;
				
			//
			// MARKERS WE SKIP
			//
			case APP0:			case APP1:			case APP2:			case APP3:			case APP4:
			case APP5:			case APP6:			case APP7:			case APP8:			case APP9:
			case APP10:			case APP11:			case APP12:			case APP13:			case APP14:
			case APP15:			case COM:			case DRI:
				skip_marker ();				
				break;
				
			//
			// ALL OTHER MARKERS ARE UNSUPPORTED BY THIS APPLICATION
			//
			default: 
				SpacePrint("----------------------------------------\n");
				SpacePrint("EXTRACTOR: JPEG DECODING UNDEFINED MARKER %d - skipping JPEG\n", last_marker); //this->name(), last_marker);
				initialize();
				end_of_jpeg = true;

				break;
			} // switch case
		
		}// while end of jpeg
	
    } // while 1
	
	
	
}

//////////////////////////////////////////////////////////////////////////////
/// 
/// Transmission of New Image command to IQTZ - wait for acknowledgement before
/// continuing
///
//////////////////////////////////////////////////////////////////////////////
void EXTR::communicate_IJPEG(unsigned short command) {
	JMSG newimage_message;

	switch (command)
	{


	case SCMD_NEW_IMAGE:
		
		if (GetVerbose())
			SpacePrint("EXTR: Sending NEW_IMAGE command to IQTZ - waiting for ACK\n");

		newimage_message.command = SCMD_NEW_IMAGE;
		newimage_message.param1 = m_currentJPEGImageNo;		// current image number
		newimage_message.param0 = m_infoImageHeaderAdd;	// current image info header

		computeFor(1);

		// transmitting request for decompressing new jpeg image - 
		// paramete is address of jpeg iomage to decompress
		ModuleWrite(IQTZ_ID, SPACE_BLOCKING, &newimage_message);
		
		computeFor(1);

		// waiting for acknowledgement with a blocking read
		// parameter is address of decompressed-YUV data
		ModuleRead(IQTZ_ID, SPACE_BLOCKING, &newimage_message);

		computeFor(1);

		if (newimage_message.command == SCMD_NEW_IMAGE_ACK) // collect address of bitmap
		{
			m_decompressedImageAddr[m_currentJPEGImageNo-1] = newimage_message.param0;
            //m_decompressedImageAddr[m_currentJPEGImageNo-1][VALID] = FALSE; // image data not valid while decompressing
		}

		if (GetVerbose())
			SpacePrint("EXTR: Return of ACK from IQTZ imageno:%d; address:%d\n", newimage_message.param0, m_decompressedImageAddr[m_currentJPEGImageNo-1]);

		computeFor(1);
		break;	


	case SCMD_END_IMAGE:
		
		if (GetVerbose())
			SpacePrint("EXTR: END_IMAGE command to send to IQTZ\n");


		newimage_message.command = SCMD_END_IMAGE;
		newimage_message.param1 = m_currentJPEGImageNo;
	
		computeFor(1);

		// transmitting end of request decompressing jpeg image - 
		// this tells the IJPEG module it will receive no more data
		
		ModuleWrite(HUFF_ID, SPACE_BLOCKING, &newimage_message);

		computeFor(1);

		// waiting for acknowledgement with a blocking read
		// parameter is address of decompressed-YUV data
		
		ModuleRead(IQTZ_ID, SPACE_BLOCKING, &newimage_message);
		
		computeFor(1);
		
		if (GetVerbose())
			SpacePrint("EXTR: Return of ACK for END_IMAGE command sent\n");

		break;	
	}
}


void EXTR::skip_bytes(UINT16 bytes) {
	m_inputAdd+=bytes;
	m_bufferValid = false;
}

inline UINT8 EXTR::read_8_bits() {
	UINT8 inputValue;

	UINT8 alignment = (m_inputAdd & 0x3);
	if(alignment == 0 || !m_bufferValid) { // Address is a multiple of 4: need to read a new word from memory
		DeviceRead(JPEGRAM_ID, m_inputAdd & 0xFFFFFFFC, &m_inputBuffer[0], 4);
		m_bufferValid = true;
	}
	inputValue = m_inputBuffer[alignment];
	
	m_inputAdd+=1;
	
	computeFor(1);

	return inputValue;
}

inline UINT16 EXTR::read_16_bits () {
	UINT16 value = (read_8_bits() << 8) | read_8_bits();
	computeFor(1);
	return (value);
}


//
//  EXTR::decode_interleaved_scan
//////////////////////////////////////////////////////////////////////////////
/// 
/// This is the loop that extracts the jpeg data
/// 
///
/// @return =>  UINT16  : SUCCESS (always)
///
/// @note   =>  
//////////////////////////////////////////////////////////////////////////////
UINT16 EXTR::decode_interleaved_scan() {
	UINT16 i, j, horizontal_mcus, vertical_mcus;
	space_uint2 k, number_of_image_components_in_frame;
	space_uint4 l, m, horizontal_sampling_factor, vertical_sampling_factor;

	JMSG msg;
	
	// use these shortcuts	
	horizontal_mcus = m_jpeg_decoder_structure.horizontal_mcus;
	vertical_mcus = m_jpeg_decoder_structure.vertical_mcus;
	number_of_image_components_in_frame = m_jpeg_decoder_structure.number_of_image_components_in_frame;

	//
	// The jpeg image is stored in minimal component units or mcu. Each unit represents
	// a luminance block (Y) or a chrominance block (C). Since this decoder only supports
	// files that are encoded in the YUV 4:2:0 format, MCUs are stored in groups of 6. 
	// The four first MCUs are luminance component. The two lasts are chrominance Cb and Cr
	// that are colour information for the 4 previous luminance blocks. 
	// This can be seen as follow:
	//
	//		 _____________
	//		|      |      |
	//		|  Y1  |  Y2  |
	//		|   ___|___   |  
	//		|  |   |   |  |
    //      |--|Cb | Cr|--|
    //      |  |___|___|  |
    //      |      |      |
	//		|  Y3  |  Y4  |
	//		|______|______|  
    //


	// we scan all the vertical & horizontal mcus
	for (i=1; i<=vertical_mcus; i++)
	//for (i=1; i<=vertical_mcus/2; i++)
	{
		for (j=1; j<=horizontal_mcus;j++)
		//for (j=1; j<=horizontal_mcus/4;j++)
		{		
			// every mcu is composed of 3 components Y, Cb and Cr
			for (k=0; k<number_of_image_components_in_frame; k ++)
			{
				vertical_sampling_factor = m_jpeg_decoder_structure.vertical_sampling_factor [k];
				horizontal_sampling_factor = m_jpeg_decoder_structure.horizontal_sampling_factor [k];

				computeFor(1);

				for (l=0; l<vertical_sampling_factor; l++)
				{
					for (m=0; m<horizontal_sampling_factor; m++)
					{
						
						// triggers a notification to huffman thread to start Huffman decoding
						m_currentHuffmanComponent = k;

						// send information to HUFF module
						msg.command = SCMD_HUFFMAN_MCU;
						msg.param0 = m_currentHuffmanComponent;
						msg.param1 = m_inputAdd;
						
						computeFor(1);

						// send over request to huffman module
						ModuleWrite(HUFF_ID, SPACE_BLOCKING, &msg);

						computeFor(1);

						ModuleRead(HUFF_ID, SPACE_BLOCKING, &msg);
						
						if (GetVerbose())
						{
							static int roulette = 1;
							switch (roulette)
							{
							case 1: //[
							case 5:
								SpacePrint("|\b");
								break;
							case 2:
							case 7:
								SpacePrint("/\b");
								break;
							case 3:
							case 6:
								SpacePrint("-\b");
								break;
							case 4:
							case 8:
								SpacePrint("\\\b");
								break;
							}

							if (++roulette > 8) roulette = 1;

						}

						if (GetVerbose())
							SpacePrint("EXTR: decode_interleave_scan returned i:%d j:%d k:%d l:%d m:%d\n", i, j, k, l, m);


						if (msg.command != SCMD_ACK)
						{
							SpacePrint("EXTRACTOR ERROR : decode_interleaved has received a FAIL %d from HUFFMAN module\n", msg.param0); //this->name(), msg.param0);
							sc_stop();
						}
						else

						{
							m_inputAdd = msg.param0;
							m_bufferValid = false;
						}

					}
				}
			}
		}
	}
		
	// (.)(.)
	//   <
	//  ____
	// \/  \/
	// when we get here, it's the end of the image!
	
	if (GetVerbose())
		SpacePrint("\n");

 	return SUCCESS;
}

void EXTR::initialize() {
	m_jpeg_decoder_structure = ZERO_JPEG_DECODER_STRUCTURE;
}

void EXTR::launch_face_detection(void) {
	if (GetVerbose())
		SpacePrint("EXTR: Launching Face Detection\n");
	
	if (++m_currentBitmapImage > MAX_NB_IMAGE)
		m_currentBitmapImage = 1;
	
	JMSG messageToFace;
	messageToFace.command = SCMD_BEGIN_FACE_DETECTION;
	messageToFace.param0 = m_decompressedImageAddr[m_currentBitmapImage-1];
	messageToFace.param1 = m_currentJPEGImageNo;
	computeFor(1);

	ModuleWrite(FACEDETECT_ID, SPACE_BLOCKING, &messageToFace);

	if (GetVerbose())
		SpacePrint("EXTR: Returned from launch face detection for image #%d\n", m_currentBitmapImage);
}


////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
////////////JPEG SPECIFIC CODE HERE BELOW //////////////////////
////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
UINT8 EXTR::read_next_marker() {
	UINT8 tmp;

	tmp = read_8_bits();

	if (tmp != 0xff)
		return ERROR_INVALID_FILL_BYTE;

	do {
		tmp = read_8_bits();
		computeFor(1);
	} while (tmp == 0xff);

	return tmp; //SUCCESS;
}

UINT8 EXTR::read_soi_marker () {
	if (read_16_bits () != (0xFF00 | SOI))
		return ERROR_SOI_NOT_DETECTED;
	return SUCCESS;
}


//
//  
//////////////////////////////////////////////////////////////////////////////
/// 
/// 
/// Read Start of Frame Marker
///
/// @return =>  UINT16 EXTR::read_sof_marker  : 
///
/// @note   =>  
//////////////////////////////////////////////////////////////////////////////
UINT8 EXTR::read_sof_marker() {
	UINT8 tmp, sof;
	UINT16 frame_header_length;
	UINT8  sample_precision;
	UINT16 number_of_lines, number_of_samples_per_line;
	space_uint2 i, number_of_image_components_in_frame;
	space_uint4 horizontal_sampling_factor, vertical_sampling_factor;
	//UINT16 quantization_table_destination_selector;
	UINT16 horizontal_mcus, vertical_mcus;

	// start_of_image must be detected only once
	if (m_jpeg_decoder_structure.sof_detected == 1)
		return ERROR_SOF_ALREADY_DETECTED;
	m_jpeg_decoder_structure.sof_detected = 1;
	sof = m_jpeg_decoder_structure.sof;
	computeFor(1);

	// 
	frame_header_length = read_16_bits ();

	// sample precision must be baseline = 8
	sample_precision = read_8_bits ();
	if (sample_precision != 8)
	{
		if ((sof == SOF0) || (sample_precision != 12))
			return ERROR_INVALID_SAMPLE_PRECISION;

		return ERROR_TWELVE_BIT_SAMPLE_PRECISION_NOT_SUPPORTED;
	}

	// read # of lines in jpeg
	m_jpeg_decoder_structure.number_of_lines = number_of_lines = 
		read_16_bits ();
	if (number_of_lines == 0)
		return ERROR_ZERO_NUMBER_OF_LINES_NOT_SUPPORTED;
	if (number_of_lines > MAXIMUM_NUMBER_OF_LINES)
		return ERROR_LARGE_NUMBER_OF_LINES_NOT_SUPPORTED;

	// read # of sample per lines in jpeg
	m_jpeg_decoder_structure.number_of_samples_per_line =
		number_of_samples_per_line = read_16_bits ();
	if (number_of_samples_per_line == 0)
		return ERROR_ZERO_SAMPLES_PER_LINE;
	if (number_of_samples_per_line > MAXIMUM_NUMBER_OF_SAMPLES_PER_LINE)
		return ERROR_LARGE_NUMBER_OF_SAMPLES_PER_LINE_NOT_SUPPORTED;
	
	// read # components per frame
	m_jpeg_decoder_structure.number_of_image_components_in_frame =
		number_of_image_components_in_frame = read_8_bits ();
	//if (number_of_image_components_in_frame != 1 &&	number_of_image_components_in_frame != 3)
	if (number_of_image_components_in_frame != 3) // luc: support only 4:2:0
		return ERROR_INVALID_NUMBER_OF_IMAGE_COMPONENTS_IN_FRAME;
	if (frame_header_length != (number_of_image_components_in_frame * 3 + 8))
		return ERROR_INVALID_FRAME_HEADER_LENGTH;

	computeFor(5);

	// for each component in frame
	for (i=0; i<number_of_image_components_in_frame; i++)
	{
		m_jpeg_decoder_structure.component_identifier [i] = read_8_bits ();

		tmp = read_8_bits ();

		// read horizontal & vertical sampling factor
		m_jpeg_decoder_structure.horizontal_sampling_factor [i] = 
			horizontal_sampling_factor = tmp >> 4;
		if (horizontal_sampling_factor == 0 || horizontal_sampling_factor > 4)
			return ERROR_INVALID_HORIZONTAL_SAMPLING_FACTOR;
		m_jpeg_decoder_structure.vertical_sampling_factor [i] = 
			vertical_sampling_factor = tmp & 0xf;
		if (vertical_sampling_factor == 0 || vertical_sampling_factor > 4)
			return ERROR_INVALID_VERTICAL_SAMPLING_FACTOR;

		// read quantization table selector
		//quantization_table_destination_selector = read_8_bits ();
		if (read_8_bits () > 3)
			return ERROR_INVALID_QUANTIZATION_TABLE_DESTINATION_SELECTOR;
			
		computeFor(3);
	}

	// compute the number of effective MCU H + V
	horizontal_mcus = (number_of_samples_per_line + 7) >> 3;
	vertical_mcus = (number_of_lines + 7) >> 3;

	computeFor(3);

	//
	// Fill structure for 4:2:0 image format. Others are rejected
	//
	if ((m_jpeg_decoder_structure . horizontal_sampling_factor [1] == 1) &&
		(m_jpeg_decoder_structure . vertical_sampling_factor [1] == 1) &&
		(m_jpeg_decoder_structure . horizontal_sampling_factor [2] == 1) &&
		(m_jpeg_decoder_structure . vertical_sampling_factor [2] == 1))
	{
		computeFor(1);
		
		if (m_jpeg_decoder_structure . horizontal_sampling_factor [0] == 2)
		{
			horizontal_mcus =	(number_of_samples_per_line + 15) >> 4;

			computeFor(1);

			if (m_jpeg_decoder_structure . vertical_sampling_factor [0] == 2)
			{
				// 4:2:0 format
				m_jpeg_decoder_structure.mcu_size = 384;

				vertical_mcus = (number_of_lines + 15) >> 4;
				
				computeFor(3);
			}
			else
				return ERROR_UNSUPPORTED_IMAGE_FORMAT; 
		}
		else
			return ERROR_UNSUPPORTED_IMAGE_FORMAT; 
	}
	else
		return ERROR_UNSUPPORTED_IMAGE_FORMAT; 
	

	m_jpeg_decoder_structure.horizontal_mcus = horizontal_mcus;
	m_jpeg_decoder_structure.vertical_mcus =	vertical_mcus;

	computeFor(1);

	return SUCCESS;
}


//
//  EXTR::read_dqt_marker
//////////////////////////////////////////////////////////////////////////////
/// 
/// Read Quantization Table Marker
/// 
///
/// @return =>  UINT16  : 
///
/// @note   =>  
//////////////////////////////////////////////////////////////////////////////
UINT8 EXTR::read_dqt_marker() {
	space_uint7 i;
	space_uint8 tmp;
	space_uint9 quantization_table_definition_length;
	space_uint4 quantization_table_element_precision;
	space_uint4 quantization_table_destination_identifier;

	quantization_table_definition_length = read_16_bits	() - 2;

	computeFor(1);

	#define JPEG_QUANTTBLSIZE	64
	while (quantization_table_definition_length >= 1 + JPEG_QUANTTBLSIZE)
	{
		tmp = read_8_bits ();

		quantization_table_element_precision = tmp >> 4;

		if (quantization_table_element_precision == 1)
			return ERROR_SIXTEEN_BIT_QUANTIZATION_TABLE_ELEMENT_PRECISION_NOT_SUPPORTED;

		if (quantization_table_element_precision > 1)
			return ERROR_INVALID_QUANTIZATION_TABLE_ELEMENT_PRECISION;

		quantization_table_destination_identifier = tmp & 0xf;

		if (quantization_table_destination_identifier > 1)
			return ERROR_INVALID_QUANTIZATION_TABLE_DESTINATION_IDENTIFIER;

		computeFor(3);

		for (i=0; i<JPEG_QUANTTBLSIZE/4; i++)
		{
			unsigned long ulQuantTblRegister = (read_8_bits () << 24) | (read_8_bits () << 16) | (read_8_bits () << 8) | (read_8_bits () );
			SPACE_REGISTER_RANGE(0,31);
			RegisterWrite(REGISTERFILE1_ID, quantization_table_destination_identifier*(JPEG_QUANTTBLSIZE/4) + i, &ulQuantTblRegister);
				
			computeFor(1);
		}

		quantization_table_definition_length -= 1 + JPEG_QUANTTBLSIZE;

		computeFor(1);
	}

	if (quantization_table_definition_length != 0)
		return ERROR_INVALID_QUANTIZATION_TABLE_DEFINITION_LENGTH;

	return SUCCESS;
}

UINT8 EXTR::skip_marker() {
	UINT16 bytes = read_16_bits() - 2;

	skip_bytes(bytes);

	computeFor(1);

	return SUCCESS;
}

/* (c) Space Codesign Systems Inc. 2005-2011 */
