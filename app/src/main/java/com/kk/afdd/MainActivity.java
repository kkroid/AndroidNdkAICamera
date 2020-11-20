package com.kk.afdd;

import android.Manifest;
import android.app.Dialog;
import android.graphics.Bitmap;
import android.graphics.Matrix;
import android.graphics.Rect;
import android.graphics.SurfaceTexture;
import android.os.Bundle;
import android.view.Gravity;
import android.view.Surface;
import android.view.TextureView;
import android.widget.Button;
import android.widget.EditText;
import android.widget.FrameLayout;
import android.widget.ImageView;

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

    native float calculateSimilar(float[] feature1, float[] feature2);

    private static Type sFaceInfoType;
    private NDKPreviewView mPreviewView;
    private FaceOverlay mFaceOverlay;
    private Gson mGson;
    private boolean mPreviewStarted = false;
    private final Object lock = new Object();
    private FaceInfo mCurrentFaceInfo;
    private List<User> mUserList;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        mGson = new Gson();
        mPreviewView = findViewById(R.id.preview_view);
        mPreviewView.setSurfaceTextureListener(this);
        if (mPreviewView.isAvailable()) {
            onSurfaceTextureAvailable(mPreviewView.getSurfaceTexture(),
                    mPreviewView.getWidth(),
                    mPreviewView.getHeight());
        }

        mFaceOverlay = findViewById(R.id.face_overlay);
        ImageView addButton = findViewById(R.id.register);
        addButton.setOnClickListener(v -> {
            if (null == mCurrentFaceInfo
                    || null == mCurrentFaceInfo.feature
                    || mCurrentFaceInfo.feature.isEmpty()) {
                Timber.i("No face found");
                return;
            }
            FaceInfo registerFace = mCurrentFaceInfo;
            Rect rect = new Rect((int) registerFace.x1,
                    (int) registerFace.y1,
                    (int) registerFace.x2,
                    (int) registerFace.y2);
            Bitmap bitmap = mPreviewView.getFaceBitmap(rect);
            Dialog dialog = new Dialog(MainActivity.this, android.R.style.Theme_DeviceDefault_Dialog);
            dialog.setContentView(R.layout.item_register);
            ImageView faceView = dialog.findViewById(R.id.face);
            faceView.setImageBitmap(bitmap);
            EditText nameView = dialog.findViewById(R.id.name);
            Button okBtn = dialog.findViewById(R.id.ok);
            okBtn.setOnClickListener(v1 -> {
                String name = nameView.getText().toString();
                Timber.d("Name:%s", name);
                float[] featureArray = FeatureUtil.getPrimitiveArray(registerFace.feature);
                String feature = FeatureUtil.featureToString(featureArray);
                User user = new User();
                user.feature = feature;
                user.name = name;
                App.getInstance().boxStore.boxFor(User.class).put(user);
                mUserList = App.getInstance().boxStore.boxFor(User.class).getAll();
                dialog.dismiss();
            });
            dialog.show();
        });
    }

    @SuppressWarnings("ResultOfMethodCallIgnored")
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
                    mUserList = App.getInstance().boxStore.boxFor(User.class).getAll();
                    Timber.i("%d user loaded", mUserList.size());
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

    @Override
    protected void onDestroy() {
        closeCamera();
        super.onDestroy();
    }

    @Override
    public void onFaceDetected(String faceInfoJson) {
        synchronized (lock) {
            if (null == sFaceInfoType) {
                sFaceInfoType = new TypeToken<List<FaceInfo>>() {
                }.getType();
            }
            // String ff = faceInfoJson;
            // int maxLogSize = 1000;
            // for(int i = 0; i <= ff.length() / maxLogSize; i++) {
            //     int start = i * maxLogSize;
            //     int end = (i+1) * maxLogSize;
            //     end = Math.min(end, ff.length());
            //     Timber.v(ff.substring(start, end));
            // }
            List<FaceInfo> faceInfoList = mGson.fromJson(faceInfoJson, sFaceInfoType);
            if (faceInfoList.size() > 0) {
                for (FaceInfo faceInfo : faceInfoList) {
                    if (null == faceInfo.feature || faceInfo.feature.isEmpty()) {
                        continue;
                    }
                    mCurrentFaceInfo = faceInfoList.get(0);
                    long start = System.currentTimeMillis();
                    for (User user : mUserList) {
                        float score = calculateSimilar(FeatureUtil.readFeature(user.feature),
                                FeatureUtil.getPrimitiveArray(faceInfo.feature));
                        if (score > 0.6f) {
                            faceInfo.score = score;
                            faceInfo.name = user.name;
                            Timber.d("calculateSimilar recognized cost %dms, %s, score:%f",
                                    (System.currentTimeMillis() - start),
                                    user.name,
                                    score);
                            break;
                        }
                    }
                }
            }
            mFaceOverlay.setFaceInfoList(faceInfoList);
        }
    }
}