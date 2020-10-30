package com.kk.afdd;

import android.app.Application;

import org.jetbrains.annotations.NotNull;

import io.objectbox.BoxStore;
import timber.log.Timber;

public class App extends Application {

    public BoxStore boxStore;

    private static App sInstance;

    public static App getInstance() {
        return sInstance;
    }

    @Override
    public void onCreate() {
        super.onCreate();
        sInstance = this;
        boxStore = MyObjectBox.builder().androidContext(this).build();
        Timber.plant(new Timber.DebugTree() {
            @Override
            protected String createStackElementTag(@NotNull StackTraceElement element) {
                return String.format("(%s:%s) %s",
                        element.getFileName(),
                        element.getLineNumber(),
                        element.getMethodName());
            }
        });
    }
}
