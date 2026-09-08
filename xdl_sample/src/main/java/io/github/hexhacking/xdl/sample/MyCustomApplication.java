package io.github.hexhacking.xdl.sample;

import android.app.Application;
import android.content.Context;

public class MyCustomApplication extends Application {

    @Override
    protected void attachBaseContext(Context base) {
        super.attachBaseContext(base);

        System.loadLibrary("xdl");
        System.loadLibrary("sample");
    }
}
