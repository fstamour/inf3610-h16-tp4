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
#include "FACEDETECT.h"
#include "SpaceDisplay.h"
#include "ApplicationDefinitions.h"
#include "PlatformDefinitions.h"
 
FACEDETECT::FACEDETECT(sc_core::sc_module_name name, double period, sc_core::sc_time_unit unit, unsigned char id, unsigned char priority, bool verbose)
:SpaceBaseModule(name, period, unit, id, priority, verbose) {
	SC_THREAD(thread);
}


/*-----------------------------------------------------------------------------
* Process de FACEDETECT :
* le module FACEDETECT attend l'arrivée d'un message (ModuleRead()) pour savoir s'il y a 
* une image à traiter. Si oui, on va lire la zone RAM où se trouve l'image (BITMAP RAM)
* (mem_read()). Puis on va traiter l'image de façon à trouver les yeux dans l'image
*
* On commence par eliminer les couleurs ne representant pas un visage (de la peau)
* puis on ajoute un median filter sur cette image.
* 
* Une fois l'image traitee, on envoie un message au module EYEDETECT pour qu'il
* commence la detection des yeux a partir de l'image filtree et stockee dans la
* zone RAM (FACE RAM).
*----------------------------------------------------------------------------*/
void FACEDETECT::thread() {
	JMSG messageFromExtractor;
	space_uint22 working_input_address_info;	//start address to read the information field of BITMAP_RAM
	space_uint22 working_input_address_data;	//start address to read the data field of BITMAP_RAM
	unsigned long dataFace;
	SPACE_ALIGNED unsigned char resultFace[4];		//data read from the BITMAP_RAM and inserted into median_filter_int_2D[][]
	space_uint4 index_address;
	
	width = 0;				//largeur de l'image en pixels 
	height = 0;				//hauteur de l'image en pixels 
	lineNumber = 0;
	
	working_OUTPUT_address = FACE_START_ADDRESS;
	reading_address_face = 0;
	address_stored = false;

	image_numbers = 0;					//nombre d'images traitées
	image_currently_analyzed = 0;		//numéro de l'image en cours de traitement
	
	index_address = 0;	

	while(true)
    {
		//--------------------------------------------------------------------------------------
		//----------------------------- récupération du message ------------------------------//
		//-------------------------------------------------------------------------------------
		if (GetVerbose())
            SpacePrint("FACEDETECT: unit ready - waiting message from EXTRACTOR\n");
		
		// on attend un message pour traiter une image
		ModuleRead(EXTR_ID, SPACE_BLOCKING, &messageFromExtractor);

		if (GetVerbose())
            SpacePrint("FACEDETECT: Starting face Detection\n");

		// start address of 
		working_input_address_info = messageFromExtractor.param0; 
		
		// start address of image data
		working_input_address_data = working_input_address_info + INFO_IMAGE_HEADER; 

		// on recupere le numéro du fichier image
		image_currently_analyzed = messageFromExtractor.param1;

		//on stocke les images dans la FACE RAM
		working_OUTPUT_address = FACE_START_ADDRESS;

		//start addersses are stored to be retrievbed by console; if required
		face_image_start_addresses_array[index_address] = working_OUTPUT_address;
        index_address++;

		if(index_address > MAX_NB_IMAGE)
			index_address = 0;
		
		//--------------------------------------------------------------------------------------
		//------------------------- lecture de la mémoire -------------------------------------//
		//-------------------------------------------------------------------------------------	
		
		//recuperation de la largeur de l'image
		DeviceRead(BITMAPRAM_ID, working_input_address_info, &width);

		working_input_address_info += 4;

		//stockage de la largeur de l'image dans le champ INFO de la FACE RAM
		DeviceWrite(SHAREDRAM_ID, working_OUTPUT_address, &width);

		working_OUTPUT_address += 4;

		//recuperation de la hauteur de l'image 	
		DeviceRead(BITMAPRAM_ID, working_input_address_info, &height);

		//stockage de la hauteur de l'image dans le champ INFO de la FACE RAM
		DeviceWrite(SHAREDRAM_ID, working_OUTPUT_address, &height);

		//position memory pointer to beginning of image data
        working_OUTPUT_address += 0x1C;

		for(unsigned int i=0; i<width; i++)
		{
			for(unsigned int j=0; j<height; j++)
			{
					DeviceRead(BITMAPRAM_ID, working_input_address_data, &dataFace);
							
					working_input_address_data += 4;	
					
					//on filtre les couleurs qui ne sont pas de la peau
					int alignment = working_OUTPUT_address % 4;
					resultFace[alignment] = colorAnalysis(dataFace);	
						
					if(alignment == 0) {
						DeviceWrite(SHAREDRAM_ID, working_OUTPUT_address, &resultFace[0], 4);
					}

					working_OUTPUT_address += 1;
			}	
		}

		computeFor(1);
   }	
}
						
/*-----------------------------------------------------------------------------
* Méthode pour eliminer les couleurs ne representant pas de la peau (visage)
* dans laquelle les couleurs eliminees deviennent blanches et les autres sont
* gardees en tons de gris.
*----------------------------------------------------------------------------*/
unsigned char FACEDETECT::colorAnalysis (unsigned long dataFace) {
		//recuperation des valeurs de R, G et B
		unsigned char B = (dataFace & 0x00FF0000) >> 16;
		unsigned char G = (dataFace & 0x0000FF00) >> 8;
		unsigned char R = (dataFace & 0x000000FF) ;

		unsigned int y;

		unsigned char max;
		unsigned char min;
		unsigned char diff;

		//calcul de la valeur max de R, G et B
		if(R > G && R > B)
			max = R;
		else if(G > R && G > B)
			max = G;
		else
			max = B;

		//calcul de la valeur min de R, G et B
		if(R < G && R < B)
			min = R;
		else if(G < R && G < B)
			min = G;
		else
			min = B;
		
		if(R>G)
			diff = R-G;
		else
			diff = G-R;
		
		//conditions pour avoir une couleur representant de la peau
		if(R>95 && G>40 && B>20 && (max - min) > 15 && diff >15 && R>G && R>B)
			//y = 0.2989*R + 0.587*G + 0.114*B); //grayscale de la couleur
			y = (153*R + 301*G + 58*B) >> 9;
		else if(R>220 && G >210 && B>170 && diff <= 15 && R>B && G>B)
			y = (153*R + 301*G + 58*B) >> 9;
		else //sinon on elimine la couleur
			y = 255;

		return y;
}

/*-----------------------------------------------------------------------------
 * Methode qui applique un filtre median sur l'image apres que le skin filter
 * y ait ete applique
 *---------------------------------------------------------------------------*/
void FACEDETECT::median_filter() {
	unsigned int median;
	unsigned int i,j,f;

	if(lineNumber == 1)
	{
		//copy 1st line of image
		for (f = 1; f < width+1; f++)
		{
			median_filter_int_2D[f][0] = median_filter_int_2D[f][1];
		}

		//copy 1st column of image
		for (f = 0; f < MAX_HEIGHT; f++)
		{
			median_filter_int_2D[0][f] = median_filter_int_2D[1][f];
		}

		//copy last column of image
		for (f = 0; f < MAX_HEIGHT; f++)
		{
			median_filter_int_2D[width +1][f] = median_filter_int_2D[width][f];
		}

		for (i = 1, j = 1; i < width+1; i++)  
		{
			//on cherche la valeur median dans le masque 3x3
			median = find_median_value(i,j);

			DeviceWrite(SHAREDRAM_ID, working_OUTPUT_address, &median);

			working_OUTPUT_address += 4;
		}

		//copy line 3 (index 2) of the array to the line 2 (index 1)
		for (f = 1; f < width+1; f++)
		{
			median_filter_int_2D[f][1] = median_filter_int_2D[f][2];
		}
	}

	if((lineNumber>1) && (lineNumber<height))	
	{
		//copy 1rst column of image
		for (f = 0; f < MAX_HEIGHT; f++)
		{
			median_filter_int_2D[0][f] = median_filter_int_2D[1][f];
		}

		//copy last column of image
		for (f = 0; f < MAX_HEIGHT; f++)
		{
			median_filter_int_2D[width +1][f] = median_filter_int_2D[width][f];
		}

		for (i = 1, j = 1; i < width+1; i++)  
		{
			//on cherche la valeur median dans le masque 3x3
			median = find_median_value(i,j);

			DeviceWrite(SHAREDRAM_ID, working_OUTPUT_address, &median);

			working_OUTPUT_address += 4;
		}

		//copy line 2 (index 1) of the array to the line 1 (index 0)
		for (f = 1; f < width+1; f++)
		{
			median_filter_int_2D[f][0] = median_filter_int_2D[f][1];
		}

		//copy line 3 (index 2) of the array to the line 2 (index 1)
		for (f = 1; f < width+1; f++)
		{
			median_filter_int_2D[f][1] = median_filter_int_2D[f][2];
		}
	}

	if(lineNumber == height)	
	{
		//copy line 2 (index 1) of the array to the line 3 (index 2)
		for (f = 1; f < width+1; f++)
		{
			median_filter_int_2D[f][2] = median_filter_int_2D[f][1];
		}

		//copy 1rst column of image
		for (f = 0; f < MAX_HEIGHT; f++)
		{
			median_filter_int_2D[0][f] = median_filter_int_2D[1][f];
		}

		//copy last column of image
		for (f = 0; f < MAX_HEIGHT; f++)
		{
			median_filter_int_2D[width +1][f] = median_filter_int_2D[width][f];
		}

		for (i = 1, j = 1; i < width+1; i++)  
		{
			//on cherche la valeur median dans le masque 3x3
			median = find_median_value(i,j);

			DeviceWrite(SHAREDRAM_ID, working_OUTPUT_address, &median);

			working_OUTPUT_address += 4;
		}
		//on ecrit l'image dans un fichier PGM une fois terminé
		//launch_eye_detection();
	}
}

/*-----------------------------------------------------------------------------
 * Methode servant a trouver la valeur median des pixels analyses
 *---------------------------------------------------------------------------*/
unsigned int FACEDETECT::find_median_value(int x, int y) {
	unsigned int values[9], temp;
	
	//allocation des valeurs necessaire au calcul de la median
	values[0] = median_filter_int_2D[x][y-1];
	values[1] = median_filter_int_2D[x][y+1];
	values[2] = median_filter_int_2D[x-1][y];
	values[3] = median_filter_int_2D[x+1][y];
	values[4] = median_filter_int_2D[x][y];
	values[5] = median_filter_int_2D[x+1][y-1];
	values[6] = median_filter_int_2D[x-1][y-1];
	values[7] = median_filter_int_2D[x+1][y+1];
	values[8] = median_filter_int_2D[x-1][y+1];

	//on "sort" les valeurs en ordre croissant pour trouver la mediane
	for(int i=0; i<9; i++)
	{
		sorted_values [i] = values[i];
		for(int j=0; j<i; j++)
		{
			if(sorted_values[i] < values[j])
			{
				temp = values[j];
				values[j] = sorted_values[i];
				sorted_values[i] = temp;
			}	
		}
	}

	//si on n'applique aucun filtre a l'image
	//return median_filter_int_2D[x][y];

	//si on veut ajouter un mean filter
	//unsigned int mean = ( values[0]+values[1]+values[2]+
	//						values[3]+values[4]+values[5]+
	//						values[6]+values[7]+values[8])/9 ;
	//
	//return mean;

	//la valeur median correspont a la (n+1)/2 eme valeur (ici, la 5eme)
	return sorted_values[4];
}

/* (c) Space Codesign Systems Inc. 2005-2011 */
