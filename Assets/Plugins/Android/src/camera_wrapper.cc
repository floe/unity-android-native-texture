#include <stdint.h>

#include "camera_wrapper.h"

#define CheckCameraError(...) res = __VA_ARGS__; if (res != ACAMERA_OK) { __android_log_print(ANDROID_LOG_ERROR, "native-camera", "%s: -> %d", #__VA_ARGS__, res); return; }
int res = ACAMERA_ERROR_UNKNOWN;

// NDK Camera API objects
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
static ACameraCaptureSession_stateCallbacks session_callbacks = {
	.context = nullptr,
	.onActive = nullptr,
	.onReady = nullptr,
	.onClosed = nullptr,
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

void open_camera( int w, int h, int format ) {

	manager = ACameraManager_create();
	CheckCameraError( (manager == nullptr) );

	CheckCameraError( ACameraManager_openCamera( manager, "0", &camera_callbacks, &device ) );

	// ImageReader -> NativeWindow -> SessionOutput
	CheckCameraError( AImageReader_new( w, h, format, 2, &reader ) );
	CheckCameraError( AImageReader_getWindow( reader, &window ) ); // no need to free/release
	CheckCameraError( ACaptureSessionOutput_create( window, &output ) );

	// create an output container with a single output
	CheckCameraError( ACaptureSessionOutputContainer_create( &outputs ) );
	CheckCameraError( ACaptureSessionOutputContainer_add( outputs, output ) );

	// create the capture session & repeating request
	CheckCameraError( ACameraDevice_createCaptureSession( device, outputs, &session_callbacks, &session ) );
	CheckCameraError( ACameraDevice_createCaptureRequest( device, TEMPLATE_PREVIEW, requests ) );

	// add target window to request, too
	CheckCameraError( ACameraOutputTarget_create( window, &target ) );
	CheckCameraError( ACaptureRequest_addTarget( requests[0], target ) );

	// TODO: tweak the capture request?
	/* right now, we rely on the capture session to select a suitable stream size that's sufficiently
	   close to the surface size (and for a standard size like 1280x720, this will probably work)

	ACameraMetadata* metadata;
	ACameraManager_getCameraCharacteristics(m_camera_manager,
	                                        m_selected_camera_id, &metadata);
	ACameraMetadata_const_entry entry;
	ACameraMetadata_getConstEntry( metadata, ACAMERA_SCALER_AVAILABLE_STREAM_CONFIGURATIONS, &entry);
	// format of the data: format, width, height, input?, type int32
	*/
}

void start_camera() {
	// start the actual capture (yay!) 
	CheckCameraError( ACameraCaptureSession_setRepeatingRequest( session, &capture_callbacks, 1, requests, NULL ) );
}

void stop_camera() {
	CheckCameraError( ACameraCaptureSession_stopRepeating(session) );
}

void close_camera() {

	ACaptureRequest_free(requests[0]);
	ACameraCaptureSession_close(session);
	ACameraOutputTarget_free(target);

	ACaptureSessionOutput_free(output);
	ACaptureSessionOutputContainer_free(outputs);

	AImageReader_delete(reader);
	ACameraDevice_close(device);
	ACameraManager_delete(manager);
}

AImage* acquire_image() {
	AImage* image;
	media_status_t status = AImageReader_acquireLatestImage( reader, &image );
	if (status != AMEDIA_OK) return nullptr;
	return image;
}

uint8_t* get_image_plane(AImage* image, int num) {
	//int32_t height,np;
	//AImage_getHeight(image,&height);
	//AImage_getNumberOfPlanes(image,&np);
	//LOGI("image height: %d, planes: %d",height,np);
	uint8_t* data; int length;
	media_status_t status = AImage_getPlaneData(image, num, &data, &length );
	if (status != AMEDIA_OK) return nullptr;
	return data;
}

void release_image( AImage* image ) {
	// return the buffer for reuse (important!)
	AImage_delete( image );
}
