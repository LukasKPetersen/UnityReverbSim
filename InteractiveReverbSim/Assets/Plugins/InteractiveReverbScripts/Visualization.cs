using System.Collections.Generic;
using UnityEngine;

public class Visualization
{
    public float rayDuration = 0.1f;

    public void VisualizeRay(bool wasHit, Vector3 a, Vector3 b, float amplitude, int depth)
    {
        if (wasHit)
        {
            // draw a colored ray if something was hit
            Debug.DrawLine(a, b, GetRayColor(depth, amplitude), rayDuration, true);
        }
        else
        {
            // draw a white ray if nothing was hit
            Debug.DrawRay(a, b, new Color(1, 1, 1, amplitude), rayDuration, true);
        }
    }
    
    private Color GetRayColor(int depth, float amplitude)
    {
        return new Color((float)(depth%10) * 0.1f, (float)(depth%20) * 0.05f, (float)(depth%100) * 0.01f, amplitude * 10.0f);
    }
}