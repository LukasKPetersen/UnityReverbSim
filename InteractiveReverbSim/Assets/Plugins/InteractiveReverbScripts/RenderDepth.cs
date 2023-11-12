using System.Collections;
using System.Collections.Generic;
using UnityEngine;

/* DEPRECATED */
class RenderDepth : MonoBehaviour
{
    // public RenderTexture gBufferTexture;
    // public Camera gBufferCamera;
    // public AudioSourceTracker tracker;
    
    // void Start()
    // {
    //     // gBufferTexture = new RenderTexture(Screen.width, Screen.height, 0, RenderTextureFormat.Depth);
    //     // gBufferTexture.Create();

    //     gBufferCamera = Camera.main;
    //     gBufferCamera.depthTextureMode = DepthTextureMode.Depth;


    //     // if (gBufferTexture == null)
    //     //     gBufferTexture = new RenderTexture(Screen.width, Screen.height, 0, RenderTextureFormat.ARGB32);
        
    //     // gBufferTexture.depth = 24;
    //     // gBufferTexture.antiAliasing = 1;
    // }

    // void Update()
    // {
    //     gBufferCamera.targetTexture = gBufferTexture;
    //     gBufferCamera.Render();

    //     // Graphics.Blit(null, gBufferTexture, gBufferCamera.depthTexture, new Vector4(1, 1, 0, 0));

    //     Texture2D gBufferData = new Texture2D(gBufferTexture.width, gBufferTexture.height);



    //     RenderTexture.active = gBufferTexture;
    //     gBufferData.ReadPixels(new Rect(0, 0, gBufferTexture.width, gBufferTexture.height), 0, 0);
    //     gBufferData.Apply();
    //     RenderTexture.active = null;



    //     Vector3 loc = tracker.getAudioSourceScreenPos();
    //     Color pixelColor = gBufferData.GetPixel((int) loc.x, (int) loc.y);
    //     // Debug.Log("Color is: " + pixelColor);

    //     float linearDepth = pixelColor.linear.r;
    //     float grayScale = pixelColor.grayscale;

    //     Debug.Log("linearDepth: " + linearDepth + ", grayScale: " + grayScale);




    //     // float nearClipPlane = gBufferCamera.nearClipPlane;
    //     // float farClipPlane = gBufferCamera.farClipPlane;
    //     // float depth = nearClipPlane + linearDepth * (farClipPlane - nearClipPlane);

    //     // Debug.Log("depth: " + depth + ", depthColor: " + pixelColor + ", linearDepth: " + linearDepth + ", nearClipPlane: " + nearClipPlane + ", farClipPlane: " + farClipPlane);

    //     gBufferCamera.targetTexture = null;

        

    // }
}