
// ***********************************************************************
//
// Filename: 		ApplicationDefinitions.h
//
// Author: 			SpaceStudio generation engine
//
// Creation date: 	Sat Mar 12 14:10:15 EST 2016
//
// Warning: 		This file will not be overwritten by the SpaceStudio 
//					generation engine.
//
// Note:			This file is automatically added in a new project or in an 
//					existing project only if this file is not already present.
//					Otherwise, this file remains untouched by the SpaceStudio 
//					generation engine.  So this is a good place to declare 
//					variables that are specific to your application and that 
//					you need to share between your configurations.
//
// ***********************************************************************

#ifndef APPLICATION_DEFINITIONS_H
#define APPLICATION_DEFINITIONS_H

//Add your definitions here


#define IMG_WIDTH 1920
#define IMG_HEIGHT 1080
#define IMG_SIZE (IMG_WIDTH * IMG_HEIGHT)



#if defined(SIMTEK) && defined(SPACE_SIMULATION_MONITORING)
	#define waitIfNotMonitoring(i) (void)0
	#define SpacePrintIfNotMonitoring(fmt, ...) (void)0
#else
	#define waitIfNotMonitoring(i) wait(i)
	#define SpacePrintIfNotMonitoring(fmt, ...) SpacePrint(fmt, ##__VA_ARGS__ )
#endif

#define MyPrint(fmt, ...) do { SpacePrintIfNotMonitoring(fmt, ##__VA_ARGS__); waitIfNotMonitoring(1); } while(0)


/**********************/
/****** Commands ******/
/**********************/

typedef unsigned long cmd_type_t;
typedef unsigned long cmd_param_t;

typedef struct {
	cmd_type_t  command_type;
	cmd_param_t param0;
	cmd_param_t param1;
	cmd_param_t param2;
} message_t;

// Ex: const cmd_type_t NOM_DU_TYPE_DE_COMMANDE = <un entier>

const cmd_type_t MSG_RGB_TO_BW = 54;
const cmd_type_t MSG_SOBEL = 45;
const cmd_type_t MSG_BMP_WRITE = 454;



#endif //APPLICATION_DEFINITIONS_H
