package com.kk.afdd;

public class BaseResponse<T> {

    public static final int STATUS_OK = 0;

    public int code;
    public String msg;
    public T data;

    public boolean getSuccess() {
        return STATUS_OK == code;
    }

    public boolean isSuccess() {
        return getSuccess();
    }
}
