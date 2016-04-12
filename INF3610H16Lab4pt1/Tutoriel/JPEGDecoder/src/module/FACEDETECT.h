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
#ifndef FACEDETECT_H
#define FACEDETECT_H

#include "ApplicationDefinitions.h"
#include "SpaceDataTypes.h"
#include "SpaceBaseModule.h"

#include "systemc"

class FACEDETECT: public SpaceBaseModule {
	public:

		SC_HAS_PROCESS(FACEDETECT);

		FACEDETECT(sc_core::sc_module_name name, double period, sc_core::sc_time_unit unit, unsigned char id, unsigned char priority, bool verbose);

		void thread();

	private:

		void median_filter();
		unsigned int find_median_value(int x, int y);
		void print_filtered_image_to_file(char* filename, unsigned long data_begin);
		unsigned char colorAnalysis(unsigned long dataFace);

		unsigned int width;
		unsigned int height;
		unsigned int lineNumber;
		space_uint22 working_OUTPUT_address;

		//variable pour l'adresse de départ des différentes images en RAM EDGE
		space_uint22 reading_address_face;

		//pour permettre de stocker l'@ de départ
		bool address_stored;
	
		//variables membres pour le statut du système demandé par l'utilisateur
		space_uint4 image_numbers;					//nombre d'images traitées
		space_uint4 image_currently_analyzed;		//numéro de l'image en cours de traitement
	
		//stockage des adresses de départ des images
		space_uint22 face_image_start_addresses_array[10];	//10 images pour commencer
	
		unsigned int sorted_values[9]; //pour le median filter
	
		//static unsigned int median_filter_int_2D [MAX_WIDTH][MAX_HEIGHT];
		unsigned int median_filter_int_2D [MAX_WIDTH][MAX_HEIGHT];
};

#endif

/* (c) Space Codesign Systems Inc. 2005-2011 */
