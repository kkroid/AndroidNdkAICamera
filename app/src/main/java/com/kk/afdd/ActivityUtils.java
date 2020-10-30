package com.kk.afdd;

import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;

import androidx.annotation.Nullable;
import androidx.fragment.app.Fragment;
import androidx.fragment.app.FragmentActivity;
import androidx.fragment.app.FragmentManager;

import org.jetbrains.annotations.NotNull;

import java.util.Objects;

import timber.log.Timber;

/**
 *
 */
public class ActivityUtils {

    public static final String EXTRA_CALLBACK_DATA = "EXTRA_CALLBACK_DATA";
    private static final String EXTRA_CALLBACK_RESULT = "EXTRA_CALLBACK_RESULT";
    public static final int REQUEST_CODE = 1000;

    private static final String FRAGMENT_TAG = "EmptyFragment";

    public static <T> void startWithCallback(Context context, Intent intent, Callback<T> callback) {
        if (context instanceof FragmentActivity) {
            FragmentActivity activity = (FragmentActivity) context;
            FragmentManager fragmentManager = activity.getSupportFragmentManager();
            EmptyFragment<T> fragment = find(fragmentManager);
            if (fragment == null) {
                fragment = new EmptyFragment<>();
            }
            fragment.setCallback(callback);
            fragment.setIntent(intent);
            fragmentManager.beginTransaction().add(fragment, FRAGMENT_TAG).commitNow();
        }
    }

    @SuppressWarnings("unchecked")
    private static <T> EmptyFragment<T> find(FragmentManager fragmentManager) {
        return (EmptyFragment) fragmentManager.findFragmentByTag(FRAGMENT_TAG);
    }

    public interface Callback<T> {
        void callback(T data);
    }

    public static class EmptyFragment<T> extends Fragment {

        private Callback<T> mCallback;
        private Intent mIntent;

        @Nullable
        @Override
        public View onCreateView(@NotNull LayoutInflater inflater,
                                 @Nullable ViewGroup container,
                                 @Nullable Bundle savedInstanceState) {
            View view = new View(getContext());
            view.setBackgroundResource(android.R.color.transparent);

            if (mIntent != null) {
                mIntent.putExtra(ActivityUtils.EXTRA_CALLBACK_RESULT, true);
                startActivityForResult(mIntent, ActivityUtils.REQUEST_CODE);
            }
            return view;
        }

        private void setCallback(Callback<T> callback) {
            mCallback = callback;
        }

        private void setIntent(Intent intent) {
            mIntent = intent;
        }

        @Override
        public void onActivityResult(int requestCode, int resultCode, Intent data) {
            super.onActivityResult(requestCode, resultCode, data);
            if (data != null && data.hasExtra(ActivityUtils.EXTRA_CALLBACK_DATA)) {
                @SuppressWarnings("unchecked")
                T payload = (T) data.getSerializableExtra(ActivityUtils.EXTRA_CALLBACK_DATA);
                Timber.i("Data callback:%s", payload);
                if (mCallback != null) {
                    mCallback.callback(payload);
                }
            }
            Objects.requireNonNull(getActivity()).getSupportFragmentManager().beginTransaction().remove(this).commitNow();
        }
    }
}
