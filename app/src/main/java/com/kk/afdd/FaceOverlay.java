package com.kk.afdd;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.RectF;
import android.util.AttributeSet;
import android.view.View;

import androidx.annotation.Nullable;

import com.google.gson.Gson;
import com.google.gson.reflect.TypeToken;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.Locale;

public class FaceOverlay extends View {

    private int mRatioWidth = 0;
    private int mRatioHeight = 0;
    private Gson mGson;
    private List<FaceInfo> mFaceInfoList = Collections.synchronizedList(new ArrayList<>());
    private Paint mFacePaint;
    private Paint mTextPaint;
    private float mScale;
    private boolean mMirror;
    private float mWidth = 0;
    private final Object lock = new Object();

    public FaceOverlay(Context context) {
        this(context, null);
    }

    public FaceOverlay(Context context, @Nullable AttributeSet attrs) {
        this(context, attrs, 0);
    }

    public FaceOverlay(Context context, @Nullable AttributeSet attrs, int defStyleAttr) {
        this(context, attrs, defStyleAttr, 0);
    }

    public FaceOverlay(Context context, @Nullable AttributeSet attrs, int defStyleAttr, int defStyleRes) {
        super(context, attrs, defStyleAttr, defStyleRes);
        setBackgroundColor(getResources().getColor(android.R.color.transparent, null));
        mFacePaint = new Paint();
        mFacePaint.setColor(Color.BLUE);
        mFacePaint.setStrokeWidth(8);
        mFacePaint.setStrokeCap(Paint.Cap.BUTT);
        mFacePaint.setStyle(Paint.Style.STROKE);
        mTextPaint = new Paint();
        mTextPaint.setColor(Color.WHITE);
        mTextPaint.setStrokeWidth(2);
        mTextPaint.setStrokeCap(Paint.Cap.BUTT);
        mTextPaint.setStyle(Paint.Style.FILL_AND_STROKE);
        mTextPaint.setTextSize(42);

        mGson = new Gson();
    }

    public void setMirror(boolean mirror) {
        mMirror = mirror;
    }

    @Override
    protected void onLayout(boolean changed, int left, int top, int right, int bottom) {
        super.onLayout(changed, left, top, right, bottom);
        mWidth = right - left;
        mScale = mWidth / mRatioWidth;
    }

    public void setFaceInfoList(String faceInfoJson) {
        synchronized (lock) {
            List<FaceInfo> faceInfoList = mGson.fromJson(faceInfoJson, new TypeToken<List<FaceInfo>>() {
            }.getType());
            mFaceInfoList.clear();
            mFaceInfoList.addAll(faceInfoList);
            invalidate();
        }
    }

    /**
     * Sets the aspect ratio for this view. The size of the view will be measured based on the ratio
     * calculated from the parameters. Note that the actual sizes of parameters don't matter, that
     * is, calling setAspectRatio(2, 3) and setAspectRatio(4, 6) make the same result.
     *
     * @param width  Relative horizontal size
     * @param height Relative vertical size
     */
    public void setAspectRatio(int width, int height) {
        if (width < 0 || height < 0) {
            throw new IllegalArgumentException("Size cannot be negative.");
        }
        mRatioWidth = width;
        mRatioHeight = height;
        requestLayout();
    }

    @Override
    public void draw(Canvas canvas) {
        super.draw(canvas);
        synchronized (lock) {
            // draw faces
            for (FaceInfo faceInfo : mFaceInfoList) {
                RectF rectF = new RectF(faceInfo.x1, faceInfo.y1, faceInfo.x2, faceInfo.y2);
                if (mMirror) {
                    // 镜像
                    mirror(rectF);
                }
                // 缩放
                scale(rectF, mScale);
                if (mMirror) {
                    // 平移
                    offset(rectF, mWidth);
                }
                canvas.drawRect(rectF, mFacePaint);
                if (faceInfo.age > 0) {
                    canvas.drawText(String.format(Locale.getDefault(), "Gender:%s, Age:%d", faceInfo.gender ?
                                    "Female" : "Male",
                            faceInfo.age),
                            rectF.left + 4,
                            rectF.centerY(),
                            mTextPaint);
                }
            }
        }
    }

    private void mirror(RectF rectF) {
        float tmp = -rectF.left;
        rectF.left = -rectF.right;
        rectF.right = tmp;
    }

    private void offset(RectF rectF, float offset) {
        rectF.offset(offset, 0);
    }

    private void scale(RectF rectF, float scale) {
        rectF.left = rectF.left * scale;
        rectF.top = rectF.top * scale;
        rectF.right = rectF.right * scale;
        rectF.bottom = rectF.bottom * scale;
    }

    @Override
    protected void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
        super.onMeasure(widthMeasureSpec, heightMeasureSpec);
        int width = MeasureSpec.getSize(widthMeasureSpec);
        int height = MeasureSpec.getSize(heightMeasureSpec);
        if (0 == mRatioWidth || 0 == mRatioHeight) {
            setMeasuredDimension(width, height);
        } else {
            if (width < height * mRatioWidth / mRatioHeight) {
                setMeasuredDimension(width, width * mRatioHeight / mRatioWidth);
            } else {
                setMeasuredDimension(height * mRatioWidth / mRatioHeight, height);
            }
        }
    }
}
