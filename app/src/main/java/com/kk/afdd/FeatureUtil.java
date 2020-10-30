package com.kk.afdd;

import java.util.Arrays;

public class FeatureUtil {
    public static String featureToString(float[] feature) {
        if (feature == null) {
            return null;
        }
        return Arrays.toString(feature)
                .trim()
                .replace("[", "")
                .replace("]", "");
    }

    public static float[] readFeature(String featureData) {
        String[] storeFeatureString = featureData.split(",");
        float[] storeFeatureFloat = new float[storeFeatureString.length];
        for (int i = 0; i < storeFeatureString.length; i++) {
            String f = storeFeatureString[i];
            storeFeatureFloat[i] = Float.parseFloat(f);
        }
        return storeFeatureFloat;
    }
}
