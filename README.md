# unity-android-native-texture

Minimal example of how to dynamically update a texture in Unity on Android from native C++ code.  
Should also work on Linux deployments with minimal changes, not tested yet.

Note: before deploying the Unity project, you need to build the native library in `Assets/Plugins/Android/src/` by running `build_plugin.sh`. This uses the old-style ndk-build process, should be upgraded to CMake and properly integrated with Unity at some point.

Built/tested with:
  - Unity 2018.2.4f1
  - Android NDK r15c
  - Android 8.0 Oreo
  - API level 24

Sources:
  - https://docs.unity3d.com/Manual/AndroidNativePlugins.html
  - https://bitbucket.org/Unity-Technologies/graphicsdemos/src/364ac57cea5c197ca9b7015ba29dcc1ff94c9f61/NativeRenderingPlugin?at=default 
