//camera.cpp
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

#include <stdio.h>
#include <cassert>
#include "highgui.h"

#include "camera.h"
#include "cameraconstants.h"

using namespace cam1394;

/* defualt constructor */
camera::camera() : guid(0), width(-1), height(-1), cam(NULL) {}

/* destructor */
camera::~camera()
{
	clean_up();
}

int camera::open() {
	dc1394video_mode_t mode = DC1394_VIDEO_MODE_MIN;
	dc1394framerate_t rate = DC1394_FRAMERATE_MIN;

	if (getBestVideoMode(&mode) < 0) {
		return -1;
	} else if (getBestFrameRate(&rate) < 0) {
		return -1;
	}

	return open("NONE", videoModeNames[mode - STARTVIDEOMODE], 
				videoFrameRates[rate - STARTFRAMERATE], 
				"NONE", "NONE");
}

void camera::printConnectedCams() {
	dc1394error_t err;
	dc1394camera_list_t *list;
	dc1394_t *d;
	
	d = dc1394_new();
	if (d == NULL) {
		fprintf(stderr, "Can't initialize dc1394_content\n");
		dc1394_camera_free_list(list);
		dc1394_free(d);
		return;
	}
	
	err = dc1394_camera_enumerate(d, &list);
	if (err != DC1394_SUCCESS) {
		fprintf(stderr, "Could not get camera list\n");
		dc1394_camera_free_list(list);
		dc1394_free(d);
		return;
	}
	
	if (list->num == 0) {
		printf("No Cameras Found\n");
		dc1394_camera_free_list(list);
		dc1394_free(d);
		return;
	}

	printf("Connected Cameras\n");	
	for (unsigned int i = 0; i < list->num; i++) {
		printf("Cam %d: %016lX\n", i, list->ids[i].guid);
	}
	
	dc1394_camera_free_list(list);
	dc1394_free(d);
}

int camera::open(const char* cam_guid, const char* video_mode, float fps, const char* bayer, const char* method)
{
	convertBayer(bayer, method);

	int err;
	dc1394_t *d;
	dc1394camera_list_t *list;
	d = dc1394_new();
	if (d == NULL)
	{
		fprintf(stderr, "Can't initialize dc1394_content\n");
		return -1;
	}


	err = dc1394_camera_enumerate(d, &list);
	if (err != DC1394_SUCCESS)
	{
		fprintf(stderr, "Could not get camera list\n");
		return -1;
	}

	if (list->num == 0)
	{
		fprintf(stderr, "No Cameras Found\n");
		return -1;
	}
	
	char* temp = (char*)malloc(1024*sizeof(char));
	uint32_t value[3];

	for(unsigned int i=0; i < list->num; i++)
	{
		cam = dc1394_camera_new(d, list->ids[i].guid);
		if (!cam) {
			fprintf(stderr, "failed to initliaze camera with GUID %lX\n", list->ids[i].guid);
			continue;
		}
		
		value[0] = cam->guid & 0xFFFFFFFF;
		value[1] = (cam->guid >> 32) & 0x000000FF;
		value[2] = (cam->guid >> 40) & 0xFFFFF;

		sprintf(temp,"%06X%02X%08X", value[2], value[1], value[0]);

		if (!strcasecmp(cam_guid, "NONE"))
		{
//#ifdef DEBUGCAMERA
			fprintf(stderr, "WARNING: No guid specified, using first camera. GUID: %s\n", temp);
//#endif
			guid = value[0] | ((uint64_t)value[1] << 32) | ((uint64_t)value[2] << 40);
			break;
		}

		if (!strcasecmp(temp, cam_guid))
		{
#ifdef DEBUGCAMERA
			fprintf(stderr, "Camera %s found\n", cam_guid);
#endif
			guid = value[0] | ((uint64_t)value[1] << 32) | ((uint64_t)value[2] << 40);
			break;
		}
		puts("cleaned_up");
		clean_up();
	}
	free(temp);
	dc1394_camera_free_list(list);
	
	if (!cam)
	{
		if (!strcasecmp(cam_guid, "NONE"))
		{
			fprintf(stderr, "Can't find camera\n");
		}
		else
		{
			fprintf(stderr, "Can't find camera with guid %s\n", cam_guid);
		}
		return -1;
	}

	bool Set_Success =  true;
	if (setVideoMode(video_mode) < 0) {
		Set_Success = false;
	} else if (setFrameRate(fps) < 0) {
		Set_Success = false;
	}

	if (DC1394_SUCCESS != dc1394_capture_setup(cam, 40, DC1394_CAPTURE_FLAGS_DEFAULT))
	{
		Set_Success = false;	
	}

	if (!Set_Success)
	{
		clean_up();
		fprintf(stderr, "Failed to open camera\n");
		return -1;
	}

	if (DC1394_SUCCESS != dc1394_video_set_transmission(cam, DC1394_ON))
	{
		clean_up();
		fprintf(stderr, "Failed to start camera\n");
		return -1;	
	}

	return 0;
}

int camera::setFrameRate(float fps) {
	dc1394framerate_t fr; 
	if (convertFrameRate(fps, &fr) < 0) {
		printSupportedFrameRates(_video_mode);
		return -1;
	}

	if (DC1394_SUCCESS != dc1394_video_set_framerate(cam, fr))
	{
		fprintf(stderr, "Failed to set the framerate\n");
		return -1;
	}

	return 0;
}

int camera::getBestVideoMode(dc1394video_mode_t *mode) {
	return 0;
}

int camera::getBestFrameRate(dc1394framerate_t *rate) {
	return 0;
}

/* Sets video_mode based on string input */
int camera::convertVideoMode(const char* mode, dc1394video_mode_t *video_mode)
{
	for (size_t i = 0; i < 32; i++) {
		if (!strcasecmp(mode, videoModeNames[i])) {
			*video_mode = (dc1394video_mode_t)(i + STARTVIDEOMODE);

			/* if a non-Format 7 mode set width and height */
			if (i < 23) {
				width = videoWidths[i];
				height = videoHeights[i];
			}

			/* check if it is a valid video */
			if (checkValidVideoMode(video_mode) < 0)
				return -1;
			else
				return 1;
		}
	}

	return -1;
}

/* Checks if the input video mode is a valid video mode */
int camera::checkValidVideoMode(dc1394video_mode_t *mode) {
	if (mode == NULL)
		return -1;
	else if (*mode < DC1394_VIDEO_MODE_MIN || *mode > DC1394_VIDEO_MODE_MAX)
		return -1;

	dc1394error_t err;
	dc1394video_modes_t modes;
	
	/* get all supported video modes for camera*/
	err = dc1394_video_get_supported_modes(cam, &modes);

	if (err != DC1394_SUCCESS) 
		fprintf(stderr, "ERROR getting supported videomodes");
	else {
		for (unsigned int i = 0; i < modes.num; i++) {
			if (*mode == modes.modes[i]) 
				return 1;
		}
	}

	return -1;
}

/* Prints the supported video modes */
void camera::printSupportedVideoModes() {
	dc1394error_t err;
	dc1394video_modes_t modes;
	err = dc1394_video_get_supported_modes(cam, &modes);
	
	if (err != DC1394_SUCCESS) 
		fprintf(stderr, "ERROR getting supported videomodes");
	else {
		printf("listing possible video modes\n");
		for (unsigned int i = 0; i < modes.num; i++) {
			printf("mode %d: [%d] %s:\n", i, modes.modes[i], videoModeNames[modes.modes[i] - STARTVIDEOMODE]);
			if (modes.modes[i] < DC1394_VIDEO_MODE_FORMAT7_MIN)
				printSupportedFrameRates(modes.modes[i]);
		}
	}
}

int camera::setVideoMode(const char* video_mode) {
	dc1394video_mode_t mode;
	if (convertVideoMode(video_mode, &mode) < 0) {
		printf("ERROR: invalid video mode: %s\n", video_mode);
		printSupportedVideoModes();
		return -1;
	}

	if (DC1394_SUCCESS != dc1394_video_set_mode(cam, mode))
	{
		fprintf(stderr, "Failed to set the video mode\n");
		return -1;
	}

	_video_mode = mode;
	return 0;
}

int camera::convertFrameRate(float fps, dc1394framerate_t *frame_rate) {
	if (fps < 3.75)
		*frame_rate = DC1394_FRAMERATE_1_875;
	else if (fps < 7.5)
		*frame_rate = DC1394_FRAMERATE_3_75;
	else if (fps < 15)
		*frame_rate = DC1394_FRAMERATE_7_5;
	else if (fps < 30)
		*frame_rate = DC1394_FRAMERATE_15;
	else if (fps < 60)
		*frame_rate = DC1394_FRAMERATE_30;
	else if (fps < 120)
		*frame_rate = DC1394_FRAMERATE_60;
	else if (fps < 240)
		*frame_rate = DC1394_FRAMERATE_120;
	else
		*frame_rate = DC1394_FRAMERATE_240;

	if (checkValidFrameRate(frame_rate) < 0)
		return -1;
	else
		return 1;
}

int camera::checkValidFrameRate(dc1394framerate_t* frame_rate) {
	if (frame_rate == NULL) 
		return -1;
	else if (*frame_rate < DC1394_FRAMERATE_MIN || *frame_rate > DC1394_FRAMERATE_MAX)
		return -1;
	else if (_video_mode < DC1394_VIDEO_MODE_MIN || _video_mode > DC1394_VIDEO_MODE_MAX) {
		fprintf(stderr, "ERROR: Haven't set video mode yet\n");
		return -1;
	}

	dc1394error_t err;
	dc1394framerates_t rates;

	err = dc1394_video_get_supported_framerates(cam, _video_mode, &rates);

	if (err != DC1394_SUCCESS)
		fprintf(stderr, "ERROR getting supported framerates");
	else {
		for (unsigned int i = 0; i < rates.num; i++) {
			if (*frame_rate == rates.framerates[i]) 
				return 1;
		}
	}

	return -1;
}

/* Prints the supported frame rates */
void camera::printSupportedFrameRates(dc1394video_mode_t mode) {
	if (mode < DC1394_VIDEO_MODE_MIN || mode > DC1394_VIDEO_MODE_MAX) {
		fprintf(stderr, "ERROR: Invalid video mode, can't get frame rates");
		return;
	}

	dc1394error_t err;
	dc1394framerates_t rates;
	err = dc1394_video_get_supported_framerates(cam, mode, &rates);
	
	if (err != DC1394_SUCCESS) 
		fprintf(stderr, "ERROR getting supported framerates");
	else {
		printf("Print Supported frame rates for video mode: %s\n", videoModeNames[mode - STARTVIDEOMODE]);
		for (unsigned int i = 0; i < rates.num; i++) {
			printf("    FPS %d: [%d] %d\n", i, rates.framerates[i], videoFrameRates[rates.framerates[i] - STARTFRAMERATE]);
		}
	}
}

void camera::convertBayer(const char* bayer, const char* method)
{
	bayer_met = (dc1394bayer_method_t)-1;
//	bayer_met = DC1394_BAYER_METHOD_NEAREST;
//	bayer_pat = DC1394_COLOR_FILTER_GRBG;
	bayer_pat = DC1394_COLOR_FILTER_BGGR;
}

void camera::clean_up()
{
	if (cam)
	{
		dc1394_capture_stop(cam);
		dc1394_camera_free(cam);
	}
	cam = NULL;
}

int camera::setBrightness(unsigned int brightness)
{
	if (DC1394_SUCCESS != dc1394_feature_set_value(cam, DC1394_FEATURE_BRIGHTNESS, brightness))
	{
		fprintf(stderr, "Unable to set brightness\n");
		return -1;
	}
	return 0;
}

int camera::setExposure(unsigned int exposure)
{
	if (DC1394_SUCCESS != dc1394_feature_set_value(cam, DC1394_FEATURE_EXPOSURE, exposure))
	{
		fprintf(stderr, "Unable to set exposure\n");
		return -1;
	}
	return 0;
}

int camera::setTrigger(int trigger_in)
{
	dc1394switch_t trigger = DC1394_OFF;
	if (trigger_in > 0)
		trigger = DC1394_ON;

	if (DC1394_SUCCESS != dc1394_external_trigger_set_power(cam, trigger))
	{
		fprintf(stderr, "Unable to set trigger mode\n");
		clean_up();
		return -1;
	}
	std::cout << trigger_in << " " << (trigger == DC1394_ON) << std::endl;
	return 0;
}

int camera::setShutter(int shutter)
{
	bool autoShutter = shutter < 0;

	if (DC1394_SUCCESS != dc1394_feature_set_mode(cam, DC1394_FEATURE_SHUTTER, (autoShutter ? DC1394_FEATURE_MODE_AUTO:DC1394_FEATURE_MODE_MANUAL)))
	{
		fprintf(stderr, "Unable to set shutter mode\n");
		clean_up();
		return -1;
	}

	if (!autoShutter)
	{
		if (DC1394_SUCCESS != dc1394_feature_set_value(cam, DC1394_FEATURE_SHUTTER, shutter))
		{
			fprintf(stderr, "Unable to set shutter value\n");
			clean_up();
			return -1;
		}
	}
	return 0;

}

int camera::setGain(int gain)
{
	bool autoGain = gain <0;

	if (DC1394_SUCCESS != dc1394_feature_set_mode(cam, DC1394_FEATURE_GAIN, (autoGain ? DC1394_FEATURE_MODE_AUTO:DC1394_FEATURE_MODE_MANUAL)))
	{
		fprintf(stderr, "Unable to set gain mode\n");
		clean_up();
		return -1;
	}

	if (!autoGain)
	{
		if (DC1394_SUCCESS != dc1394_feature_set_value(cam, DC1394_FEATURE_GAIN, gain))
		{
			fprintf(stderr, "Unable to set gain value\n");
			clean_up();
			return -1;
		}
	}

	return 0;
}

int camera::setWhiteBalance(unsigned int b_u, unsigned int r_v)
{
	if (DC1394_SUCCESS != dc1394_feature_whitebalance_set_value(cam, b_u, r_v))
	{
		fprintf(stderr, "Unable to set white balance value\n");
		clean_up();
		return -1;
	}

	return 0;
}

cv::Mat camera::read()
{
	if (!cam)
	{
		fprintf(stderr, "Camera not initialized\n");
		exit(1);
	}
	
	const int W = width;
	const int H = height;
	dc1394video_frame_t end;
	dc1394video_frame_t * frame;
	dc1394video_frame_t * prev_frame;
	prev_frame = (dc1394video_frame_t *)malloc(sizeof(dc1394video_frame_t));
	prev_frame->id = 255;
	dc1394error_t err;
	end.image = (unsigned char*)malloc(W * H * 3 * sizeof(unsigned char));
	end.color_coding = DC1394_COLOR_CODING_RGB8;
	
	
	int frames_read = 0;
		
	err = dc1394_capture_dequeue(cam, DC1394_CAPTURE_POLICY_WAIT, &frame);
	if (frame != NULL && err == DC1394_SUCCESS)
	{
		dc1394_capture_enqueue(cam, frame);
		memcpy(prev_frame, frame, sizeof(dc1394video_frame_t));
		frames_read++;
	}

	while (1)
	{
		err = dc1394_capture_dequeue(cam, DC1394_CAPTURE_POLICY_POLL, &frame);
		if (frame == NULL && err == DC1394_SUCCESS && prev_frame->id != 255)
			break;
		else if (frame != NULL && err == DC1394_SUCCESS)
		{
			dc1394_capture_enqueue(cam, frame);
			memcpy(prev_frame, frame, sizeof(dc1394video_frame_t));
			frames_read++;
		}
	}
	droppedframes = frames_read - 1;
	prev_frame->color_filter= bayer_pat;

	cv::Mat ret;
	if (bayer_met != -1) {
		if (DC1394_SUCCESS != dc1394_debayer_frames(prev_frame, &end, bayer_met))
		{
			fprintf(stderr, "Unable to debayer frame\n");
		}

		cv::Mat final(H, W, CV_8UC3, end.image);
		final.copyTo(ret);
	} else {
		cv::Mat final(H, W, CV_8UC1, prev_frame->image);
		final.copyTo(ret);
	}

	timestamp = prev_frame->timestamp;
	free(end.image);
	free(prev_frame);
	return ret;
}

long camera::getTimestamp()
{
	return timestamp;
}

int camera::getNumDroppedFrames()
{
	return droppedframes;
}

void camera::printGUID()
{
	printf("GUID of attached camera is: %016lX\n", guid);
}
