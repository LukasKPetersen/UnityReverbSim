using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class RayCastAudioSource : MonoBehaviour
{
    public AudioSource audioSource;
    public GameObject listener;
    public Visualization visualization = new Visualization();

    public AudioManager audioManager;

    public float sampleRate = 44100.0f;

    // Ray parameters
    public int rayCount = 300;
    public float rayRange = 5.0f;
    public bool debug = true;
    public bool visualize = true;
    public bool groupRays = true;

    // Type of raycast
    public enum RaycastType 
    {
        RegularEquidistributed, 
        RandomEquidistributed,
        Random 
    };
    public RaycastType raycastType = RaycastType.Random;

    // The number of frequency bands to analyze
    public int numBands = 64;

    // Array to store the amplitude values for each frequency band
    private float[] spectrumData;

    // List to store the raycast results
    private List<RaycastResult> raycastResults = new List<RaycastResult>();

    // Start is called before the first frame update
    void Start()
    {
        audioSource = GetComponent<AudioSource>();
        if (audioSource == null)
            audioSource = gameObject.AddComponent<AudioSource>();


        // Find and initialize the ImpactSphere GameObject
        listener = GameObject.Find("Listener");
        if (listener == null)
        {
            Debug.LogError("Listener not found! Make sure it has the correct name in the script.");
        }

        audioSource.volume = 1.0f;
        audioSource.loop = true;
        audioSource.playOnAwake = true;

        // Initialize the spectrumData array with the number of bands
        spectrumData = new float[numBands];

        // We call a repeating function that will send the raycast results to the audioManager
        // This is done at the sample rate of the audio source
        // InvokeRepeating("SendRaycastResults", 0.0f, 1.0f / sampleRate);
    }

    // void SendRaycastResults()
    // {
        // // send the raycast results to the audioManager and on to the JUCE plugin
        // if (audioManager != null && raycastResults.Count > 0)
        // {
        //     audioManager.ApplyRaycastResults(raycastResults);
        //     // reset the raycast results every frame
        //     raycastResults.Clear();
        // }
        // else
        // {
        //     (if debug) Debug.Log("AudioManager not selected!");
        // }
    // }

    // Update is called once per frame
    void Update()
    {
        // Get the amplitude of the audio source
        float amplitude = 0.0f;
        if (audioSource.isPlaying)
        {
            // Calculate the average amplitude
            audioSource.GetSpectrumData(spectrumData, 0, FFTWindow.Rectangular); // or perhaps with FFTWindow.BlackmanHarris?
            for (int i = 0; i < numBands; i++)
            {
                amplitude += spectrumData[i];
            }
            amplitude /= numBands;

            if (groupRays)
            {
                CastSphericalRays(amplitude);
            
                // send the raycast results to the audioManager and on to the JUCE plugin
                if (audioManager != null)
                {
                    // we clear previous echoes from last frame
                    if (AudioManager.ClearEchoes() == 0)
                    {
                        Debug.Log("Error clearing echoes!");
                    }

                    if (raycastResults.Count > 0)
                    {
                        audioManager.ApplyRaycastResults(raycastResults);
                        // reset the raycast results every frame
                        raycastResults.Clear();
                    }
                    else
                    {
                        if (debug) Debug.Log("No listener hits!");
                    }
                }
                else
                {
                    if (debug) Debug.Log("AudioManager not selected!");
                }
            }
            else
            {
                if (audioManager != null)
                {
                    // we clear previous echoes from last frame
                    if (AudioManager.ClearEchoes() == 0)
                    {
                        Debug.Log("Error clearing echoes!");
                    }

                    CastSphericalRays(amplitude);
                }
            }
        }
        // TODO: fix case in which audio source stopped playing. 
        // I assume that this would case the echoes to continue playing, which is not what we want.
    }
    
    private void CastSphericalRays(float amplitude) 
    {
        Vector3 origin = transform.position;

        // Choose what raycast algorithm to use
        switch (raycastType)
        {
            case RaycastType.RegularEquidistributed:
                RegularEquidistributedSphericalPlacement(origin, amplitude);
                break;
            case RaycastType.RandomEquidistributed:
                RandomEquidistributedSphericalPlacement(origin, amplitude);
                break;
            case RaycastType.Random:
                RandomSphericalPlacement(origin, amplitude);
                break;
            default:
                RandomSphericalPlacement(origin, amplitude);
                break;
        }
    }

    private void RegularEquidistributedSphericalPlacement(Vector3 origin, float amplitude)
    {
        // algorithm from 'How to generate equidistributed points on the surface of a sphere' by Markus Deserno
        float a = 4 * Mathf.PI / rayCount;
        float d = Mathf.Sqrt(a);
        int mTheta = Mathf.RoundToInt(Mathf.PI / d);
        float dTheta = Mathf.PI / mTheta;
        float dPhi = a / dTheta;

        for (int i = 0; i < mTheta; i++)
        {
            float theta = Mathf.PI * (i + 0.5f) / mTheta;
            float mPhi = Mathf.RoundToInt(2 * Mathf.PI * Mathf.Sin(theta) / dPhi);

            for (int j = 0; j < mPhi; j++)
            {
                float phi = 2 * Mathf.PI * j / mPhi;
                // initialize ray with calculated direction
                Vector3 direction = new Vector3(Mathf.Sin(theta) * Mathf.Cos(phi), Mathf.Sin(theta) * Mathf.Sin(phi), Mathf.Cos(theta));

                CastRay(origin, direction, amplitude, 0.0f, 0);
            }
        }
    }

    private void RandomSphericalPlacement(Vector3 origin, float amplitude)
    {
        for (int i = 0; i < rayCount; i++)
        {
            // initialize ray with random direction
            Vector3 direction = Random.onUnitSphere;

            CastRay(origin, direction, amplitude, 0.0f, 0);
        }
    }

    private void RandomEquidistributedSphericalPlacement(Vector3 origin, float amplitude)
    {
        // algorithm from 'How to generate equidistributed points on the surface of a sphere' by Markus Deserno
        // we assume r = 1, i.e. the unit sphere
        float r = 1.0f;
        for (int i = 0; i < rayCount; i++)
        {
            float z = (2 * r / (float)rayCount) * (Random.value * rayCount) - r;
            float phi = (2 * Mathf.PI / rayCount) * (Random.value * rayCount);
            float x = Mathf.Sqrt(1 - z*z) * Mathf.Cos(phi);
            float y = Mathf.Sqrt(1 - z*z) * Mathf.Sin(phi);

            // initialize ray with calculated direction
            Vector3 direction = new Vector3(x, y, z);

            CastRay(origin, direction, amplitude, 0.0f, 0);
        }
    }

    private void CastRay(Vector3 origin, Vector3 direction, float amplitude, float distanceTravelled, int depth)
    {
        // (TODO: find a ray range reduction factor that is based on distance traveled)
        float maxRayRange = rayRange - distanceTravelled;
        if (maxRayRange <= 0.0f) return;

        RaycastHit hit;

        if (Physics.Raycast(origin, direction, out hit, maxRayRange))
        {
            if (visualize) visualization.VisualizeRay(true, origin, hit.point, amplitude, depth);

            if (!HandleListenerHit(hit, depth, amplitude, distanceTravelled + hit.distance))
                if (depth < 5) ReflectRay(hit, ++depth, amplitude, distanceTravelled + hit.distance);
        }
        else
        {
            if (visualize) visualization.VisualizeRay(false, origin, direction * maxRayRange, amplitude, depth);
        }
    }

    private bool HandleListenerHit(RaycastHit hit, int depth, float amplitude, float distanceTravelled)
    {
        if (hit.collider != null && hit.collider.gameObject == listener)
        {
            Vector3 impactPoint = hit.point;

            // calculate the front-back angle
            Vector3 referenceDirection = listener.transform.forward;
            float frontBackAngle = Vector3.Angle(impactPoint - listener.transform.position, referenceDirection);

            // calculate the left-right angle
            Vector3 leftRightDirection = listener.transform.right;
            float leftRightAngle = Vector3.Angle(impactPoint - listener.transform.position, leftRightDirection);

            // create a new RaycastResult
            RaycastResult result = new RaycastResult
            {
                amplitude = amplitude,
                panInformation = leftRightAngle,
                frontBackInformation = frontBackAngle,
                distanceTravelled = distanceTravelled
            };

            if (groupRays)
            {
                // package data to deliver to JUCE plugin
                raycastResults.Add(result);
            }
            else
            {
                // alternative method: send each raycast result as soon as it is calculated
                if (audioManager != null)
                {
                    audioManager.ApplyRaycastResult(result);
                }
                else
                {
                    Debug.Log("AudioManager not selected!");
                }
            }
            return true;
        }
        return false;
    }

    private void ReflectRay(RaycastHit hit, int depth, float amplitude, float distanceTravelled)
    {
        Vector3 newDirection = Vector3.Reflect(hit.point - transform.position, hit.normal);
        
        // reduce amplitude for every reflection
        // TODO: make the reduction factor dependent on the reflection angle
        amplitude *= 0.7f;

        CastRay(hit.point, newDirection, amplitude, distanceTravelled, depth);
    }

}
