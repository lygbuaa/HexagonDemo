package com.hexagondemo.calculator;

import android.content.Intent;
//import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.EditText;
import android.widget.TextView;
import androidx.appcompat.app.AppCompatActivity;

import java.io.File;

public class MainCalculator extends AppCompatActivity {
    String TAG = "hexagon";
    public static final String RESULT_MESSAGE = "com.hexagondemo.calculator.MESSAGE";
    // Used to load the 'calculator' library on application startup.
    static {
        System.loadLibrary("calculator");
    }
    private int[] mVec;


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main_calculator);
        Log.i(TAG, "MainCalculator launch");
        try {
            //Set the path of libcalculator_skel.so
            //Push the skel shared object to the location /data/app
            String skel_location = "/vendor/lib/rfsa/adsp"; // /data/app/hexagon  /vendor/lib/rfsa/adsp
//            System.out.println("Skel library location : " + skel_location);
            Log.i(TAG, "Skel library location : " + skel_location);
            //Set the ADSP_LIBRARY_PATH to skel_location
            init(skel_location);
            EditText resultText = findViewById(R.id.resultText);
            resultText.setText("push *skel.so to " + skel_location);
            Log.i(TAG, "skel init done");
        } catch (Exception e) {
//            System.out.println(e.getMessage());
            Log.e(TAG, "MainCalculator exception " + e.toString());
        }
    }
    /** Called when the user taps the Send button */
    public void sendMessage(View view) {
//        Intent intent = new Intent(this, DisplayResult.class);
        EditText inputText = findViewById(R.id.inputText);
        EditText resultText = findViewById(R.id.resultText);
        String message;
        try {
            //Create an array of length len defined by user
            int len = Integer.valueOf(inputText.getText().toString());
            mVec = new int[len];
            for (int i = 0; i < len; i++) {
                mVec[i] = i;
            }
//            message = "sum of " + String.valueOf(len) + " numbers is " + String.valueOf(sum(mVec, len));
            message = "remap: " + String.valueOf(remap());
            resultText.setText(message);
        } catch (Exception e) {
            message = "Please retry with a valid number !";
        }
//        intent.putExtra(RESULT_MESSAGE, message);
//        startActivity(intent);
    }
    /**
     * The native methods that are implemented by the 'calculator' native library,
     * which is packaged with this application.
     * init: input is Skel_location which sets ADSP_LIBRARY_PATH
     * sum: input is a vector containing data, and its length
     * return value is the sum of the whole vector
     */
    public native int init(String skel_location);
    public native long sum(int[] vec, int len);
    public native long remap();
}
