# UnityReverbSim
Source code for my Bachelor Project: Interactive Spatiotemporal Simulation of Reverberation

## Interface between JUCE and Unity
### Setup
The setup between JUCE and Unity is mainly fleshed out in the JUCE project.
1. In the file `juce_UnityPluginInterface.h`, add each of the functions within JUCE that you want to make visible to Unity.
2. In the file `juce_audio_plugin_client_Unity.cpp`, implement those same functions. Often, the implementation on this level would make a function call to another function implememted at a deeper level. So in most cases, the functions implemented at this level are a sort of intermediary functions.
#### Add a 'MyAudioProcessor.h' file
3. In the folder `JUCE/modules/juce_audio_plugin_client/Unity`, add a new file; `MyAudioProcessor.h` (doesn't have to be in this location or with this file name).
4. In the `MyAudioProcessor.h` file, add the function declarations that you want to implement in the `PluginProcessor.cpp` (likely matching those that you implemented in the `juce_audio_plugin_client_Unity.cpp` file).
