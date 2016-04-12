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
#ifndef EXTR_H
#define EXTR_H

#include "ApplicationDefinitions.h"
#include "SpaceDataTypes.h"
#include "SpaceBaseModule.h"

#include "systemc"

// JPEG DEFINITIONS
/////////////////////////////////////////////////////////
#define DECOMPRESSED_IMAGESIZE		64*1024 // maximal size for decompressed image


class EXTR: public SpaceBaseModule {
	public:

		SC_HAS_PROCESS(EXTR);

		EXTR(sc_core::sc_module_name name, double period, sc_core::sc_time_unit unit, unsigned char id, unsigned char priority, bool verbose);

		void thread_extract_jpeg();

	protected:
		//
		// communication members
		//
		void communicate_IJPEG(unsigned short command);
		void launch_face_detection(void);
	
	
	private:
		//
		// General and pointers
		//
		UINT16		decode_interleaved_scan (void);
		void		initialize();
	
		space_uint4		m_currentJPEGImageNo;
		space_uint4		m_currentBitmapImage;
		space_uint22	m_decompressedImageAddr[MAX_NB_IMAGE];   // Adresses of decompressed images
															// second field is for validity of address
		space_uint22	    m_infoImageHeaderAdd;	// latest image header jpeg data address,
	
		space_uint22	    m_inputAdd;				// latest jpeg data address, multiple of 4 and 0x20 afetr header
	
		space_uint22 m_jpegAddress ; // address of very first image in memory
		space_uint22 m_jpegDist;		// distance between 2 images
		
		//
		//  Huffman
		//
		space_uint2		m_currentHuffmanComponent;

		SPACE_ALIGNED UINT8 m_inputBuffer[4];
		bool m_bufferValid;
	
		//
		// JPEG related
		//
		JPEG_DECODER_STRUCTURE m_jpeg_decoder_structure;

		UINT8		read_next_marker();
		UINT8		read_soi_marker ();
		UINT8		read_sof_marker ();
		UINT8		read_dqt_marker ();
		UINT8		skip_marker ();
		UINT8		read_8_bits ();
		UINT16		read_16_bits ();
		void 		skip_bytes(UINT16 nbBytes);
};

#endif

/* (c) Space Codesign Systems Inc. 2005-2011 */
