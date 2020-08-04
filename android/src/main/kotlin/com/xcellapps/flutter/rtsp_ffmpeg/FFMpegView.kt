package com.xcellapps.flutter.rtsp_ffmpeg

import android.content.Context
import android.view.SurfaceHolder
import android.view.SurfaceView
import android.view.View
import com.potterhsu.rtsplibrary.NativeCallback
import com.potterhsu.rtsplibrary.RtspClient
import io.flutter.plugin.common.BinaryMessenger
import io.flutter.plugin.common.MethodCall
import io.flutter.plugin.common.MethodChannel
import io.flutter.plugin.platform.PlatformView

class FFMpegView internal constructor(context: Context?, messenger: BinaryMessenger, id: Int): PlatformView, MethodChannel.MethodCallHandler{

    private val playerView = SurfaceView(context)
    private val methodChannel: MethodChannel
    private val rtspClient = RtspClient(NativeCallback { frame, nChannel, width, height ->})
    init {
        playerView.holder.addCallback(object : SurfaceHolder.Callback {
            override fun surfaceChanged(p0: SurfaceHolder?, p1: Int, p2: Int, p3: Int) {
                p0?.let {
                    rtspClient.setHolder(it.surface, p1, p2)
                }
            }

            override fun surfaceDestroyed(p0: SurfaceHolder?) {
            }

            override fun surfaceCreated(p0: SurfaceHolder?) {
            }

        })
        methodChannel = MethodChannel(messenger, "rtsp_ffmpeg$id")
        methodChannel.setMethodCallHandler(this)
    }

    override fun getView(): View {
        return playerView
    }

    override fun dispose() {
        methodChannel.setMethodCallHandler(null)
    }

    override fun onMethodCall(call: MethodCall, result: MethodChannel.Result) {
        when(call.method) {
            "play" -> onPlay(call.arguments as String, result)
            "stop" -> onStop(result)
            else -> result.notImplemented()
        }
    }

    private fun onStop(result: MethodChannel.Result) {
        rtspClient.stop()
        result.success(true)
    }

    private fun onPlay(s: String, result: MethodChannel.Result) {
        val res = rtspClient.play(s) == 0
        if(res) {
            result.success(res)
        }else{
            result.error("$res", "Error opening URL", "Error opening url")
        }
    }
}