package com.kk.afdd;


import io.reactivex.Observable;
import io.reactivex.ObservableSource;
import io.reactivex.ObservableTransformer;
import io.reactivex.android.schedulers.AndroidSchedulers;
import io.reactivex.disposables.Disposable;
import io.reactivex.functions.Function;
import io.reactivex.schedulers.Schedulers;


@SuppressWarnings("WeakerAccess")
public class RxRequestWrapper<T> {

    private Observable<T> mObservable;

    private RxRequestWrapper(Observable<T> observable) {
        mObservable = observable;
    }

    public static <T> RxRequestWrapper<T> with(Observable<T> observable) {
        return new RxRequestWrapper<>(observable);
    }

    /**
     * 编排
     */
    public RxRequestWrapper<T> compose(ObservableTransformer<T, T> composer) {
        mObservable = mObservable.compose(composer);
        return this;
    }

    /**
     * 在IO线程中执行
     */
    public RxRequestWrapper<T> subscribeOnIoThread() {
        return compose(upstream -> upstream.subscribeOn(Schedulers.io()));
    }

    /**
     * 在IO线程中观察
     */
    public RxRequestWrapper<T> observeOnIoThread() {
        return compose(upstream -> upstream.observeOn(Schedulers.io()));
    }

    /**
     * 在主线程中观察
     */
    public RxRequestWrapper<T> observeOnMainThread() {
        return compose(upstream -> upstream.observeOn(AndroidSchedulers.mainThread()));
    }

    /**
     * 解析执行结果，以callback形式返回
     * 注意：需要自己设置好callback的返回参数
     */
    @SuppressWarnings({"UnusedReturnValue"})
    public Disposable callback(Callback<T> callback) {
        return callback(mObservable, callback);
    }

    private <X> Disposable callback(Observable<X> observable, Callback<X> callback) {
        return observable.subscribe(data -> {
            if (callback != null) {
                callback.onSuccess(data);
            }
        }, throwable -> {
            if (callback != null) {
                callback.onError(throwable);
            }
        }, () -> {
            if (callback != null) {
                callback.onFinished();
            }
        }, disposable -> {
            if (callback != null) {
                callback.onSubscribe();
            }
        });
    }

    /**
     * 解析Http request，直接返回BaseResponse中的Data
     */
    @SuppressWarnings({"UnusedReturnValue", "unchecked"})
    public <X> Disposable httpCallback(Callback<X> callback) {
        Observable<BaseResponse<X>> observable = (Observable<BaseResponse<X>>) mObservable;
        Observable<X> mergedObserver = observable.compose(upstream ->
                upstream.onErrorResumeNext((Function<Throwable,
                        ObservableSource<BaseResponse<X>>>) throwable -> {
                    // 网络错误、解析错误等
                    return Observable.error(OneExceptionHandler.handleException(throwable));
                }).flatMap((Function<BaseResponse<X>, ObservableSource<X>>) response -> {
                    try {
                        // 请求成功
                        if (response.isSuccess()) {
                            return Observable.just(response.data);
                        } else {
                            // 网络请求成功、但逻辑有误
                            String message = response.msg;
                            return Observable.error(new OneApiException(response.code, message));
                        }
                    } catch (Exception e) {
                        e.printStackTrace();
                        return Observable.error(OneExceptionHandler.handleException(e));
                    }
                }));
        return callback(mergedObserver, callback);
    }


    public interface Callback<T> {
        /**
         * 成功
         */
        void onSuccess(T response);

        /**
         * 错误
         */
        default void onError(Throwable throwable) {
        }

        /**
         * 完成
         */
        default void onFinished() {
        }

        default void onSubscribe() {
        }
    }
}
