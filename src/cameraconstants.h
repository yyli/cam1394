#ifndef _CAMERACONSTANTS_H_
#define _CAMERACONSTANTS_H_

const int STARTVIDEOMODE = DC1394_VIDEO_MODE_MIN;

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

const int STARTFRAMERATE = DC1394_FRAMERATE_MIN;

const int videoFrameRates[] = {
	1.875,
	3.75,
	7.5,
	15,
	30,
	60,
	120,
	240
};

const int STARTBAYERMETHODS = DC1394_BAYER_METHOD_MIN;

const char *bayerMethods[] = {
	"NEAREST",
	"SIMPLE",
	"BILINEAR",
	"HQLINEAR",
	"DOWNSAMPLE",
	"EDGESENSE",
	"VNG",
	"AHD"
};

const int STARTCOLORFILTER = DC1394_COLOR_FILTER_MIN;

const char *bayerPatterns[] = {
	"RGGB",
	"GBRG",
	"GRBG",
	"BGGR"
};
#endif
