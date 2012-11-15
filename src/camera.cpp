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

#include "camera.h"
#include <stdio.h>
#include "highgui.h"

using namespace cam1394;

//! \internal
const int STARTVIDEOMODE = DC1394_VIDEO_MODE_MIN;

//! \hideinitializer
const int videoWidths[] = {
	160,
	320,
	640,
	640,
	640,
	640,
	640,
	800,
	800,
	800,
	1024,
	1024,
	1024,
	800,
	1024,
	1280,
	1280,
	1280,
	1600,
	1600,
	1600,
	1280,
	1600
};

//! \hideinitializer
const int videoHeights[] = {
	120,
	240,
	480,
	480,
	480,
	480,
	480,
	600,
	600,
	600,
	768,
	768,
	768,
	600,
	768,
	960,
	960,
	960,
	1200,
	1200,
	1200,
	960,
	1200
};

//! \showinitializer
const char *videoModeNames[] = {
	"160x120_YUV444",
	"320x240_YUV422",
	"640x480_YUV411",
	"640x480_YUV422",
	"640x480_RGB8",
	"640x480_MONO8",
	"640x480_MONO16",
	"800x600_YUV422",
	"800x600_RGB8",
	"800x600_MONO8",
	"1024x768_YUV422",
	"1024x768_RGB8",
	"1024x768_MONO8",
	"800x600_MONO16",
	"1024x768_MONO16",
	"1280x960_YUV422",
	"1280x960_RGB8",
	"1280x960_MONO8",
	"1600x1200_YUV422",
	"1600x1200_RGB8",
	"1600x1200_MONO8",
	"1280x960_MONO16",
	"1600x1200_MONO16",
	"EXIF",
	"FORMAT7_0",
	"FORMAT7_1",
	"FORMAT7_2",
	"FORMAT7_3",
	"FORMAT7_4",
	"FORMAT7_5",
	"FORMAT7_6",
	"FORMAT7_7"
};

/* defualt constructor */
camera::camera() : width(-1), height(-1), cam(NULL) {}

/* destructor */
camera::~camera()
{
	clean_up();
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

		sprintf(temp,"%06x%02x%08x", value[2], value[1], value[0]);

		if (!strcmp(cam_guid, "NONE"))
		{
//#ifdef DEBUGCAMERA
			fprintf(stderr, "WARNING: No guid specified, using first camera. GUID: %s\n", temp);
//#endif
			guid = value[0] | ((long)value[1] << 32) | ((long)value[2] << 40);
			break;
		}

		if (!strcmp(temp, cam_guid))
		{
#ifdef DEBUGCAMERA
			fprintf(stderr, "Camera %s found\n", cam_guid);
#endif
			guid = value[0] | ((long)value[1] << 32) | ((long)value[2] << 40);
			break;
		}
		puts("cleaned_up");
		clean_up();
	}
	free(temp);
	dc1394_camera_free_list(list);
	
	if (!cam)
	{
		if (!strcmp(cam_guid, "NONE"))
		{
			fprintf(stderr, "Can't find camera\n");
		}
		else
		{
			fprintf(stderr, "Can't find camera with guid %s\n", cam_guid);
		}
		return -1;
	}
	
	dc1394framerate_t fr = convertFrameRate(fps);
	dc1394video_mode_t vid_mode;
	if (getVideoMode(video_mode, &vid_mode) < 0) {
		printf("ERROR: invalid video mode: %s\n", video_mode);
		printf("listing possible modes\n");
		printSupportedVideoModes();
		return -1;
	}

	bool Set_Success =  true;
	if (DC1394_SUCCESS != dc1394_video_set_framerate(cam, fr))
	{
		fprintf(stderr, "Failed to set the framerate\n");
		Set_Success = false;
	}
	if (DC1394_SUCCESS != dc1394_video_set_mode(cam, vid_mode))
	{
		fprintf(stderr, "Failed to set the video mode\n");
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

/* Prints the supported video modes */
void camera::printSupportedVideoModes() {
	dc1394error_t err;
	dc1394video_modes_t modes;
	err = dc1394_video_get_supported_modes(cam, &modes);
	
	if (err != DC1394_SUCCESS) 
		fprintf(stderr, "ERROR getting supported videomodes");
	else {
		for (unsigned int i = 0; i < modes.num; i++) {
			printf("mode %d: [%d] %s\n", i, modes.modes[i], videoModeNames[modes.modes[i] - STARTVIDEOMODE]);
		}
	}
}

/* Checks if the input video mode is a valid video mode */
int camera::checkValidVideoMode(dc1394video_mode_t *mode) {
	if (*mode == 0)
		return 0;

	dc1394error_t err;
	dc1394video_modes_t modes;
	
	err = dc1394_video_get_supported_modes(cam, &modes);
	if (err != DC1394_SUCCESS) 
		fprintf(stderr, "ERROR getting supported videomodes");
	else {
		for (unsigned int i = 0; i < modes.num; i++) {
			if (*mode == modes.modes[i])
				return 1;
		}
	}

	return 0;
}

dc1394framerate_t camera::convertFrameRate(float fps)
{
	if (fps < 3.75)
		return DC1394_FRAMERATE_1_875;
	else if (fps < 7.5)
		return DC1394_FRAMERATE_3_75;
	else if (fps < 15)
		return DC1394_FRAMERATE_7_5;
	else if (fps < 30)
		return DC1394_FRAMERATE_15;
	else if (fps < 60)
		return DC1394_FRAMERATE_30;
	else
		return DC1394_FRAMERATE_60;
}

/* Sets video_mode based on string input */
int camera::getVideoMode(const char* mode, dc1394video_mode_t *video_mode)
{
	*video_mode = (dc1394video_mode_t)0;
	for (size_t i = 0; i < 32; i++) {
		if (!strcmp(mode, videoModeNames[i])) {
			*video_mode = (dc1394video_mode_t)(i + STARTVIDEOMODE);
			if (i < 23) {
				width = videoWidths[i];
				height = videoHeights[i];
			}
		}
	}

	if (checkValidVideoMode(video_mode))
		return 1;
	else
		return -1;
}

void camera::convertBayer(const char* bayer, const char* method)
{
	bayer_met = DC1394_BAYER_METHOD_SIMPLE;
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
	if (DC1394_SUCCESS != dc1394_debayer_frames(prev_frame, &end, bayer_met))
	{
		fprintf(stderr, "Unable to debayer frame\n");
	}

	cv::Mat final(H, W, CV_8UC3, end.image);
	cv::Mat ret;
	final.copyTo(ret);
	timestamp = prev_frame->timestamp;
	free(prev_frame);
	free(end.image);
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

long camera::getGUID()
{
	return guid;
}
