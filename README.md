# UnityReverbSim
Source code for my Bachelor Project: Interactive Spatiotemporal Simulation of Reverberation

## Interface between JUCE and Unity
### Setup
The setup between JUCE and Unity is mainly fleshed out in the JUCE project. Note that the following guide includes changes to the JUCE library source code, so it might affect other JUCE projects that uses the same library code.
1. In the file `juce_UnityPluginInterface.h`, add each of the functions within JUCE that you want to make visible to Unity.
2. In the file `juce_audio_plugin_client_Unity.cpp`, implement those same functions. Often, the implementation on this level would make a function call to another function implememted at a deeper level. So in most cases, the functions implemented at this level are sort of intermediary functions.
3. Above the function implementations in the `juce_audio_plugin_client_Unity.cpp` file, add this line: `OwnedArray<AudioProcessorUnityWrapper> unityWrappers;`.
#### Add a 'MyAudioProcessor.h' file
4. In the folder `JUCE/modules/juce_audio_plugin_client/Unity`, add a new file; `MyAudioProcessor.h` (doesn't have to be in this location or with this file name).
5. In the `MyAudioProcessor.h` file, add the function declarations that you want to implement in the `PluginProcessor.cpp` (likely matching those that you implemented in the `juce_audio_plugin_client_Unity.cpp` file).
6. In the `juce_audio_plugin_client_Unity.cpp` file, within the `AudioProcessorUnityWrapper` class, change the type of `pluginInstance` to `std::unique_ptr<MyAudioProcessor>`. Also, in the constructor of the same class, initialize `pluginInstance` as `createPluginFilterOfTypeNEW (MyAudioProcessor::wrapperType_Unity)`.
7. Implement the `createPluginFilterOfTypeNEW()` function in the file `juce_CreatePluginFilter.h`. This function is the exact same as the normal `createPluginFilterOfType()` function, except it uses `MyAudioProcessor` instead of `AudioProcessor` and it calls the function `createPluginFilterNEW()` instead of the function `createPluginFilterNEW()`. Remember to `#include` the `MyAudioProcessor.h` file.
8. Implement the `createPluginFilterNEW()` function in the file `PluginProcessor.cpp`. This function is the same as the `createPluginFilter()` function, except it returns a `MyAudioProcessor*`.
9. Change the `PluginProcessor.h` file such that it extends `MyAudioProcessor` and remember to `#implement` the `MyAudioProcessor.h` file.
10. Change the `PluginProcessor.cpp` file such that the constructor extends `MyAudioProcessor`.
11. Back in the `AudioProcessorUnityWrapper` class, add a public getter function for `pluginInstance`.
12. In the `getEffectDefinition()` function in the `juce_audio_plugin_client_Unity.cpp` file, change the lambda functions `create()`, release()`, and `reset()` to add, remove, and reset the `pluginInstance` object respectively.
#### Call the custom functions from Unity
13. In the Unity script from which you wish to call the custom function in JUCE, add the following lines of code:
    ```
    [DllImport("audioplugin_<plugin_name>", CallingConvention = CallingConvention.Cdecl)]
    public static extern int MyCustomJUCEFunction(int x, int y);
    ```
14. Build your JUCE application and drag the `.bundle` file into `Assets/Plugins/` in your Unity project.
15. Create a new Audio Mixer in Unity and add the JUCE plugin to the mixer.
### Troubleshooting
If you get the error: `EntryPointNotFoundException: <function_name()> assembly:<unknown assembly> type:<unknown type> member:(null)`, make sure that you have enabled testability for debug builds in the Build Settings of your Xcode project.
