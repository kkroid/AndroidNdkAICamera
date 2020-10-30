package com.kk.afdd;

import android.net.ParseException;

import com.google.gson.JsonParseException;

import org.json.JSONException;

import java.net.ConnectException;
import java.net.SocketTimeoutException;
import java.net.UnknownHostException;

@SuppressWarnings("WeakerAccess")
public class OneExceptionHandler {
    /**
     * 未知错误
     */
    public static final int UNKNOWN = 1000;

    /**
     * 解析错误
     */
    public static final int PARSE_ERROR = 1001;

    /**
     * 网络错误
     */
    public static final int NETWORK_ERROR = 1002;

    private OneExceptionHandler() {
    }

    public static OneApiException handleException(Throwable throwable) {
        OneApiException ex;
        String message = throwable.getMessage();
        if (throwable instanceof JsonParseException
                || throwable instanceof JSONException
                || throwable instanceof ParseException) {
            // 解析错误
            ex = new OneApiException(PARSE_ERROR, message);
            return ex;
        } else if (throwable instanceof ConnectException
                || throwable instanceof UnknownHostException
                || throwable instanceof SocketTimeoutException) {
            // 网络错误
            ex = new OneApiException(NETWORK_ERROR, message);
            return ex;
        } else {
            // 未知错误
            ex = new OneApiException(UNKNOWN, message);
            return ex;
        }
    }
}
