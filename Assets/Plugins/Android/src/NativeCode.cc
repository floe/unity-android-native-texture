#include <stddef.h>
#include <stdlib.h>
#include <GLES2/gl2.h>

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
}


extern "C" float add(float x, float y)
{
	return x + y;
}
