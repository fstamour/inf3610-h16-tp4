///////////////////////////////////////////////////////////////////////////////
///\\file     definition_jpeg.h
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

#ifndef DEFINITION_JPEGFILE_H
#define DEFINITION_JPEGFILE_H

#include <SpaceDataTypes.h>

// data types
typedef signed char		INT8;
typedef unsigned char	UINT8;

typedef signed short	INT16;
typedef unsigned short	UINT16;

typedef signed int		INT32;
typedef unsigned int	UINT32;



#define MAXIMUM_NUMBER_OF_LINES					512
#define MAXIMUM_NUMBER_OF_SAMPLES_PER_LINE		512
#define NBITS									8
#define FOUR_TWO_ZERO							1

typedef struct JPEG_DECODER_STRUCTURE
{
	space_uint1 sof_detected;
	UINT8 sof;

	UINT32 number_of_lines;
	UINT32 number_of_samples_per_line;
	space_uint2 number_of_image_components_in_frame;
	space_uint3 component_identifier [3];
	space_uint4 horizontal_sampling_factor [3];
	space_uint4 vertical_sampling_factor [3];

	UINT16 horizontal_mcus;
	UINT16 vertical_mcus;
	space_uint10 mcu_size;
} JPEG_DECODER_STRUCTURE;

// ajout par Luc
#define MARKER_SOF 0x0
#define MARKER_DHT 0x1
#define MARKER_SOS 0x2
#define MARKER_DQT 0x3
#define MARKER_DRI 0x4
#define MARKER_SKIP 0x5
#define MARKER_SOI 0x6
#define MARKER_EOI 0x7

typedef enum
{
	SOF0  = 0xc0,
	SOF1  = 0xc1,
	SOF2  = 0xc2,
	SOF3  = 0xc3,

	SOF5  = 0xc5,
	SOF6  = 0xc6,
	SOF7  = 0xc7,

	JPG   = 0xc8,
	SOF9  = 0xc9,
	SOF10 = 0xca,
	SOF11 = 0xcb,

	SOF13 = 0xcd,
	SOF14 = 0xce,
	SOF15 = 0xcf,

	DHT   = 0xc4,

	DAC   = 0xcc,

	RST0  = 0xd0,
	RST1  = 0xd1,
	RST2  = 0xd2,
	RST3  = 0xd3,
	RST4  = 0xd4,
	RST5  = 0xd5,
	RST6  = 0xd6,
	RST7  = 0xd7,

	SOI   = 0xd8,
	EOI   = 0xd9,
	SOS   = 0xda,
	DQT   = 0xdb,
	DNL   = 0xdc,
	DRI   = 0xdd,
	DHP   = 0xde,
	EXP   = 0xdf,

	APP0  = 0xe0,
	APP1  = 0xe1,
	APP2  = 0xe2,
	APP3  = 0xe3,
	APP4  = 0xe4,
	APP5  = 0xe5,
	APP6  = 0xe6,
	APP7  = 0xe7,
	APP8  = 0xe8,
	APP9  = 0xe9,
	APP10 = 0xea,
	APP11 = 0xeb,
	APP12 = 0xec,
	APP13 = 0xed,
	APP14 = 0xee,
	APP15 = 0xef,

	JPG0  = 0xf0,
	JPG1  = 0xf1,
	JPG2  = 0xf2,
	JPG3  = 0xf3,
	JPG4  = 0xf4,
	JPG5  = 0xf5,
	JPG6  = 0xf6,
	JPG7  = 0xf7,
	JPG8  = 0xf8,
	JPG9  = 0xf9,
	JPG10  = 0xfa,
	JPG11  = 0xfb,
	JPG12  = 0xfc,
	JPG13  = 0xfd,

	COM   = 0xfe,

	TEM   = 0x01
} JPEG_MARKER;


#define SUCCESS 0
#define FAIL	1

#define ERROR_CANNOT_OPEN_FILE											0
#define ERROR_SOF_ALREADY_DETECTED										1
#define ERROR_INVALID_SAMPLE_PRECISION									2
#define ERROR_TWELVE_BIT_SAMPLE_PRECISION_NOT_SUPPORTED					3
#define ERROR_ZERO_NUMBER_OF_LINES_NOT_SUPPORTED						4
#define ERROR_LARGE_NUMBER_OF_LINES_NOT_SUPPORTED						5
#define ERROR_ZERO_SAMPLES_PER_LINE										6
#define ERROR_LARGE_NUMBER_OF_SAMPLES_PER_LINE_NOT_SUPPORTED			7


#define ERROR_SOI_ALREADY_DETECTED										8

#define ERROR_INVALID_NUMBER_OF_IMAGE_COMPONENTS_IN_FRAME				9
#define ERROR_INVALID_FRAME_HEADER_LENGTH								10
#define ERROR_INVALID_HORIZONTAL_SAMPLING_FACTOR						11
#define ERROR_INVALID_VERTICAL_SAMPLING_FACTOR							12
#define ERROR_INVALID_QUANTIZATION_TABLE_DESTINATION_SELECTOR			13
#define ERROR_UNSUPPORTED_IMAGE_FORMAT									14
#define ERROR_SOF_NOT_DETECTED											15
#define ERROR_INVALID_NUMBER_OF_COMPONENTS_IN_SCAN						16
#define ERROR_INVALID_SCAN_HEADER_LENGTH								17
#define ERROR_INVALID_START_OF_SPECTRAL_SELECTION						18
#define ERROR_INVALID_END_OF_SPECTRAL_SELECTION							19
#define ERROR_INVALID_SUCCESSIVE_APPROXIMATION_BIT_POSITION_HIGH		20
#define ERROR_INVALID_SUCCESSIVE_APPROXIMATION_BIT_POSITION_LOW			21
#define ERROR_INVALID_QUANTIZATION_TABLE_ELEMENT_PRECISION				22
#define ERROR_INVALID_QUANTIZATION_TABLE_DESTINATION_IDENTIFIER			23
#define ERROR_QUANTIZATION_TABLE_ELEMENT_ZERO							24
#define ERROR_INVALID_QUANTIZATION_TABLE_DEFINITION_LENGTH				25
#define ERROR_INVALID_DEFINE_RESTART_INTERVAL_SEGMENT_LENGTH			26
#define ERROR_SOF3_MARKER_NOT_SUPPORTED									27
#define ERROR_SOF5_MARKER_NOT_SUPPORTED									28
#define ERROR_SOF6_MARKER_NOT_SUPPORTED									29
#define ERROR_SOF7_MARKER_NOT_SUPPORTED									30
#define ERROR_JPG_MARKER_NOT_SUPPORTED									31
#define ERROR_SOF9_MARKER_NOT_SUPPORTED									32
#define ERROR_SOF10_MARKER_NOT_SUPPORTED								33
#define ERROR_SOF11_MARKER_NOT_SUPPORTED								34
#define ERROR_SOF13_MARKER_NOT_SUPPORTED								35
#define ERROR_SOF14_MARKER_NOT_SUPPORTED								36
#define ERROR_SOF15_MARKER_NOT_SUPPORTED								37
#define ERROR_DAC_MARKER_NOT_SUPPORTED									38
#define ERROR_DNL_MARKER_NOT_SUPPORTED									39
#define ERROR_DRI_MARKER_NOT_SUPPORTED									40
#define ERROR_DHP_MARKER_NOT_SUPPORTED									41
#define ERROR_EXP_MARKER_NOT_SUPPORTED									42
#define ERROR_JPG0_MARKER_NOT_SUPPORTED									43
#define ERROR_JPG1_MARKER_NOT_SUPPORTED									44
#define ERROR_JPG2_MARKER_NOT_SUPPORTED									45
#define ERROR_JPG3_MARKER_NOT_SUPPORTED									46
#define ERROR_JPG4_MARKER_NOT_SUPPORTED									47
#define ERROR_JPG5_MARKER_NOT_SUPPORTED									48
#define ERROR_JPG6_MARKER_NOT_SUPPORTED									49
#define ERROR_JPG7_MARKER_NOT_SUPPORTED									50
#define ERROR_JPG8_MARKER_NOT_SUPPORTED									51
#define ERROR_JPG9_MARKER_NOT_SUPPORTED									52
#define ERROR_JPG10_MARKER_NOT_SUPPORTED								53
#define ERROR_JPG11_MARKER_NOT_SUPPORTED								54
#define ERROR_JPG12_MARKER_NOT_SUPPORTED								55
#define ERROR_JPG13_MARKER_NOT_SUPPORTED								56
#define ERROR_TEM_MARKER_NOT_SUPPORTED									57
#define ERROR_RES_MARKER_NOT_SUPPORTED									58
#define ERROR_INVALID_SOS_MARKER										59
#define ERROR_INVALID_DC_ENTROPY_CODING_DESTINATION_SELECTOR			60
#define ERROR_INVALID_AC_ENTROPY_CODING_DESTINATION_SELECTOR			61
#define ERROR_INVALID_TABLE_CLASS										62
#define ERROR_INVALID_NUMBER_OF_HUFFMAN_CODES							63
#define ERROR_BAD_HUFFMAN_TABLE											64
#define ERROR_INVALHUFF_ID_TABLE_DESTINATION_IDENTIFIER				65
#define ERROR_INVALID_COMPONENT_IDENTIFIER								66
#define ERROR_INVALID_SCAN_COMPONENT_SELECTOR							67
#define ERROR_SOI_NOT_DETECTED											68

#define ERROR_INVALHUFF_ID_CODE										69

#define ERROR_NON_INTERLEAVED_DECODING_NOT_SUPPORTED					70
#define ERROR_INTERLEAVED_AC_SCAN										71
#define ERROR_ONE_SAMPLE_PER_LINE_NOT_SUPPORTED							72
#define ERROR_CODING													73
#define ERROR_RST0_MARKER_DETECTED_OUTSIDE_SCAN							74
#define ERROR_RST1_MARKER_DETECTED_OUTSIDE_SCAN							75
#define ERROR_RST2_MARKER_DETECTED_OUTSIDE_SCAN							76
#define ERROR_RST3_MARKER_DETECTED_OUTSIDE_SCAN							77
#define ERROR_RST4_MARKER_DETECTED_OUTSIDE_SCAN							78
#define ERROR_RST5_MARKER_DETECTED_OUTSIDE_SCAN							79
#define ERROR_RST6_MARKER_DETECTED_OUTSIDE_SCAN							80
#define ERROR_RST7_MARKER_DETECTED_OUTSIDE_SCAN							81
#define ERROR_INVALID_RESTART_MARKER									82

#define ERROR_INVALID_FILL_BYTE											83
#define ERROR_SIXTEEN_BIT_QUANTIZATION_TABLE_ELEMENT_PRECISION_NOT_SUPPORTED	84

#define ERROR_INSUFFICIENT_MEMORY										85
#define ERROR_QUANTIZATION_TABLE										86

#define ERROR_JPEG_FORMAT_NOT_SUPPORTED									0xff




#define		QUANTTABLE_L	0x00000000	// Luminance Quantization Table
#define		QUANTTABLE_C	0x00000001	// Chrominance Quantization Table

#define		HUFFTABLE_ACL	0x00000000	// Huffman Ac Luminance Component
#define		HUFFTABLE_ACC	0x00000001	// Huffman Ac Chrominance Component
#define		HUFFTABLE_DCL	0x00000002	// Huffman Dc Luminance Component
#define		HUFFTABLE_DCC	0x00000003	// Huffman Dc Chrominance Component
//#define		HUFFTABLE_X		0x00000004	// Done with Huffman table transfer


// define for version 1.1
 
#define HUFF_MAXCODE	0x00000000
#define HUFF_MINCODE	0x00000001
#define HUFF_HUFFSIZE	0x00000002
#define HUFF_HUFFVAL	0x00000003


// HUFFMAN

// for ACDC
#define DC 0 
#define AC 1

// for LUMICHRO
#define LUMINANCE 0
#define CHROMINANCE 1




#endif //DEFINITION_JPEGFILE_H

/* (c) Space Codesign Systems Inc. 2005-2011 */
