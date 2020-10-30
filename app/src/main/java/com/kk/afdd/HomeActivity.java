package com.kk.afdd;

import android.annotation.SuppressLint;
import android.content.Context;
import android.net.Uri;
import android.os.Bundle;
import android.widget.ArrayAdapter;
import android.widget.GridView;
import android.widget.Toast;

import com.qingmei2.rximagepicker.core.RxImagePicker;
import com.qingmei2.rximagepicker.entity.Result;
import com.qingmei2.rximagepicker.entity.sources.Camera;
import com.qingmei2.rximagepicker.entity.sources.Gallery;

import java.io.InputStream;

import io.reactivex.Observable;
import io.reactivex.ObservableOnSubscribe;
import timber.log.Timber;

@SuppressWarnings("ResultOfMethodCallIgnored")
@SuppressLint("CheckResult")
public class HomeActivity extends BaseActivity {

    private static final String[] BUTTONS = {"注册", "识别"};

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_home);

        GridView gridView = findViewById(R.id.buttons);
        ArrayAdapter<String> simpleAdapter = new ArrayAdapter<>(this,
                R.layout.button_layout,
                R.id.item,
                BUTTONS);
        gridView.setOnItemClickListener((parent, view, position, id) -> {
            String item = simpleAdapter.getItem(position);
            if (BUTTONS[0].equals(item)) {
                Timber.i("item click:%s", item);
                RxImagePicker.create(ImagePicker.class)
                        .openGallery(this)
                        .subscribe(result -> {
                            Uri imageUri = result.getUri();
                            RxRequestWrapper.with(Observable.create((ObservableOnSubscribe<User>) emitter -> {
                                try {
                                    InputStream iStream = getContentResolver().openInputStream(imageUri);
                                    if (iStream != null) {
                                        byte[] inputData = FileUtils.getBytes(iStream);
                                        emitter.onNext(null);
                                    }
                                } catch (Exception e) {
                                    e.printStackTrace();
                                    emitter.onError(e);
                                }
                                emitter.onComplete();
                            })).observeOnIoThread()
                                    .observeOnMainThread()
                                    .callback(response -> {
                                        String register = response.name + " 注册成功";
                                        Toast.makeText(this, register, Toast.LENGTH_SHORT).show();
                                        finish();
                                    });
                        });
            }
        });
        gridView.setAdapter(simpleAdapter);
    }

    public interface ImagePicker {

        @Gallery
        Observable<Result> openGallery(Context context);

        @Camera
        Observable<Result> openCamera(Context context);
    }
}