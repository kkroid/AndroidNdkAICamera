package com.kk.afdd;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapRegionDecoder;
import android.graphics.Rect;
import android.util.AttributeSet;
import android.view.TextureView;

import androidx.annotation.Nullable;

import java.io.ByteArrayOutputStream;
import java.io.IOException;

public class NDKPreviewView extends TextureView {

    private int mRatioWidth = 0;
    private int mRatioHeight = 0;
    private float mScale;
    private float mWidth = 0;

    public NDKPreviewView(Context context) {
        this(context, null);
    }

    public NDKPreviewView(Context context, @Nullable AttributeSet attrs) {
        this(context, attrs, 0);
    }

    public NDKPreviewView(Context context, @Nullable AttributeSet attrs, int defStyleAttr) {
        this(context, attrs, defStyleAttr, 0);
    }

    public NDKPreviewView(Context context, @Nullable AttributeSet attrs, int defStyleAttr, int defStyleRes) {
        super(context, attrs, defStyleAttr, defStyleRes);
    }

    @Override
    protected void onLayout(boolean changed, int left, int top, int right, int bottom) {
        super.onLayout(changed, left, top, right, bottom);
        mWidth = right - left;
        mScale = mWidth / Config.PREVIEW_WIDTH;
    }

    public Bitmap getFaceBitmap(Rect rect) {
        transform(rect, Config.MIRROR, mScale, (int) mWidth);
        Bitmap bitmap = getBitmap();
        ByteArrayOutputStream stream = new ByteArrayOutputStream();
        bitmap.compress(Bitmap.CompressFormat.JPEG, 100, stream);
        byte[] byteArray = stream.toByteArray();
        bitmap.recycle();
        try {
            BitmapRegionDecoder bitmapRegionDecoder =
                    BitmapRegionDecoder.newInstance(byteArray, 0, byteArray.length, false);
            return bitmapRegionDecoder.decodeRegion(rect, null);
        } catch (IOException e) {
            e.printStackTrace();
        }
        return null;
    }

    private void transform(Rect rect, boolean mirror, float scale, int offset) {
        if (mirror) {
            // 镜像
            int tmp = -rect.left;
            rect.left = -rect.right;
            rect.right = tmp;
        }
        // 缩放
        rect.left = (int) (rect.left * scale);
        rect.top = (int) (rect.top * scale);
        rect.right = (int) (rect.right * scale);
        rect.bottom = (int) (rect.bottom * scale);
        if (mirror) {
            // 平移
            rect.offset(offset, 0);
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
