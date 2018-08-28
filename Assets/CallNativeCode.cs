using UnityEngine;
using System.Collections;
using System.Runtime.InteropServices;

public class CallNativeCode : MonoBehaviour {

	[DllImport("native")]
	private static extern float add(float x, float y);

	[DllImport ("native")]
	private static extern void SetTextureFromUnity(System.IntPtr texture, int w, int h);

	void OnGUI ()
	{
		float x = 3;
		float y = 10;
		GUI.Label (new Rect (15, 125, 450, 100), "adding " + x  + " and " + y + " in native code equals " + add(x,y));
	}

	private void CreateTextureAndPassToPlugin()
	{
		// Create a texture
		Texture2D tex = new Texture2D(256,256,TextureFormat.ARGB32,false);
		// Set point filtering just so we can see the pixels clearly
		tex.filterMode = FilterMode.Point;
		// Call Apply() so it's actually uploaded to the GPU
		tex.Apply();

		// Set texture onto our material
		GetComponent<Renderer>().material.mainTexture = tex;

		// Pass texture pointer to the plugin
		SetTextureFromUnity (tex.GetNativeTexturePtr(), tex.width, tex.height);
	}
}
