using System.Collections;
using System.Collections.Generic;
using UnityEngine;

class MaterialAudioAttributes : MonoBehaviour
{
    public float absorptionCoefficient;
    public float scatteringCoefficient;
    public float diffractionCoefficient;
    public float transmissionCoefficient;
    public float filterCoefficient;

    public void Start()
    {
        absorptionCoefficient = 0.95f;
        scatteringCoefficient = 0.2f;
        diffractionCoefficient = 0.2f;
        transmissionCoefficient = 0.2f;
        filterCoefficient = 0.2f;
    }
}