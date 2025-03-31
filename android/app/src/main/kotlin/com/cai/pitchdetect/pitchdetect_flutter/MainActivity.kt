package com.cai.pitchdetect.pitchdetect_flutter

import android.Manifest
import android.R
import android.content.pm.PackageManager
import android.util.Log
import android.widget.TextView
import androidx.core.app.ActivityCompat
import androidx.core.content.ContextCompat
import io.flutter.embedding.android.FlutterActivity
import io.flutter.embedding.engine.FlutterEngine
import io.flutter.plugin.common.MethodCall
import io.flutter.plugin.common.MethodChannel


// 回调接口
interface Callback {
    fun onStringReceived(message: String)
}

// 声明本地方法
private external fun processAudioData(data: ByteArray, size: Int)
private external fun initCallback(callback: Callback)
//private external fun releaseCallback()

class MainActivity : FlutterActivity() {

    companion object {
        init {
            System.loadLibrary("pitch")
            Log.d("MainActivity", "Native library loaded")
        }
        private const val CHANNEL = "com.cai.pitchDetect/native"
        private const val PERMISSION_REQUEST_CODE = 123
        private const val TAG = "MainActivity"
    }

    private var isInitialized = false

    override fun configureFlutterEngine(flutterEngine: FlutterEngine) {
        super.configureFlutterEngine(flutterEngine)
        Log.d(TAG, "Configuring Flutter engine")

        // 创建 MethodChannel，通道名称需与 Flutter 端保持一致
        MethodChannel(flutterEngine.dartExecutor.binaryMessenger, CHANNEL)
            .setMethodCallHandler { call: MethodCall, result: MethodChannel.Result ->

                when (call.method) {
                    "startRecord" -> {
                        Log.d(TAG, "Starting record...")
                        if (checkAndRequestPermissions()) {
                            Log.d(TAG, "Permissions granted, setting up callback")
                            val callback = object : Callback {
                                override fun onStringReceived(message: String) {

                                    runOnUiThread(Runnable { // 在这里执行与 UI 相关的操作，比如更新 TextView 的文本
                                        MethodChannel(flutterEngine.dartExecutor.binaryMessenger, CHANNEL).invokeMethod(
                                            "sendString",
                                            message
                                        )
                                    })

                                }
                            }
                            initCallback(callback)
                            isInitialized = true
                            result.success("Recording started")
                        } else {
                            Log.e(TAG, "Permissions denied")
                            result.error("PERMISSION_DENIED", "Audio recording permission denied", null)
                        }
                    }
                    "receiveAudioData" -> {
                        if (!isInitialized) {
                            Log.e(TAG, "Audio recorder not initialized")
                            result.error("NOT_INITIALIZED", "Audio recorder not initialized", null)
                            return@setMethodCallHandler
                        }

                        if (call.arguments is Map<*, *>) {
                            val args = call.arguments as Map<*, *>
                            val audioData = args["audioData"]
                            if (audioData is ByteArray) {
                                try {
                                    if (audioData.isEmpty()) {
                                        Log.e(TAG, "Empty audio data received")
                                        result.error("INVALID_DATA", "Empty audio data", null)
                                        return@setMethodCallHandler
                                    }
//                                    Log.d(TAG, "Processing audio data, size: ${audioData.size}")
                                    processAudioData(audioData, audioData.size)
                                    result.success(null)
                                } catch (e: Exception) {
                                    Log.e(TAG, "Error processing audio data: ${e.message}")
                                    result.error("PROCESSING_ERROR", e.message, null)
                                }
                            } else {
                                Log.e(TAG, "Invalid audio data type: ${audioData?.javaClass?.simpleName}")
                                result.error("INVALID_ARGUMENT", "Audio data is not ByteArray", null)
                            }
                        } else {
                            Log.e(TAG, "Invalid arguments type: ${call.arguments?.javaClass?.simpleName}")
                            result.error("INVALID_ARGUMENT", "Arguments must be a Map", null)
                        }
                    }
                    else -> {
                        Log.e(TAG, "Method not implemented: ${call.method}")
                        result.notImplemented()
                    }
                }
            }
        Log.d(TAG, "Method channel setup complete")
    }

    private fun checkAndRequestPermissions(): Boolean {
        Log.d(TAG, "Checking permissions")
        val permissions = arrayOf(
            Manifest.permission.RECORD_AUDIO,
            Manifest.permission.WRITE_EXTERNAL_STORAGE,
            Manifest.permission.READ_EXTERNAL_STORAGE
        )

        val permissionsToRequest = mutableListOf<String>()
        for (permission in permissions) {
            if (ContextCompat.checkSelfPermission(this, permission) != PackageManager.PERMISSION_GRANTED) {
                Log.d(TAG, "Permission not granted: $permission")
                permissionsToRequest.add(permission)
            }
        }

        if (permissionsToRequest.isNotEmpty()) {
            Log.d(TAG, "Requesting permissions: $permissionsToRequest")
            ActivityCompat.requestPermissions(this, permissionsToRequest.toTypedArray(), PERMISSION_REQUEST_CODE)
            return false
        }

        Log.d(TAG, "All permissions granted")
        return true
    }

    override fun onRequestPermissionsResult(
        requestCode: Int,
        permissions: Array<out String>,
        grantResults: IntArray
    ) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults)
        if (requestCode == PERMISSION_REQUEST_CODE) {
            val allGranted = grantResults.all { it == PackageManager.PERMISSION_GRANTED }
            if (!allGranted) {
                Log.e(TAG, "Some permissions were denied")
            } else {
                Log.d(TAG, "All permissions granted")
            }
        }
    }

    override fun onDestroy() {
        super.onDestroy()
        if (isInitialized) {
            release()
            isInitialized = false
        }
    }
}