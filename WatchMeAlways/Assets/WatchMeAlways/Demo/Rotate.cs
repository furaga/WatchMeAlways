using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class Rotate : MonoBehaviour {

	// Use this for initialization
	void Start () {
		
	}
	
	// Update is called once per frame
	void Update () {
        float dt = Time.deltaTime;
        float wx = 360 * dt * 0.5f;
        float wy = 360 * dt * 0.3f;
        float wz = 360 * dt * 0.2f;
        transform.Rotate(new Vector3(wx, wy, wz));
	}
}
