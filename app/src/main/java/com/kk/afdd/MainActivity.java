package com.kk.afdd;

import android.Manifest;
import android.graphics.SurfaceTexture;
import android.os.Bundle;
import android.view.Surface;
import android.view.TextureView;

import androidx.annotation.NonNull;

import com.tbruyelle.rxpermissions3.RxPermissions;

public class MainActivity extends BaseActivity implements TextureView.SurfaceTextureListener {

    static {
        System.loadLibrary("ndk_camera");
    }

    native void openCamera();

    native void closeCamera();

    native void setSurface(Surface surface);

    native void setFaceOverlay(FaceOverlay faceOverlay);

    NDKPreviewView mPreviewView;
    FaceOverlay mFaceOverlay;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        mPreviewView = findViewById(R.id.preview_view);
        mPreviewView.setSurfaceTextureListener(this);
        mPreviewView.setAspectRatio(Config.PREVIEW_WIDTH, Config.PREVIEW_HEIGHT);
        mFaceOverlay = findViewById(R.id.face_overlay);
        mFaceOverlay.setAspectRatio(Config.PREVIEW_WIDTH, Config.PREVIEW_HEIGHT);
        mFaceOverlay.setMirror(true);
    }

    @Override
    protected void onDestroy() {
        closeCamera();
        super.onDestroy();
    }

    @Override
    public void onSurfaceTextureAvailable(@NonNull SurfaceTexture surface, int width, int height) {
        final RxPermissions rxPermissions = new RxPermissions(this);
        rxPermissions.request(Manifest.permission.CAMERA,
                Manifest.permission.WRITE_EXTERNAL_STORAGE).subscribe(granted -> {
            if (granted) {
                setSurface(new Surface(surface));
                setFaceOverlay(mFaceOverlay);
                openCamera();
            }
        });
    }

    @Override
    public void onSurfaceTextureSizeChanged(@NonNull SurfaceTexture surface, int width, int height) {

    }

    @Override
    public boolean onSurfaceTextureDestroyed(@NonNull SurfaceTexture surface) {
        return false;
    }

    @Override
    public void onSurfaceTextureUpdated(@NonNull SurfaceTexture surface) {

    }
}