package io.github.hexhacking.xdl.sample;

import androidx.activity.EdgeToEdge;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.graphics.Insets;
import androidx.core.view.ViewCompat;
import androidx.core.view.WindowInsetsCompat;

import android.os.Bundle;
import android.view.View;

import io.github.hexhacking.xdl.Xdl;

public class MainActivity extends AppCompatActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        EdgeToEdge.enable(this);
        setContentView(R.layout.activity_main);
        ViewCompat.setOnApplyWindowInsetsListener(findViewById(R.id.main), (v, insets) -> {
            Insets systemBars = insets.getInsets(WindowInsetsCompat.Type.systemBars());
            v.setPadding(systemBars.left, systemBars.top, systemBars.right, systemBars.bottom);
            return insets;
        });

        findViewById(R.id.testButton).setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                long libartHandle = Xdl.open("libart.so", true);
                long libcHandle = Xdl.open("libc.so", true);

                NativeSample.test();

                Xdl.close(libartHandle);
                Xdl.close(libcHandle);
            }
        });
    }
}
