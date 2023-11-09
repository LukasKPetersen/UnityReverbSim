using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;

public class MoveTracker : MonoBehaviour
{
    public RectTransform imagePosition;

    private void Start()
    {
        if (imagePosition == null)
        {
            imagePosition = GetComponent<RectTransform>();
        }

        MoveToPosition(0, 0);
    }

    public void MoveToPosition(float x, float y)
    {
        imagePosition.anchoredPosition = new Vector2(x, y);
    }
}
