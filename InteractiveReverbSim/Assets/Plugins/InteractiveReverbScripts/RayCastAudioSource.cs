using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class RayCastAudioSource : MonoBehaviour
{
    public AudioSource audioSource;
    public GameObject listener;

    public AudioManager audioManager;

    // Rays
    public int rayCount = 600;
    public float rayRange = 10.0f;
    public float rayDuration = 0.1f;
    public bool debug = true;

    // Type of raycast
    public enum RaycastType 
    { 
        Regular, 
        Random 
    };
    public RaycastType raycastType = RaycastType.Random;

    // The number of frequency bands to analyze
    public int numBands = 64;

    // Array to store the amplitude values for each frequency band
    private float[] spectrumData;

    private List<RaycastResult> raycastResults = new List<RaycastResult>();


    // Start is called before the first frame update
    void Start()
    {
        audioSource = GetComponent<AudioSource>();
        if (audioSource == null)
            audioSource = gameObject.AddComponent<AudioSource>();

        // Find and initialize the ImpactSphere GameObject
        listener = GameObject.Find("Listener"); // Replace with the actual name of your sphere GameObject
        if (listener == null)
        {
            Debug.LogError("Listener not found! Make sure it has the correct name.");
        }

        audioSource.volume = 1.0f;
        audioSource.loop = true;
        audioSource.playOnAwake = true;

        // Initialize the spectrumData array with the number of bands
        spectrumData = new float[numBands];

        // assign the audioManager
        // audioManager = AudioManager.instance;
    }

    // Update is called once per frame
    void Update()
    {
        // Get the amplitude of the audio source
        float amplitude = 0.0f;
        if (audioSource.isPlaying)
        {
            audioSource.GetSpectrumData(spectrumData, 0, FFTWindow.Rectangular); // or perhaps with FFTWindow.BlackmanHarris?
            // Calculate the average amplitude
            for (int i = 0; i < numBands; i++)
            {
                amplitude += spectrumData[i];
            }
            amplitude /= numBands;
        }
        
        CastSpericalRays(amplitude);
    }
    
    private void CastSpericalRays(float amplitude) 
    {
        Vector3 origin = transform.position;

        switch (raycastType)
        {
            case RaycastType.Regular:
                RegularSphericalPlacement(origin, amplitude);
                break;
            case RaycastType.Random:
                RandomSphericalPlacement(origin, amplitude);
                break;
        }
    }

    private void RegularSphericalPlacement(Vector3 origin, float amplitude)
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

                // cast ray
                RaycastHit hit;
                if (Physics.Raycast(origin, direction, out hit, rayRange))
                {
                    // draw a colored ray if something was hit
                    if (debug) Debug.DrawLine(origin, hit.point, GetRayColor(0, amplitude), rayDuration, true);
                    if (!HandleListenerHit(hit, 0, amplitude))
                        ReflectRay(hit, 1, amplitude);
                } 
                else
                {
                    // draw a white ray if nothing was hit
                    if (debug) Debug.DrawRay(origin, direction * rayRange, new Color(1, 1, 1, amplitude), rayDuration, true);
                }
            }
        }
    }

    private void RandomSphericalPlacement(Vector3 origin, float amplitude)
    {
        for (int i = 0; i < rayCount; i++)
        {
            // initialize ray with random direction
            Vector3 direction = Random.onUnitSphere;

            // cast ray
            RaycastHit hit;
            if (Physics.Raycast(origin, direction, out hit, rayRange))
            {
                // draw a colored ray if something was hit
                if (debug) Debug.DrawLine(origin, hit.point, GetRayColor(0, amplitude), rayDuration, true);
                // if the ray did not hit the listener, reflect it
                if (!HandleListenerHit(hit, 0, amplitude))
                    ReflectRay(hit, 1, amplitude);
            } 
            else
            {
                // draw a white ray if nothing was hit
                if (debug) Debug.DrawRay(origin, direction * rayRange, new Color(1, 1, 1, amplitude), rayDuration, true);
            }
        }
    }

    private bool HandleListenerHit(RaycastHit hit, int depth, float amplitude)
    {
        if (hit.collider != null && hit.collider.gameObject == listener)
        {
            Vector3 impactPoint = hit.point;

            // Calculate the front-back angle
            Vector3 referenceDirection = listener.transform.forward;
            float frontBackAngle = Vector3.Angle(impactPoint - listener.transform.position, referenceDirection);

            // Calculate the left-right angle
            Vector3 leftRightDirection = listener.transform.right;
            float leftRightAngle = Vector3.Angle(impactPoint - listener.transform.position, leftRightDirection);

            // create a new RaycastResult
            RaycastResult result = new RaycastResult
            {
                amplitude = amplitude,
                panInformation = leftRightAngle,
                frontBackInformation = frontBackAngle
            };

            // deliver data to JUCE plugin
            if (audioManager != null)
            {
                audioManager.ApplyRaycastResult(result);
            }
            else
            {
                Debug.Log("AudioManager not selected!");
            }

            return true;
        }
        return false;
    }

    private void ReflectRay(RaycastHit hit, int depth, float amplitude)
    {

        Vector3 inDirection = hit.point - transform.position;
        Vector3 newDirection = Vector3.Reflect(inDirection, hit.normal);
        
        // reduce amplitude for every reflection
        amplitude *= 0.7f; // TODO: make the reduction factor dependent on the reflection angle


        RaycastHit newHit;
        if (Physics.Raycast(hit.point, newDirection, out newHit, rayRange-(rayRange * 0.2f * depth))) // reduce ray range for every reflection (TODO: find a ray range reduction factor that is based on distance traveled)
        {
            // draw a colored ray if something was hit
            if (debug) Debug.DrawLine(hit.point, newHit.point, GetRayColor(depth, amplitude), rayDuration, true);
            // if the ray did not hit the listener, reflect it
            if (!HandleListenerHit(hit, depth, amplitude))
                if (depth < 5) ReflectRay(newHit, ++depth, amplitude);
        } 
        else
        {
            // draw a white ray if nothing was hit
            if (debug) Debug.DrawRay(hit.point, newDirection * rayRange, new Color(1, 1, 1, amplitude), rayDuration, true);
        }
    }
    
    private Color GetRayColor(int depth, float amplitude)
    {
        return new Color((float)(depth%10) * 0.1f, (float)(depth%20) * 0.05f, (float)(depth%100) * 0.01f, amplitude);
    }
}
