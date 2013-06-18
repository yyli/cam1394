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
 * \author Yiying Li and Dylan Davis
 */
#ifndef CAMERA_H
#define CAMERA_H

#include <dc1394/dc1394.h>

#ifndef NOOPENCV
#include <cv.h>
#include <highgui.h>
#endif

#include <vector>



//! Contains the camera class definition and other misc variables
namespace cam1394
{
	// External constants for conversion between ENUMs and Values
	extern const int STARTVIDEOMODE;
	extern const char *videoModeNames[];
	extern const int STARTFRAMERATE;
	extern const float videoFrameRates[];
	extern const int STARTBAYERMETHODS;
	extern const char *bayerMethods[];
	extern const int STARTCOLORFILTER;
	extern const char *bayerPatterns[];

	// Inline functions that return the Value based on the ENUM
	inline const char *videoModeString(dc1394video_mode_t m) {
		return videoModeNames[m - STARTVIDEOMODE];
	}
	inline float frameRateValue(dc1394framerate_t f) {
		return videoFrameRates[f - STARTFRAMERATE];
	}
	inline const char *bayerPatternString(dc1394color_filter_t c) {
		return bayerPatterns[c - STARTCOLORFILTER];
	}

	
	/*!\brief Structure for holding images grabbed from the camera
	 */
	struct cam1394Image {
		/*!\brief Unsigned char buffer containing the raw image */
		uchar *data;
		/*!\brief Width of the image in pixels */
		int width;
		/*!\brief Height of the image in pixels */
		int height;
		/*!\brief Size of the \link data \endlink buffer */
		int size;

		cam1394Image() : data(NULL) {}
		/*!\brief destroys cam1394Image
		 * \return 1 if success, < 0 failure
		 */
		int destroy() {
			if (data != NULL) {
				delete[] data;
				data = NULL;
			}

			return 0;
		}
	};

	/*!\brief Structure for defining a specific Video Mode
	 */
	struct video_mode {
		dc1394video_mode_t mode;

		/*!\brief Vector of possible framerates for the \link mode \endlink video mode */
		std::vector<dc1394framerate_t> framerates;

		/*!\brief Is the camera outputting raw images */
		bool raw;
		/*!\brief Is there control over whether the camera outputs raw images */
		bool raw_control;
		/*!\brief The bayer pattern that the camera thinks it is using */
		dc1394color_filter_t bayer_pattern;

		bool format7;
		dc1394format7mode_t format7_mode;
	};

	/*!\brief Structure containing information about a camera
	 */
	struct camera_info {
		uint64_t guid;
		int unit;

		uint32_t vendor_id;
		uint32_t model_id;
		/*!\brief Char array containing the name of the vendor */
		char vendor[257];
		/*!\brief Char array containing the model of the camera */
		char model[257];

		/*!\brief Is there control over whether the camera outputs raw images */
		bool raw_control;
	
		/*!\brief All the possible video modes for this camera */	
		std::vector<video_mode> modes;
	};

	/*! 
	 * \class camera
	 * \brief Reads and writes from a FireWire 1394 or Point Grey camera
	 */
	class camera
	{
	public:
		//! Makes a camera object without opening an interface
		camera();

		//! Destroys a camera object
		~camera();
				
		/*!\brief Opens an interface to the camera with the 
		 * largest resolution and the fastest possible frame rate 
		 * with no debayering
		 * \return 1 if success, <0 if failure
		 */
		int open();
		
		/*!\brief Opens an interface to the specific camera with the 
		 * largest resolution and the fastest possible frame rate 
		 * with no debayering
		 * \param cam_guid		NONE will pick the first available camera, 
		 * 						else use the GUID printed by #printGUID
		 * \return 1 if success, <0 if failure
		 */
		int open(const char *cam_guid);

		/*!\brief Opens an interface to the camera
		 * \param cam_guid		NONE will pick the first available camera, 
		 * 						else use the GUID printed by #printGUID
		 * \param video_mode	This is a string from one of the following 
		 * 						\link cam1394::videoModeNames \endlink
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

#ifndef NOOPENCV
		/*!\brief Reads an image from a camera
		 * \return cv::Mat with image if success, an empty cv::Mat if failure
		 */
		cv::Mat read();//cv::Mat&);
#endif

		/*!\brief Reads an image from a camera
		 * \return 1 if success, < 0 failure
		 */
		int read(cam1394Image* image);
		
		/*!\brief Sets the brightness of the camera
		 * \param brightness brightness value
		 * \return 0 if success, <0 if failure
		 */
		int setBrightness(unsigned int brightness);

		/*!\brief Sets the exposure of the camera
		 * \param exposure exposure value
		 * \return 0 if success, <0 if failure
		 */
		int setExposure(unsigned int exposure);

		/*!\brief Sets the shutter speed
		 * \param shutter shutter value in hz, if <0 then auto shutter
		 * \return 0 if success, <0 if failure
		 */
		int setShutter(int shutter);

		/*!\brief Sets the gain value of the camera
		 * \param gain gain value, if <0 then auto gain
		 * \return 0 if success, <0 if failure
		 */
		int setGain(int gain);

		/*!\brief Sets the trigger of the camera
		 * \param trigger_in 0 to disable trigger, 1 to enable trigger
		 * \return 0 if success, <0 if failure
		 */
		int setTrigger(int trigger_in);

		/*!\brief Sets the the white balence
		 * \param b_u blue value (0-255)
		 * \param r_v red value (0-255)
		 * \return 0 if success, <0 if failure
		 */
		int setWhiteBalance(unsigned int b_u, unsigned int r_v);

		/*!\brief Sets whether the camera outputs raw
		 * \param raw True is raw, False is grayscale
		 * \return 0 if success, <0 if failure
		 */
		int setRawOutput(bool raw);

		/*!\brief gets the timestamp of the last frame
		 * \return timestamp in milliseconds (ms)
		 */
		long getTimestamp();

		/*!\brief gets the number of dropped frames for the last frame
		 * \return number of dropped frames
		 */
		int getNumDroppedFrames();

		/*!\brief prints the GUID of attached camera
		 */
		void printGUID();

		/*!\brief prints the GUIDs of connected cameras
		 */
		void printConnectedCams();

		/*!\brief prints the GUID of attached camera
		 */
		long getGUID();
		
		/*!\brief sets the debayering method and pattern
		 * \param method the string name of the method from 
		 * \link cam1394::bayerMethods \endlink
		 * \param pattern the string name of the pattern from
		 * \link cam1394::bayerPatterns \endlink
		 * \return 0 if success, <0 if failure
		 */
		int setBayer(const char* method, const char* pattern);

		int setVideoMode(const char*);
		int setFrameRate(float fps);

		void printFrameRate();
		void printVideoMode();

		std::vector<camera_info> getConnectedCameras();

	private:
		uint64_t guid;
		int width;
		int height;
		
		dc1394_t *d;
		dc1394camera_t* cam;
		dc1394color_filter_t bayer_pat;
		dc1394bayer_method_t bayer_met;
		dc1394video_mode_t _video_mode;
		dc1394framerate_t _fps;
		
		long timestamp;
		int droppedframes;

		int initCam(const char* cam_guid);
		int initParam(const char* video_mode, float fps, const char* method, const char* pattern);

		int getBestVideoMode(dc1394video_mode_t*);
		int getBestFrameRate(dc1394framerate_t*, dc1394video_mode_t);

		int convertVideoMode(const char*, dc1394video_mode_t*);
		int checkValidVideoMode(dc1394video_mode_t*);
		void printSupportedVideoModes(dc1394camera_t*);

		int convertFrameRate(float, dc1394framerate_t*);
		int checkValidFrameRate(dc1394framerate_t* frame_rate);
		void printSupportedFrameRates(dc1394camera_t*, dc1394video_mode_t mode);

		int _setVideoMode(const char*);
		int _setFrameRate(float fps);

#ifndef NOOPENCV
		int getOpenCVbits(int, int); 
#endif

		void clean_up();
	};
};
#endif
