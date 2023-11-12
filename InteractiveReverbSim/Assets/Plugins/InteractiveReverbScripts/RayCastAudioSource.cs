using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class RayCastAudioSource : MonoBehaviour
{
    public AudioSource audioSource;
    // public AudioManager audioManager;

    private SphericalRaycast sphericalRaycast = new SphericalRaycast();
    private float obstructedRays = 0.0f;

    // Type of raycast
    public RaycastType raycastType = RaycastType.Random;


    // Start is called before the first frame update
    void Start()
    {
        audioSource = GetComponent<AudioSource>();
        if (audioSource == null)
            audioSource = gameObject.AddComponent<AudioSource>();
    }

    // Update is called once per frame
    void Update()
    {
        if (audioSource.isPlaying)
        {
            // TODO: only call on player or soundsource movement
            obstructedRays = sphericalRaycast.CastSphericalRays(transform.position, raycastType);
            Debug.Log("Obstructed rays: " + obstructedRays);
            if (AudioManager.SetObstructedReflections(obstructedRays) == 0)
            {
                Debug.Log("Error setting the obstructed reflections!");
            }
        }
        // TODO: fix case in which audio source stopped playing.
        // I assume that this would case the echoes to continue playing, which is not what we want.
    }
}
