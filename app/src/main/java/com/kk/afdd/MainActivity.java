package com.kk.afdd;

import android.Manifest;
import android.graphics.Matrix;
import android.graphics.SurfaceTexture;
import android.os.Bundle;
import android.view.Gravity;
import android.view.Surface;
import android.view.TextureView;
import android.widget.FrameLayout;

import androidx.annotation.NonNull;

import com.google.gson.Gson;
import com.google.gson.reflect.TypeToken;
import com.tbruyelle.rxpermissions3.RxPermissions;

import java.io.File;
import java.lang.reflect.Type;
import java.util.List;

import io.reactivex.Observable;
import io.reactivex.ObservableOnSubscribe;
import timber.log.Timber;

public class MainActivity extends BaseActivity implements TextureView.SurfaceTextureListener, FaceListener {

    static {
        System.loadLibrary("ndk_camera");
    }

    native void openCamera(int width, int height, int rotation, int fps, boolean mirror, String path);

    native void closeCamera();

    native void setSurface(Surface surface);

    native void setFaceListener(FaceListener faceListener);

    NDKPreviewView mPreviewView;
    FaceOverlay mFaceOverlay;
    private Gson mGson;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        mPreviewView = findViewById(R.id.preview_view);
        mPreviewView.setSurfaceTextureListener(this);
        if (mPreviewView.isAvailable()) {
            onSurfaceTextureAvailable(mPreviewView.getSurfaceTexture(),
                    mPreviewView.getWidth(),
                    mPreviewView.getHeight());
        }

        mFaceOverlay = findViewById(R.id.face_overlay);
        mGson = new Gson();
    }

    @Override
    protected void onDestroy() {
        closeCamera();
        super.onDestroy();
    }

    private boolean mPreviewStarted = false;

    @Override
    public void onSurfaceTextureAvailable(@NonNull SurfaceTexture surface, int width, int height) {
        Timber.d("surface width = %d, height = %d", width, height);
        resizeTextureView(width);
        final RxPermissions rxPermissions = new RxPermissions(this);
        rxPermissions.request(Manifest.permission.CAMERA,
                Manifest.permission.WRITE_EXTERNAL_STORAGE).subscribe(granted -> {
            if (granted) {
                if (mPreviewStarted) {
                    return;
                }
                mPreviewStarted = true;
                RxRequestWrapper.with(Observable.create((ObservableOnSubscribe<BaseResponse<String>>) emitter -> {
                    File moduleFolder = new File(MainActivity.this.getFilesDir(), "modules");
                    if (!moduleFolder.exists()) {
                        FileUtils.copyAssetsToDst(MainActivity.this,
                                "",
                                moduleFolder.getAbsolutePath());
                    }
                    BaseResponse<String> response = new BaseResponse<>();
                    response.data = moduleFolder.getAbsolutePath();
                    emitter.onNext(response);
                    emitter.onComplete();
                })).observeOnIoThread()
                        .observeOnMainThread()
                        .callback(response -> {
                            String path = response.data + File.separator +
                                    "lbpcascades" + File.separator +
                                    "lbpcascade_frontalface.xml";
                            Timber.d("module path:%s", path);
                            setSurface(new Surface(surface));
                            setFaceListener(this);
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

    @SuppressWarnings("ConstantConditions")
    private void resizeTextureView(int textureWidth) {
        int rotation = Config.ROTATION / 90;
        int newHeight;
        if (Surface.ROTATION_90 == rotation || Surface.ROTATION_270 == rotation) {
            newHeight = (textureWidth * Config.PREVIEW_HEIGHT) / Config.PREVIEW_WIDTH;
        } else {
            newHeight = textureWidth * Config.PREVIEW_WIDTH / Config.PREVIEW_HEIGHT;
        }
        Timber.d("resized surface width = %d, height = %d", textureWidth, newHeight);
        FrameLayout.LayoutParams lp = new FrameLayout.LayoutParams(textureWidth, newHeight, Gravity.CENTER);
        mPreviewView.setLayoutParams(lp);
        mFaceOverlay.setLayoutParams(lp);
        configureTransform(textureWidth, newHeight);
    }

    /**
     * configureTransform()
     * Courtesy to https://github.com/google/cameraview/blob/master/library/src/main/api14/com/google/android/cameraview/TextureViewPreview.java#L108
     *
     * @param width  TextureView width
     * @param height is TextureView height
     */
    void configureTransform(int width, int height) {
        int mDisplayOrientation = getWindowManager().getDefaultDisplay().getRotation() * 90;
        Matrix matrix = new Matrix();
        if (mDisplayOrientation % 180 == 90) {
            //final int width = getWidth();
            //final int height = getHeight();
            // Rotate the camera preview when the screen is landscape.
            matrix.setPolyToPoly(
                    new float[]{
                            0.f, 0.f, // top left
                            width, 0.f, // top right
                            0.f, height, // bottom left
                            width, height, // bottom right
                    }, 0,
                    mDisplayOrientation == 90 ?
                            // Clockwise
                            new float[]{
                                    0.f, height, // top left
                                    0.f, 0.f,    // top right
                                    width, height, // bottom left
                                    width, 0.f, // bottom right
                            } : // mDisplayOrientation == 270
                            // Counter-clockwise
                            new float[]{
                                    width, 0.f, // top left
                                    width, height, // top right
                                    0.f, 0.f, // bottom left
                                    0.f, height, // bottom right
                            }, 0,
                    4);
        } else if (mDisplayOrientation == 180) {
            matrix.postRotate(180, width / 2.f, height / 2.f);
        }
        mPreviewView.setTransform(matrix);
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

    private static Type sFaceInfoType;
    private final Object lock = new Object();

    @Override
    public void onFaceDetected(String faceInfoJson) {
        synchronized (lock) {
            if (null == sFaceInfoType) {
                sFaceInfoType = new TypeToken<List<FaceInfo>>() {
                }.getType();
            }
            List<FaceInfo> faceInfoList = mGson.fromJson(faceInfoJson, sFaceInfoType);
            mFaceOverlay.setFaceInfoList(faceInfoList);
        }
    }
}