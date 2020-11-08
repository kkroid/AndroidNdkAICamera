package com.kk.afdd;

import java.io.Serializable;
import java.util.List;

public class FaceInfo implements Serializable {
    public float x1;
    public float y1;
    public float x2;
    public float y2;
    public float score;
    public List<Float> feature;
}
