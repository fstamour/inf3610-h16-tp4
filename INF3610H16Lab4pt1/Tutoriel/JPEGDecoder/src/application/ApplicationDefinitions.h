///////////////////////////////////////////////////////////////////////////////
///\\file     ApplicationDefinitions.h
///
///         Space Codesign Systems Inc. (http://www.spacecodesign.com)
///         (c) All Rights Reserved. 2011                                  
///                                                                       
///         This file contains proprietary, confidential information of Space Codesign 
///         Systems Inc. and may be used only pursuant to the terms of a valid license 
///         agreement with Space Codesign Systems Inc. For more information about licensing,
///         please contact your Space Codesign representative. 
/// 
///////////////////////////////////////////////////////////////////////////////

#ifndef APPLICATION_DEFINITIONS_H
#define APPLICATION_DEFINITIONS_H

#include "definition_jpeg.h"

#define FRAME_DONE	0xFFFFC

///
/// Facts on Images
///
/// Images are stored into memory at different locations. Before each raw image
/// data is inserted an application specific header of 20 bytes. These bytes contain
/// information about the associated image, like width and height, temp buffer address
/// and so on
///


/*
BITMAP HEADER
0	Temp Buffer Address start address
1   Temp buffer end address
2
3
4
5
6
7

  */

  /*
BITMAP TEMP BUFFER HEADER

  no header



  */

// this is for compilation of SW L2 and L4 since these are defined in MS-Win
// and not in microC
#ifndef TRUE
	#define FALSE 0
	#define TRUE !FALSE
#endif


#define BURST_SIZE_SHORT 2


/*************************************/
/* **** Devices Extra definitions **** */
/*************************************/
const unsigned long INFO_IMAGE_HEADER			= 0x00000020ul; 

const unsigned long JPEG_IMAGE_SPACER			= 0x00001000ul;

const unsigned long BITMAP_IMAGE_SPACER			= 0x00010800ul;
//const unsigned long BITMAP_TEMP1_ADDRESS		= 0x00033000ul;
//const unsigned long BITMAP_TEMP2_ADDRESS		= 0x0003B000ul;
//const unsigned long BITMAP_TEMP2_ADDRESS		= 0x0002E000ul;
const unsigned long BITMAP_TEMP_ADDRESS		= 0x00021000ul;
//const unsigned long BITMAP_TEMP2_ADDRESS		= 0x0002E000ul;

const unsigned long EDGE_START_ADDRESS			= 0x00200000ul;

const unsigned long FACE_START_ADDRESS	        = 0x00000000ul;
const unsigned long EYE_START_ADDRESS	        = 0x00100000ul;




/*************************************/
/* **** Commands				**** */
/*************************************/
const unsigned int SCMD_BEGIN_EDGE_DETECTION				= 0x00;

const unsigned int SCMD_HUFFMAN_DHT							= 0x60;
const unsigned int SCMD_HUFFMAN_MCU							= 0x61;

const unsigned int SCMD_START_IDCT							= 0x70;

const unsigned int SCMD_CONVERT_MCU							= 0x80;		
const unsigned int SCMD_CONVERT_MCU_END						= 0x81;		

const unsigned int SCMD_NEW_IMAGE							= 0x90;
const unsigned int SCMD_NEW_IMAGE_ACK						= 0x91;
const unsigned int SCMD_END_IMAGE							= 0x98;
const unsigned int SCMD_END_IMAGE_ACK						= 0x99;

const unsigned int SCMD_BEGIN_COMP_DATA_TRANSMISSION		= 0xA0;
const unsigned int SCMD_END_COMP_DATA_TRANSMISSION			= 0xA1;
const unsigned int SCMD_COMP_DATA_TRANSMISSION				= 0xA2;

const unsigned int SCMD_IQUANT_DQT							= 0xB0;

const unsigned int SCMD_BEGIN_FACE_DETECTION	            = 0xC1;
const unsigned int SCMD_BEGIN_EYE_DETECTION	                = 0xC2;


const unsigned int SCMD_ACK_ERROR							= 0xFE;
const unsigned int SCMD_ACK									= 0xFF;

//
// commands in support of GUI
//

const unsigned int GUI_CMD_GET_IMAGE							= 0x70;
const unsigned int GUI_CMD_GET_IMAGE_OK							= 0x701;
const unsigned int GUI_CMD_GET_IMAGE_FAILURE					= 0x702;
const unsigned int GUI_CMD_START_JPEG_COMPRESSION				= 0x71;
const unsigned int GUI_CMD_START_JPEG_COMPRESSION_NOT_STARTED	= 0x711;
const unsigned int GUI_CMD_START_JPEG_COMPRESSION_STARTED		= 0x712;
const unsigned int GUI_CMD_START_JPEG_COMPRESSION_ALREADY		= 0x713;
const unsigned int GUI_CMD_PAUSE_JPEG_COMPRESSION				= 0x72;
const unsigned int GUI_CMD_STOP_JPEG_COMPRESSION				= 0x73;
const unsigned int GUI_CMD_GET_STATUS							= 0x74;
const unsigned int GUI_CMD_STATUS_RETURNED						= 0x75;

const unsigned int GUI_CMD_START_EDGE_DETECTION					= 0x75;
const unsigned int GUI_CMD_PAUSE_EDGE_DETECTION					= 0x76;
const unsigned int GUI_CMD_STOP_EDGE_DETECTION					= 0x77;

const unsigned int GUI_CMD_GET_EDGE_IMAGE						= 0x78;
const unsigned int GUI_CMD_GET_DECOMP_IMAGE						= 0x79;
const unsigned int GUI_CMD_GET_COMPARE_RESULT					= 0x7A;

const unsigned int GUI_CMD_ACK									= 0x7EE;
const unsigned int GUI_CMD_ACK_ERROR							= 0x7FF;

const unsigned int GUI_CMD_START								= 0x120;
const unsigned int GUI_CMD_PAUSE								= 0x130;

//-- Reponses des modules a certaines commandes de la Console Windows
//#define	SRESP_CAM_STATUS		0x220
const unsigned int SRESP_EDGE_STATUS							= 0x200;
const unsigned int SRESP_EDGE_GET_IMAGE							= 0x210;
const unsigned int SRESP_EDGE_NO_IMAGE							= 0x230;


/*************************************/
/* **** Types					**** */
/*************************************/

const unsigned int JPRM_L_TYPE								= 0;
const unsigned int JPRM_C_TYPE								= 1;

const unsigned int ADDRESS									= 0;
const unsigned int VALID									= 1;


// structure for message content to be sent on the bus - these messages
// are uninterruptable
typedef struct _JMSG {
	unsigned long   command;
	unsigned long	param0;
	unsigned long	param1;
} JMSG;


/* **** Data types **** */

#define	MAX_NB_IMAGE	2


/* /////// STATES \\\\\\\ */

const unsigned long JPEG_STARTED							= 0x1ul;
const unsigned long JPEG_STOPPED							= 0x2ul;
const unsigned long JPEG_PAUSED								= 0x4ul;


//-- Edge
//etiquette pour specifier la taille du tableau statique
#define MAX_WIDTH	460				// colonnes  = limite de la largeur de l'image
#define MAX_HEIGHT	3				// lignes = hauteur de l'image pas limite


#endif

/* (c) Space Codesign Systems Inc. 2005-2011 */
