using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class SphericalRaycast
{
    private DiffusionRayHandler rayHandler = new DiffusionRayHandler();
    private int rayCount = 32;

    public float CastSphericalRays(Vector3 origin, RaycastType raycastType)
    {
        // Choose what raycast algorithm to use
        switch (raycastType)
        {
            case RaycastType.RegularEquidistributed:
                return RegularEquidistributedSphericalPlacement(origin);
            case RaycastType.RandomEquidistributed:
                return RandomEquidistributedSphericalPlacement(origin);
            case RaycastType.Random:
                return RandomSphericalPlacement(origin);
            default:
                Debug.LogError("RaycastType not found!");
                return 0.0f;
        }
    }

    public float RegularEquidistributedSphericalPlacement(Vector3 origin)
    {
        float numListenerHits = 0.0f;

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

                numListenerHits += rayHandler.CastRay(origin, direction, 0.0f, 0);
            }
        }
        return numListenerHits / rayCount;
    }

    private float RandomEquidistributedSphericalPlacement(Vector3 origin)
    {
        float numListenerHits = 0.0f;

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

            numListenerHits += rayHandler.CastRay(origin, direction, 0.0f, 0);
        }
        return numListenerHits / rayCount;
    }

    public float RandomSphericalPlacement(Vector3 origin)
    {
        float numListenerHits = 0.0f;

        for (int i = 0; i < rayCount; i++)
        {
            // initialize ray with random direction
            Vector3 direction = Random.onUnitSphere;

            numListenerHits += rayHandler.CastRay(origin, direction, 0.0f, 0);
        }
        return numListenerHits / rayCount;
    }

}