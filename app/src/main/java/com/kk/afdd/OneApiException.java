package com.kk.afdd;


import androidx.annotation.Nullable;

public class OneApiException extends Exception {
    private int mCode;
    private String mMessage;

    public OneApiException(int code, String message) {
        super(message);
        this.mCode = code;
        this.mMessage = message;
    }

    public int getCode() {
        return mCode;
    }

    public void setCode(int code) {
        mCode = code;
    }

    @Nullable
    @Override
    public String getMessage() {
        return mMessage;
    }

    public void setMessage(String message) {
        mMessage = message;
    }
}
