using System.Collections;
using System.Collections.Generic;
using UnityEngine;

class SourceToListenerLink : MonoBehaviour
{
    public AudioSource audioSource;
    public GameObject listener;
    public AudioManager audioManager;
    public Visualization visualization = new Visualization();

    public int numAnalysisRays = 32;
    public bool debug = false;
    public bool boostEffect = false;
    float factor = 1.0f;

    float leftRightAngle = 90.0f;
    float frontBackAngle = 0.0f;
    float transmissionCoefficient = 1.0f;
    float[] filterCoefficient = { 5e3f, 5e3f };

    void Start()
    {
        // Initialize the audioSource
        audioSource = GetComponent<AudioSource>();
        if (audioSource == null)
            audioSource = gameObject.AddComponent<AudioSource>();

        // Initialize the listener GameObject
        listener = GameObject.Find("Listener");
        if (listener == null)
        {
            Debug.LogError("Listener not found! Make sure it has the correct name in the script.");
        }
    }

    void Update()
    {   
        if (boostEffect) factor = 7.0f;
        else factor = 1.0f;

        // Calculate the distance between the source and the listener
        float distance = Vector3.Distance(transform.position, listener.transform.position);

        // Get all the objects between the source and the listener
        Vector3 direction = listener.transform.position - transform.position;
        RaycastHit[] hits = Physics.RaycastAll(transform.position, direction, distance);

        // TODO: only call when player or soundsource moves
        handleLinkRay(hits);
        
        // if the direct line of sight is blocked, shoot analysis rays
        if (hits.Length > 1) shootAnalysisRays(direction, distance);

        if (debug) Debug.Log("filterCoef Left: " + filterCoefficient[0]);
        if (debug) Debug.Log("filterCoef Right: " + filterCoefficient[1]);
        if (debug) Debug.Log("transmissionCoef: " + transmissionCoefficient);

        // create a new RaycastResult
        RaycastResult link = new RaycastResult
        {
            soundReduction = transmissionCoefficient,
            panInformation = leftRightAngle,
            frontBackInformation = frontBackAngle,
            distanceTravelled = distance,
            filterCoefficients = filterCoefficient
        };

        
        // Send the link to the audio source
        if (audioManager != null)
        {
            audioManager.ApplyRaycastResult(link);
        }
        else
        {
            Debug.Log("AudioManager not selected!");
        }

        // reset the transmission coefficient
        transmissionCoefficient = 1.0f;

        // reset the filter coefficients
        filterCoefficient[0] = 5e3f;
        filterCoefficient[1] = 5e3f;
    }

    void shootAnalysisRays(Vector3 linkDirection, float distance)
    {
        // shoot rays in a circle around the listener
        for (int ray = 1; ray < numAnalysisRays; ray++) // TODO: parallelize this
        {
            // TODO: NOTE: right now I'm handling the central bottom ray which is not relevant

            // calculate the direction of the ray (rotating around the axis of the S2L link)
            float angle = ray * 360.0f / numAnalysisRays;
            Vector3 direction = Quaternion.AngleAxis(angle, linkDirection) * (linkDirection + new Vector3(1.0f, 1.0f, 1.1f));

            // shoot the ray
            RaycastHit hit;
            if (Physics.Raycast(transform.position, direction, out hit, distance))
            {
                // handle rays that gets obstructed
                if (hit.collider.gameObject.GetComponent<MaterialAudioAttributes>() != null)
                {
                    if (ray > numAnalysisRays / 2)
                    {
                        // handle left side
                        filterCoefficient[0] *= 1.0f - factor / (numAnalysisRays/2) * hit.collider.gameObject.GetComponent<MaterialAudioAttributes>().filterCoefficient;
                    }
                    else
                    {
                        // handle right side
                        filterCoefficient[1] *= 1.0f - factor / (numAnalysisRays/2) * hit.collider.gameObject.GetComponent<MaterialAudioAttributes>().filterCoefficient;
                    }
                }

                // visualize the ray
                if (visualization != null)
                {
                    visualization.VisualizeRay(true, transform.position, hit.point, 1.0f, 3);
                }
            }
            else
            {
                // handle rays that doesn't get obstructed
                if (ray > numAnalysisRays / 2)
                {
                    // handle left side
                    filterCoefficient[0] *= 1.0f + factor * 0.2f / (numAnalysisRays/2);
                }
                else
                {
                    // handle right side
                    filterCoefficient[1] *= 1.0f + factor * 0.2f / (numAnalysisRays/2);
                }

                // visualize the ray
                if (visualization != null)
                {
                    visualization.VisualizeRay(false, transform.position, hit.point, 1.0f, 3);
                }
            }
        }
    }

    void handleLinkRay(RaycastHit[] hits)
    {
        Vector3 connectionPoint = transform.position;

        // Loop through all the objects between the source and the listener
        foreach (RaycastHit hit in hits)
        {
            // if the hit was an obstruction
            if (hit.collider.gameObject.GetComponent<MaterialAudioAttributes>() != null)
            {
                // TODO: introduce an S-curve function
                transmissionCoefficient *= 1.0f - hit.collider.gameObject.GetComponent<MaterialAudioAttributes>().transmissionCoefficient;
                for (int i = 0; i < filterCoefficient.Length; i++)
                {
                    filterCoefficient[i] *= 1.0f - factor * 0.5f * hit.collider.gameObject.GetComponent<MaterialAudioAttributes>().filterCoefficient; // TODO: a more sophisticated function
                }
            }

            // if the hit was the listener
            if (hit.collider.gameObject == listener)
            {
                Vector3 impactPoint = hit.point;

                // calculate the left-right angle
                Vector3 leftRightDirection = listener.transform.right;
                leftRightAngle = Vector3.Angle(impactPoint - listener.transform.position, leftRightDirection);

                // calculate the front-back angle
                Vector3 referenceDirection = listener.transform.forward;
                frontBackAngle = Vector3.Angle(impactPoint - listener.transform.position, referenceDirection);
            }

            // visualize the ray
            if (visualization != null)
            {
                visualization.VisualizeRay(true, connectionPoint, hit.point, transmissionCoefficient, hits.Length - 1);
            }

            connectionPoint = hit.point;
        }
    }
}