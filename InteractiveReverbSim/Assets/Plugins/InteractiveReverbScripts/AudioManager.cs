using System.Collections.Generic;
using UnityEngine;
using System.Runtime.InteropServices; // for communicating with the reverb plugin

public class AudioManager : MonoBehaviour
{    
    public enum soundMedium { air, water, metal, wood, glass, concrete, carpet, grass, sand, snow };
    public soundMedium medium = soundMedium.air;

    // scaling factor for delay times
    public float delayScalingFactor = 1.0f;

    /* * * Declare the native functions using DllImport * * */
    // function for checking the connection to the plugin
    [DllImport("audioplugin_SpatiotemporalReverb", CallingConvention = CallingConvention.Cdecl)]
    [return: MarshalAs(UnmanagedType.I1)] // return a boolean
    public static extern bool TestUnityConnection();

    // function for applying audio positioning
    [DllImport("audioplugin_SpatiotemporalReverb", CallingConvention = CallingConvention.Cdecl)]
    public static extern int ApplyAudioPositioning (float panInfo, float frontBackInfo, float distance, float transmission, float filterCoefLeft, float filterCoefRight);
    
    [DllImport("audioplugin_SpatiotemporalReverb", CallingConvention = CallingConvention.Cdecl)]
    public static extern int SetObstructedReflections (float obstructedReflections);
    
    [DllImport("audioplugin_SpatiotemporalReverb", CallingConvention = CallingConvention.Cdecl)]
    public static extern int SetDiffusionSize (float diffusionTime);
    
    [DllImport("audioplugin_SpatiotemporalReverb", CallingConvention = CallingConvention.Cdecl)]
    public static extern int SetDelayTime (float delayTime);

    private void Awake()
    {
        TestConnectionToJuce();
    }

    public void ApplyRaycastResult(RaycastResult raycastResult)
    {
        // map the panInformation to a value between 0 and 1 (since JUCE parameters are always interpreted as values between 0 and 1 in Unity)
        // TODO: Why was this again?????
        float panInfoJUCE = Map(raycastResult.panInformation, 180.0f, 0.0f, 0.0f, 1.0f);

        // if the player has their back to the audio source, the filter channel information is inverted
        // TODO: make this cooler
        int left = raycastResult.frontBackInformation > 90.0f ? 1 : 0;
        int right = raycastResult.frontBackInformation > 90.0f ? 0 : 1;

        if (ApplyAudioPositioning  (panInfoJUCE, 
                                    raycastResult.frontBackInformation, 
                                    raycastResult.distanceTravelled, 
                                    raycastResult.soundReduction,
                                    raycastResult.filterCoefficients[left],
                                    raycastResult.filterCoefficients[right]) == 0) 
        { 
            Debug.Log("Error applying audio positioning!"); 
        }
        // else 
        // { 
        //     Debug.Log("Successfully applied audio positioning!"); 
        // }
    }

    public void SendObstructionReflections(float obstructedRays)
    {
        if (SetObstructedReflections(obstructedRays) == 0)
        {
            Debug.Log("Error setting the obstructed reflections!");
        }
        // else
        // {
        //     Debug.Log("Successfully set the obstructed reflections!");
        // }
    }

    public void ApplyDiffusionTime(float diffusionTime)
    {
        if (SetDiffusionSize(diffusionTime / getSoundSpeed(soundMedium.air)) == 0)
        {
            Debug.Log("Error applying diffusion time!");
        }
        // else
        // {
        //     Debug.Log("Successfully set the diffusion time!");
        // }
    }

    public void ApplyDelayTime(float distance)
    {
        if (SetDelayTime(distance / getSoundSpeed(soundMedium.air)) == 0)
        {
            Debug.Log("Error applying delay time!");
        }
        // else
        // {
        //     Debug.Log("Successfully set the delay time!");
        // }
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

    public float calculateDelay(float distanceTravelled)
    {
        return distanceTravelled / getSoundSpeed(medium) * delayScalingFactor;
    }

    // TODO: double check values
    public float getSoundSpeed(soundMedium medium)
    {
        switch (medium)
        {
            case soundMedium.air:
                return 343.0f;
            case soundMedium.water:
                return 1482.0f;
            case soundMedium.metal:
                return 6420.0f;
            case soundMedium.wood:
                return 3760.0f;
            case soundMedium.glass:
                return 4540.0f;
            case soundMedium.concrete:
                return 3430.0f;
            case soundMedium.carpet:
                return 1500.0f;
            case soundMedium.grass:
                return 1500.0f;
            case soundMedium.sand:
                return 1500.0f;
            case soundMedium.snow:
                return 1500.0f;
            default:
                return 343.0f;
        }
    }
}