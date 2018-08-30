#include <math.h>
#include <stddef.h>
#include <stdlib.h>
#include <GLES2/gl2.h>

#include <camera/NdkCameraDevice.h>
#include <camera/NdkCameraCaptureSession.h>
#include <camera/NdkCaptureRequest.h>
#include <media/NdkImage.h>
#include <media/NdkImageReader.h>

#include "native_debug.h"
#include "camera_utils.h"

typedef void (*UnityRenderingEvent)(int eventId);

// NDK Camera API objects (phew ';-)
ACameraManager* manager;
ACameraDevice* device;
AImageReader* reader;
ANativeWindow* window;
ACaptureSessionOutput* output;
ACameraOutputTarget* target;
ACaptureSessionOutputContainer* outputs;
ACameraCaptureSession* session;
ACaptureRequest* requests[1];

// device state callbacks
static ACameraDevice_stateCallbacks camera_callbacks = {
	.context = nullptr,
	.onDisconnected = nullptr,
	.onError = nullptr,
};

// capture session state callbacks
void OnSessionClosed(void* ctx, ACameraCaptureSession* ses) { LOGW("session %p closed", ses); }
void OnSessionReady (void* ctx, ACameraCaptureSession* ses) { LOGW("session %p ready",  ses); }
void OnSessionActive(void* ctx, ACameraCaptureSession* ses) { LOGW("session %p active", ses); }

static ACameraCaptureSession_stateCallbacks session_callbacks = {
	.context = nullptr,
	.onActive = OnSessionActive,
	.onReady = OnSessionReady,
	.onClosed = OnSessionClosed,
};

// capture state callbacks
static ACameraCaptureSession_captureCallbacks capture_callbacks = {
	.context = nullptr,
	.onCaptureStarted = nullptr,
	.onCaptureProgressed = nullptr,
	.onCaptureCompleted = nullptr,
	.onCaptureFailed = nullptr,
	.onCaptureSequenceCompleted = nullptr,
	.onCaptureSequenceAborted = nullptr,
	.onCaptureBufferLost = nullptr,
};


void start_camera() {

	manager = ACameraManager_create();
	//PrintCameras(manager);

	int res = ACameraManager_openCamera( manager, "0", &camera_callbacks, &device ); PrintCameraDeviceError(res);

	// ImageReader -> NativeWindow -> SessionOutput
	res = AImageReader_new( 1280, 720, AIMAGE_FORMAT_YUV_420_888, 2, &reader ); PrintCameraDeviceError(res);
	res = AImageReader_getWindow( reader, &window );  PrintCameraDeviceError(res);// no need to free/release
	res = ACaptureSessionOutput_create( window, &output ); PrintCameraDeviceError(res);
	// TODO: set callback on image reader

	// create an output container with a single output
	res = ACaptureSessionOutputContainer_create( &outputs ); PrintCameraDeviceError(res);
	res = ACaptureSessionOutputContainer_add( outputs, output ); PrintCameraDeviceError(res);

	// create the capture session & repeating request
	res = ACameraDevice_createCaptureSession( device, outputs, &session_callbacks, &session ); PrintCameraDeviceError(res);
	res = ACameraDevice_createCaptureRequest( device, TEMPLATE_PREVIEW, requests ); PrintCameraDeviceError(res);

	// add target window to request, too
	res = ACameraOutputTarget_create( window, &target ); PrintCameraDeviceError(res);
	res = ACaptureRequest_addTarget( requests[0], target ); PrintCameraDeviceError(res);

	// TODO: tweak the capture request?

	// start the actual capture (yay!) int sequence;
	res = ACameraCaptureSession_setRepeatingRequest( session, &capture_callbacks, 1, requests, NULL );  PrintCameraDeviceError(res);// &sequence );
}


void stop_camera() {

	ACameraCaptureSession_stopRepeating(session);

	ACaptureRequest_free(requests[0]);
	ACameraCaptureSession_close(session);
	ACameraOutputTarget_free(target);

	ACaptureSessionOutput_free(output);
	ACaptureSessionOutputContainer_free(outputs);

	AImageReader_delete(reader);
	ACameraDevice_close(device);
	ACameraManager_delete(manager);
}



// --------------------------------------------------------------------------
// SetTimeFromUnity, an example function we export which is called by one of the scripts.

static float g_Time;

extern "C" void SetTimeFromUnity (float t) { g_Time = t; }

// --------------------------------------------------------------------------
// SetTextureFromUnity, an example function we export which is called by one of the scripts.

static void* g_TextureHandle = NULL;
static int   g_TextureWidth  = 0;
static int   g_TextureHeight = 0;

extern "C" void SetTextureFromUnity(void* textureHandle, int w, int h)
{
	// A script calls this at initialization time; just remember the texture pointer here.
	// Will update texture pixels each frame from the plugin rendering event (texture update
	// needs to happen on the rendering thread).
	g_TextureHandle = textureHandle;
	g_TextureWidth = w;
	g_TextureHeight = h;

	start_camera();
}

static void ModifyTexturePixels()
{
	void* textureHandle = g_TextureHandle;
	int width = g_TextureWidth;
	int height = g_TextureHeight;
	if (!textureHandle)
		return;

	int textureRowPitch = width * 4;
	// Just allocate a system memory buffer here for simplicity
	void* textureDataPtr = new unsigned char[textureRowPitch * height];
	if (!textureDataPtr)
		return;

	const float t = g_Time * 4.0f;

	unsigned char* dst = (unsigned char*)textureDataPtr;
	for (int y = 0; y < height; ++y)
	{
		unsigned char* ptr = dst;
		for (int x = 0; x < width; ++x)
		{
			// Simple "plasma effect": several combined sine waves
			int vv = int(
				(127.0f + (127.0f * sinf(x / 7.0f + t))) +
				(127.0f + (127.0f * sinf(y / 5.0f - t))) +
				(127.0f + (127.0f * sinf((x + y) / 6.0f - t))) +
				(127.0f + (127.0f * sinf(sqrtf(float(x*x + y*y)) / 4.0f - t)))
				) / 4;

			// Write the texture pixel
			ptr[0] = vv;
			ptr[1] = vv;
			ptr[2] = vv;
			ptr[3] = vv;

			// To next pixel (our pixels are 4 bpp)
			ptr += 4;
		}

		// To next image row
		dst += textureRowPitch;
	}

	GLuint gltex = (GLuint)(size_t)(textureHandle);
	// Update texture data, and free the memory buffer
	glBindTexture(GL_TEXTURE_2D, gltex);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, textureDataPtr);

	delete[](unsigned char*)textureDataPtr;
}

static void OnRenderEvent(int eventID)
{
	AImage* image;
	int status = AImageReader_acquireLatestImage( reader, &image );

	if (image) {
		int32_t height,np;
		AImage_getHeight(image,&height);
		AImage_getNumberOfPlanes(image,&np);
		LOGI("image height: %d, planes: %d",height,np);

		uint8_t* data; int length;
		AImage_getPlaneData(image, 0, &data, &length );

		GLuint gltex = (GLuint)(size_t)(g_TextureHandle);
		// Update texture data, and free the memory buffer
		glBindTexture(GL_TEXTURE_2D, gltex);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 256, 256, GL_RGBA, GL_UNSIGNED_BYTE, data);

	}

	// return the buffer for reuse (important!)
	AImage_delete(image);
}

// --------------------------------------------------------------------------
// GetRenderEventFunc, an example function we export which is used to get a rendering event callback function.

extern "C" UnityRenderingEvent GetRenderEventFunc()
{
	return OnRenderEvent;
}

