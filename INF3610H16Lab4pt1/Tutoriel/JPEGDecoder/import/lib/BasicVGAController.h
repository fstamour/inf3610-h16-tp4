///////////////////////////////////////////////////////////////////////////////
///
///         Space Codesign System Inc. - (http://www.spacecodesign.com)
///         (c) All Rights Reserved. 2014
///
///         No authorization to modify or use this file for
///         commercial purposes without prior written consentement
///         of its author(s)
///
///////////////////////////////////////////////////////////////////////////////
#ifndef BASICVGACONTROLLER_H_
#define BASICVGACONTROLLER_H_

#ifdef WIN32
	#include <windows.h>
#endif
#include <GL/freeglut.h>
#include "VGADefinitions.h"

class BasicVGAController {
	public:

		BasicVGAController(unsigned int width, unsigned int height, unsigned int refresh_rate);
		~BasicVGAController();

		void save_bitmap(rgba** frame, int index);
		void display_frame(rgba** frame);
		void synchronize();
		inline bool can_animate() { return m_can_animate; }
		inline bool can_continue() { return !m_exit_requested; }
		
		#ifdef WIN32
			static HANDLE hThreadVGAMain;
		#endif

	private:

		///
		/// Methods
		///
		static void VGADisplay();
		static void VGAReshape(int iWidht, int iHeight);
		static void VGATimer(int iValue);
		static void createGLUTMenus();
		static void processMenuEvents(int option);

		///
		/// Members
		///
		static bool m_exit_requested;
		static bool m_can_animate;

		#ifdef WIN32
			static DWORD WINAPI VGAMain(LPVOID _data);
			DWORD iVGAMainID;
		#endif
};

#endif /*BASICVGACONTROLLER_H_*/
