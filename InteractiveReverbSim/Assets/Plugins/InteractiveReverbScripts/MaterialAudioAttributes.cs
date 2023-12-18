using System.Collections;
using System.Collections.Generic;
using UnityEngine;

class MaterialAudioAttributes : MonoBehaviour
{
    public float absorptionCoefficient = 0.95f;
    public float scatteringCoefficient = 0.2f;
    public float diffractionCoefficient = 0.2f;
    public float transmissionCoefficient = 0.2f;
    public float filterCoefficient = 0.2f;

    public void Start()
    {
        absorptionCoefficient = 0.95f;
        scatteringCoefficient = 0.2f;
        diffractionCoefficient = 0.2f;
        transmissionCoefficient = 0.2f;
        filterCoefficient = 0.2f;
    }
}