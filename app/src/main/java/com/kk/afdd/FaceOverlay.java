package com.kk.afdd;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.RectF;
import android.text.TextUtils;
import android.util.AttributeSet;
import android.view.View;

import androidx.annotation.Nullable;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

public class FaceOverlay extends View {

    private int mRatioWidth = 0;
    private int mRatioHeight = 0;
    private final List<FaceInfo> mFaceInfoList = Collections.synchronizedList(new ArrayList<>());
    private final Paint mFacePaint;
    private final Paint mTextPaint;
    private float mScale;
    private boolean mMirror;
    private float mWidth = 0;

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

        mMirror = Config.MIRROR;
    }

    public void setMirror(boolean mirror) {
        mMirror = mirror;
    }

    @Override
    protected void onLayout(boolean changed, int left, int top, int right, int bottom) {
        super.onLayout(changed, left, top, right, bottom);
        mWidth = right - left;
        mScale = mWidth / Config.PREVIEW_WIDTH;
    }

    public void setFaceInfoList(List<FaceInfo> faceInfoJson) {
        mFaceInfoList.clear();
        mFaceInfoList.addAll(faceInfoJson);
        postInvalidate();
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

    private String currentFaceName;

    @Override
    public void draw(Canvas canvas) {
        super.draw(canvas);
        // draw faces
        for (FaceInfo faceInfo : mFaceInfoList) {
            RectF rectF = new RectF(faceInfo.x1, faceInfo.y1, faceInfo.x2, faceInfo.y2);
            transform(rectF, mMirror, mScale, mWidth);
            canvas.drawRect(rectF, mFacePaint);
            String name = faceInfo.name;
            if (!TextUtils.isEmpty(name) && !"unknown".equals(name)) {
                currentFaceName = name + " " + faceInfo.score;
                canvas.drawText(currentFaceName,
                        rectF.left + 4,
                        rectF.centerY(),
                        mTextPaint);
            }
        }
    }

    public static void transform(RectF rectF, boolean mirror, float scale, float offset) {
        if (mirror) {
            // 镜像
            float tmp = -rectF.left;
            rectF.left = -rectF.right;
            rectF.right = tmp;
        }
        // 缩放
        rectF.left = rectF.left * scale;
        rectF.top = rectF.top * scale;
        rectF.right = rectF.right * scale;
        rectF.bottom = rectF.bottom * scale;
        if (mirror) {
            // 平移
            rectF.offset(offset, 0);
        }
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
