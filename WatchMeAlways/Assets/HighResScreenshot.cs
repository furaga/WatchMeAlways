using UnityEngine;
using System.Collections;

public class HighResScreenshot : MonoBehaviour
{

    void Start()
    {



    }

    bool takeHiResShot = false;

    void LateUpdate()
    {

        var camera = GetComponent<Camera>();

        takeHiResShot |= Input.GetKeyDown("k");
        if (takeHiResShot)
        {
            myScreenShot();
            takeHiResShot = false;
        }
    }

    public void myScreenShot()
    {

        StartCoroutine(TakeScreenShot());

    }




    IEnumerator TakeScreenShot()
    {

        yield return new WaitForEndOfFrame();


        var width = Screen.width / 2;
        var height = Screen.height / 2;
        var tex = new Texture2D(width, height, TextureFormat.RGB24, false);


        tex.ReadPixels(new Rect(0, 0, width, height), 0, 0);
        tex.Apply();

        {
            byte[] bytes = tex.EncodeToPNG();
            string filename = ScreenShotName(width, height);
            System.IO.File.WriteAllBytes(filename, bytes);
            Debug.Log(string.Format("Took screenshot to: {0}", filename));
        }

    }

    public static string ScreenShotName(int width, int height)
    {
        return string.Format("screen_{1}x{2}_{3}.png",
                             Application.dataPath,
                             width, height,
                             System.DateTime.Now.ToString("yyyy-MM-dd_HH-mm-ss"));
    }

}
/*
{
    int resWidth = 1920;
    int resHeight = 1080;

    private bool takeHiResShot = false;

    public static string ScreenShotName(int width, int height)
    {
        return string.Format("screen_{1}x{2}_{3}.png",
                             Application.dataPath,
                             width, height,
                             System.DateTime.Now.ToString("yyyy-MM-dd_HH-mm-ss"));
    }

    public void TakeHiResShot()
    {
        takeHiResShot = true;
    }

    System.Diagnostics.Stopwatch sw = new System.Diagnostics.Stopwatch();
    long t1, t2, t3, t4, t5;
    int cnt = 0;


    void LateUpdate()
    {

        var camera = GetComponent<Camera>();

        takeHiResShot |= Input.GetKeyDown("k");
        if (takeHiResShot)
        {
            cnt++;
            Debug.Log(cnt);
            if (cnt % 30 == 0)
            {
                Debug.Log(string.Format("t1={0},t2={1},t3={2},t4={3},t5={4}",
                    t1, t2, t3, t4, t5));
            }

            sw.Reset();
            sw.Start();

            RenderTexture rt = new RenderTexture(resWidth, resHeight, 24);

            camera.targetTexture = rt;
            Texture2D frameTexture = new Texture2D(resWidth, resHeight, TextureFormat.RGB24, false);
            //frameTexture.hideFlags = HideFlags.HideAndDontSave;
            //frameTexture.wrapMode = TextureWrapMode.Clamp;
            //frameTexture.filterMode = FilterMode.Trilinear;
            //frameTexture.hideFlags = HideFlags.HideAndDontSave;
            //frameTexture.anisoLevel = 0;
            camera.Render();
            RenderTexture.active = rt;
            t1 += sw.ElapsedMilliseconds;
            sw.Reset();
            sw.Start();

            frameTexture.ReadPixels(new Rect(0, 0, resWidth, resHeight), 0, 0);
            t2 += sw.ElapsedMilliseconds;
            sw.Reset();
            sw.Start();

            camera.targetTexture = null;
            RenderTexture.active = null; // JC: added to avoid errors
            Destroy(rt);
            t3 += sw.ElapsedMilliseconds;
            sw.Reset();
            sw.Start();

            byte[] bytes = frameTexture.EncodeToPNG();
            t4 += sw.ElapsedMilliseconds;
            sw.Reset();
            sw.Start();

            if (cnt % 30 == 0)
            {
                string filename = ScreenShotName(resWidth, resHeight);
                System.IO.File.WriteAllBytes(filename, bytes);
                Debug.Log(string.Format("Took screenshot to: {0}", filename));
                takeHiResShot = false;
                t5 += sw.ElapsedMilliseconds;
            }
        }
    }
}
    */
