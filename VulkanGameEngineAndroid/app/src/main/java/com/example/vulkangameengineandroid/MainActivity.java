package com.example.vulkangameengineandroid;

import com.google.androidgamesdk.GameActivity;

public class MainActivity extends GameActivity {
    static {
        System.loadLibrary("VulkanGameEngine");
    }
}