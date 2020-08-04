import Flutter
import UIKit

public class SwiftRtspFfmpegPlugin: NSObject, FlutterPlugin {
  public static func register(with registrar: FlutterPluginRegistrar) {
    let channel = FlutterMethodChannel(name: "rtsp_ffmpeg", binaryMessenger: registrar.messenger())
    let instance = SwiftRtspFfmpegPlugin()
    registrar.addMethodCallDelegate(instance, channel: channel)
  }

  public func handle(_ call: FlutterMethodCall, result: @escaping FlutterResult) {
    result("iOS " + UIDevice.current.systemVersion)
  }
}
