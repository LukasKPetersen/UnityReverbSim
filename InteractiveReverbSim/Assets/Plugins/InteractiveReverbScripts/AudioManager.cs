using System.Collections.Generic;
using UnityEngine;
using System.Runtime.InteropServices; // for communicating with the reverb plugin

public class AudioManager : MonoBehaviour
{    
    /* * * Declare the native functions using DllImport * * */
    // function for checking the connection to the plugin
    [DllImport("audioplugin_SpatiotemporalReverb", CallingConvention = CallingConvention.Cdecl)]
    [return: MarshalAs(UnmanagedType.I1)] // return a boolean
    public static extern bool TestUnityConnection();

    // function for applying audio positioning
    [DllImport("audioplugin_SpatiotemporalReverb", CallingConvention = CallingConvention.Cdecl)]
    public static extern int ApplyAudioPositioning(float panInfo, float amplitude);
    
    private void Awake()
    {
        TestConnectionToJuce();
    }

    public void ApplyRaycastResult(RaycastResult raycastResult)
    {
        // map the panInformation to a value between 0 and 1 (since JUCE parameters are always interpreted as values between 0 and 1 in Unity)
        float panInfoJUCE = Map(raycastResult.panInformation, 180.0f, 0.0f, 0.0f, 1.0f);

        // calculate the amplitude
        float amplitudeJUCE = raycastResult.amplitude * 80.0f * Map(raycastResult.frontBackInformation, 180.0f, 0.0f, 0.3f, 1.0f);
        
        if (ApplyAudioPositioning (panInfoJUCE, amplitudeJUCE) == 0)
        {
            Debug.Log("Error applying audio positioning!");
        }
    }

    public void TestConnectionToJuce() 
    {
        if (TestUnityConnection())
        {
            Debug.Log("Connection to plugin successful!");
        }
        else
        {
            Debug.Log("Connection to plugin failed!");
        }
    }

    public float Map(float source, float sourceMin, float sourceMax, float targetMin, float targetMax) 
    {
        return targetMin + ((targetMax - targetMin) * (source - sourceMin)) / (sourceMax - sourceMin);
    }
}