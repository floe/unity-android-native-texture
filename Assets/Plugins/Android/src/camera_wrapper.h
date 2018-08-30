#ifndef __CAMERA_WRAPPER_H__
#define __CAMERA_WRAPPER_H__

#include <camera/NdkCameraManager.h>
#include <camera/NdkCameraDevice.h>
#include <camera/NdkCameraCaptureSession.h>
#include <camera/NdkCaptureRequest.h>
#include <media/NdkImage.h>
#include <media/NdkImageReader.h>

#include <android/log.h>

#define LOG_TAG "camera-wrapper"

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,  LOG_TAG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN,  LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

void open_camera( int w, int h, int format = AIMAGE_FORMAT_YUV_420_888 );
void close_camera();

void start_camera();
void stop_camera();

AImage* acquire_image();
uint8_t* get_image_plane( AImage* image, int num );
void release_image( AImage* image );

#endif // __CAMERA_WRAPPER_H__
