using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class DiffusionRayHandler
{
    public bool visualize = true;
    
    private float maxRayRange = 40.0f;
    private float longestDistance = 0.0f;
    private int numListenerHits = 0;
    private float averageDistance = 0.0f;

    // we return 1 if the ray was obstructed and zero if the ray had direct contact with the listener
    public int CastRay(Vector3 origin, Vector3 direction, float distanceTravelled, int numReflections)
    {
        // (TODO: find a ray range reduction factor that is based on distance traveled)
        float rayRange = maxRayRange - distanceTravelled;
        if (rayRange <= 0.0f) return 0;

        RaycastHit hit;
        if (Physics.Raycast(origin, direction, out hit, rayRange))
        {
            // update the longest distance
            if (hit.distance > longestDistance) longestDistance = hit.distance;

            // handle ray hit
            if (HandleListenerHit(hit, distanceTravelled + hit.distance) == 1)
            {
                // if the ray didn't hit the listener, trace the path from the surface to the listener
                if (numReflections < 1)
                {
                    if (visualize) Debug.DrawLine(origin, hit.point, new Color(1, 0, 0, 0.2f), 0.1f, true);
                    return ReflectRay(hit, distanceTravelled + hit.distance, numReflections);
                }
                else 
                {
                    if (visualize) Debug.DrawLine(origin, hit.point, new Color(1, 0, 0, 0.2f), 0.1f, true);
                    return 1; // ray was obstructed
                }
            }
            else
            {
                // if the ray hit the listener, we don't need to trace the path from the surface to the listener
                numListenerHits++;
                averageDistance += hit.distance;
                if (visualize) Debug.DrawLine(origin, hit.point, new Color(0, 0, 1, 0.2f), 0.1f, true);
                return 0;
            }
        }
        else
        {
            // ray didn't hit anything (we count this as a hit on the listener because we don't want to apply any filter to the sound)
            if (visualize) Debug.DrawRay(origin, direction, new Color(1, 1, 1, 0.2f), 0.1f, true);
            return 0;
        }
    }

    private int HandleListenerHit(RaycastHit hit, float distanceTravelled)
    {
        if (hit.collider != null && hit.collider.gameObject == GameObject.Find("Listener"))
        {
            /*Vector3 impactPoint = hit.point;

            // calculate the front-back angle
            Vector3 referenceDirection = GameObject.Find("Listener").transform.forward;
            float frontBackAngle = Vector3.Angle(impactPoint - GameObject.Find("Listener").transform.position, referenceDirection);

            // calculate the left-right angle
            Vector3 leftRightDirection = GameObject.Find("Listener").transform.right;
            float leftRightAngle = Vector3.Angle(impactPoint - GameObject.Find("Listener").transform.position, leftRightDirection);

            // create a new RaycastResult
            RaycastResult result = new RaycastResult
            {
                panInformation = leftRightAngle,
                frontBackInformation = frontBackAngle,
                distanceTravelled = distanceTravelled
            };

            // alternative method: send each raycast result as soon as it is calculated
            if (audioManager != null)
            {
                audioManager.ApplyRaycastResult(result);
            }
            else
            {
                Debug.Log("AudioManager not selected!");
            }*/

            // if the ray was not obstructed, we return 0
            return 0;
        }
        // ray was obstructed
        return 1;
    }

    private int ReflectRay(RaycastHit hit, float distanceTravelled, int numReflections)
    {
        // we trace the path from the hit point to the listener
        Vector3 listenerPosition = GameObject.Find("Listener").transform.position;
        Vector3 newDirection = listenerPosition - hit.point;
        return CastRay(hit.point, newDirection, distanceTravelled, ++numReflections);
    }

    public float GetLongestDistance()
    {
        return longestDistance;
    }

    public void ResetLongestDistance()
    {
        longestDistance = 0.0f;
    }

    public float GetAverageDistance()
    {
        if (numListenerHits == 0) 
            return 0.0f;
        else 
            return averageDistance / numListenerHits;
    }

    public void ResetAverageDistance()
    {
        averageDistance = 0.0f;
        numListenerHits = 0;
    }
}