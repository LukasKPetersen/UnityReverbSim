using System.Collections;
using System.Collections.Generic;
using UnityEngine;

class SourceToListenerLink : MonoBehaviour
{
    public AudioSource audioSource;
    public GameObject listener;
    public AudioManager audioManager;
    public Visualization visualization = new Visualization();

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
        // Calculate the distance between the source and the listener
        float distance = Vector3.Distance(transform.position, listener.transform.position);

        // Get all the objects between the source and the listener
        RaycastHit[] hits = Physics.RaycastAll(transform.position, listener.transform.position - transform.position, distance);

        float transmissionCoefficient = 1.0f;
        float filterCoefficient = 4e3f;
        float frontBackAngle = 0.0f;
        float leftRightAngle = 90.0f;

        Vector3 connectionPoint = transform.position;
        // Loop through all the objects
        foreach (RaycastHit hit in hits)
        {
            // Check if the object is the listener
            if (hit.collider.gameObject == listener)
            {
                Vector3 impactPoint = hit.point;

                // calculate the front-back angle
                Vector3 referenceDirection = listener.transform.forward;
                frontBackAngle = Vector3.Angle(impactPoint - listener.transform.position, referenceDirection);

                // calculate the left-right angle
                Vector3 leftRightDirection = listener.transform.right;
                leftRightAngle = Vector3.Angle(impactPoint - listener.transform.position, leftRightDirection);

                // visualize the ray
                if (visualization != null)
                {
                    visualization.VisualizeRay(true, transform.position, hit.point, 1.0f, 0);
                }

                break;
            }

            if (hit.collider.gameObject.GetComponent<MaterialAudioAttributes>() != null)
            {
                transmissionCoefficient *= 1.0f - hit.collider.gameObject.GetComponent<MaterialAudioAttributes>().transmissionCoefficient;
                filterCoefficient *= hit.collider.gameObject.GetComponent<MaterialAudioAttributes>().filterCoefficient;

                // visualize the ray
                if (visualization != null)
                {
                    visualization.VisualizeRay(true, connectionPoint, hit.point, transmissionCoefficient, hits.Length - 1);
                }

                connectionPoint = hit.point;
            }
        }

        // create a new RaycastResult
        RaycastResult link = new RaycastResult
        {
            soundReduction = transmissionCoefficient,
            panInformation = leftRightAngle,
            frontBackInformation = frontBackAngle,
            distanceTravelled = distance,
            filterCoefficient = filterCoefficient
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
    }
}