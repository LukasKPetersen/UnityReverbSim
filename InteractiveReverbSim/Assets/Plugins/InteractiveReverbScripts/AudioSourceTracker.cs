using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;

public class AudioSourceTracker : MonoBehaviour
{
    public Transform soundSource;
    public Image marker;
    public MoveTracker tracker;
    public bool debug;

    private Camera mainCamera;
    private Vector3 relativePos;

    private void Start()
    {
        mainCamera = Camera.main;
        if (debug) Debug.Log("Pixel width: " + mainCamera.pixelWidth + ", pixel height: " + mainCamera.pixelHeight);
    }

    private void Update()
    {
        if (soundSource != null && mainCamera != null && marker != null && tracker != null)
        {
            // Calculate screen position of the sound source
            relativePos = mainCamera.WorldToScreenPoint(soundSource.position);            

            // Check if the sound source is within the camera's view.
            if (relativePos.x >= 0 && relativePos.x <= mainCamera.pixelWidth && relativePos.y >= 0 && relativePos.y <= mainCamera.pixelHeight && relativePos.z > 0)
            {
                marker.enabled = true;
                tracker.MoveToPosition(relativePos.x, relativePos.y);
            }
            else
            {
                marker.enabled = false;
            }
        }
        else
        {
            if (marker != null)
                marker.enabled = false;
            else
                Debug.Log("Marker was null");
        }
    }

    public Vector3 getAudioSourceScreenPos()
    {
        return relativePos;
    }
}
