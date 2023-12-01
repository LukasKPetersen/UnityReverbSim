using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class RayCastAudioSource : MonoBehaviour
{
    public AudioSource audioSource;
    public AudioManager audioManager;

    private SphericalRaycast sphericalRaycast = new SphericalRaycast();

    // Type of raycast
    public RaycastType raycastType = RaycastType.Random;

    // Variables for the plugin
    private float obstructedRays = 0.0f;
    private float longestDistance = 0.0f;
    private float averageDistance = 0.0f;
    private float averageAbsorption = 0.0f;

    // Temp variables
    private float obstructedRaysTemp = 0.0f;
    private float longestDistanceTemp = 0.0f;
    private float averageDistanceTemp = 0.0f;
    private float averageAbsorptionTemp = 0.0f;

    bool playerMoved = false;

    void Start()
    {
        audioSource = GetComponent<AudioSource>();
        if (audioSource == null)
            audioSource = gameObject.AddComponent<AudioSource>();
    }

    void LateUpdate()
    {
        if (playerMoved)
        {
            if (audioSource.isPlaying)
            {
                // communicate values to the plugin (Note: we only call the functions if the values have changed)

                // Set the filter value for the reverb effect
                obstructedRays = sphericalRaycast.CastSphericalRays(transform.position, raycastType);
                if (obstructedRays != obstructedRaysTemp)
                {
                    audioManager.SendObstructionReflections(obstructedRays);
                    obstructedRaysTemp = obstructedRays;
                }

                // Set the diffusion size for the reverb effect
                longestDistance = sphericalRaycast.GetLongestDistance();
                if (longestDistance != longestDistanceTemp)
                {
                    audioManager.ApplyDiffusionTime(longestDistance);
                    longestDistanceTemp = longestDistance;
                }

                // Set the delay time for the reverb effect
                averageDistance = sphericalRaycast.GetAverageDistance();
                if (averageDistance != averageDistanceTemp)
                {
                    audioManager.ApplyDelayTime(averageDistance);
                    averageDistanceTemp = averageDistance;
                }

                // Set the feedback for the reverb effect
                averageAbsorption = sphericalRaycast.GetAverageAbsorption();
                if (averageAbsorption != averageAbsorptionTemp)
                {
                    audioManager.ApplyFeedback(averageAbsorption);
                    averageAbsorptionTemp = averageAbsorption;
                }
            }
        }

        playerMoved = false;
    }

    public void SetPlayerMoved()
    {
        playerMoved = true;
    }
}
