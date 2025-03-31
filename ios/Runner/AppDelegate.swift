import Flutter
import UIKit

@main
@objc class AppDelegate: FlutterAppDelegate {
  override func application(
    _ application: UIApplication,
    didFinishLaunchingWithOptions launchOptions: [UIApplication.LaunchOptionsKey: Any]?
  ) -> Bool {


  let controller : FlutterViewController = window?.rootViewController as! FlutterViewController
let channel = FlutterMethodChannel(name: "com.cai.pitchDetect/native", binaryMessenger: controller.binaryMessenger)
let wrapper: MyCppWrapper? = MyCppWrapper { value in
                              print("Received value: \(value)")
            channel.invokeMethod("sendString", arguments: value)
                          }

          channel.setMethodCallHandler { (call: FlutterMethodCall, result: @escaping FlutterResult) in
              if call.method == "startRecord" {
                        // 开始监听
                  wrapper?.startRecord()

                  result("startRecord")
              }
              if call.method == "receiveAudioData" {
                  if let args = call.arguments as? [String: Any],
                     let audioData = args["audioData"] as? FlutterStandardTypedData {
                      // 处理接收到的音频字节数组
                      let data = audioData.data
                      wrapper?.processByteArray(data)
                      result(nil)
                  }
              }
              else {
                  result(FlutterMethodNotImplemented)
              }
          }

    GeneratedPluginRegistrant.register(with: self)
    return super.application(application, didFinishLaunchingWithOptions: launchOptions)
  }
}
