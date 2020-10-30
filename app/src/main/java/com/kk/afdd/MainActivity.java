package com.kk.afdd;

import android.Manifest;
import android.graphics.SurfaceTexture;
import android.os.Bundle;
import android.view.Surface;
import android.view.TextureView;

import androidx.annotation.NonNull;

import com.tbruyelle.rxpermissions3.RxPermissions;

import java.io.File;

import io.reactivex.Observable;
import timber.log.Timber;

public class MainActivity extends BaseActivity implements TextureView.SurfaceTextureListener {

    static {
        System.loadLibrary("ndk_camera");
    }

    native void openCamera(int width, int height, int rotation, int fps, boolean mirror, String path);

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
        mFaceOverlay.setMirror(Config.MIRROR);
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
                RxRequestWrapper.with(Observable.create(emitter -> {
                    File moduleFolder = new File(MainActivity.this.getFilesDir(), "modules");
                    if (!moduleFolder.exists()) {
                        FileUtils.copyAssetsToDst(MainActivity.this,
                                "",
                                MainActivity.this.getFilesDir().getAbsolutePath());
                    }
                    emitter.onNext(new BaseResponse<>());
                    emitter.onComplete();
                })).observeOnIoThread()
                        .observeOnMainThread()
                        .callback(response -> {
                            String path = MainActivity.this.getFilesDir().getAbsolutePath() +
                                    File.separator + "haarcascades" + File.separator + "haarcascade_frontalface_alt" +
                                    ".xml";
                            Timber.d("module path:" + path);
                            setSurface(new Surface(surface));
                            setFaceOverlay(mFaceOverlay);
                            openCamera(Config.PREVIEW_WIDTH,
                                    Config.PREVIEW_HEIGHT,
                                    Config.ROTATION,
                                    Config.FPS,
                                    Config.MIRROR,
                                    path);
                        });
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