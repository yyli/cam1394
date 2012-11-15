//camera.h
//Copyright (C) <2011, 2012>  <Yiying Li>
//
//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program.  If not, see <http://www.gnu.org/licenses/>.
//

/*!
 * \file camera.h
 *
 * \author Yiying Li
 */
#ifndef CAMERA_H
#define CAMERA_H
#include <cv.h>
#include <highgui.h>
#include <dc1394/dc1394.h>

//! Contains the camera class definition and other misc variables
namespace cam1394
{
	/*! 
	 * \class camera
	 * \brief reads and writes from a FireWire 1394 or Point Grey camera
	 */
	class camera
	{
	public:
		//! Makes a camera object without opening an interface
		camera();

		//! Destroys a camera object
		~camera();
		
		/*!\brief Opens an interface to the camera
		 * \param cam_guid		NONE will pick the first available camera, 
		 * 						else use the GUID returned by #getGUID
		 * \param video_mode	This is a string from one of the following 
		 * 						\link camera.cpp::videoModeNames \endlink
		 * \param fps			FPS value, floored to closest possible FPS
		 * \param bayer 		CURRENTLY THIS DOES NOTHING
		 * \param method 		CURRENTLY THIS DOES NOTHING
		 * \return 1 if success, <0 if failure
		 *
		 * <b> Example </b>
		 * \code 
		 * if (open("NONE", "640x480_MONO8", 30, NULL, NULL) > 0) {
		 * 		// display error
		 * 		return -1;
		 * }
		 * \endcode
		 */
		int open(const char* cam_guid, const char* video_mode, float fps, const char* bayer, const char* method);

		/*!\brief closes the interface to the camera
		 * \return 1 if success, < 0 if failure
		 */
		int close();

		/*!\brief Reads an image from a camera
		 * \return cv::Mat with image if success, an empty cv::Mat if failure
		 */
		cv::Mat read();//cv::Mat&);
		
		/*!\brief Sets the brightness of the camera
		 * \param brightness brightness value
		 * \return 0 if success, <0 if failure
		 */
		int setBrightness(unsigned int brightness);
		int setExposure(unsigned int);
		int setShutter(int);
		int setGain(int);
		int setTrigger(int);
		int setWhiteBalance(unsigned int, unsigned int);
		long getTimestamp();
		int getNumDroppedFrames();
		long getGUID();

	private:
		long guid;
		int width;
		int height;
		dc1394camera_t* cam;
		dc1394color_filter_t bayer_pat;
		dc1394bayer_method_t bayer_met;
		
		long timestamp;
		int droppedframes;

		dc1394framerate_t convertFrameRate(float);
		int getVideoMode(const char*, dc1394video_mode_t*);
		int checkValidVideoMode(dc1394video_mode_t*);
		void printSupportedVideoModes();

		void convertBayer(const char*, const char*);
		void clean_up();
	};
};
#endif
