import 'package:flutter/services.dart';

const platform = MethodChannel('com.cai.pitchDetect/native');

class PitchDetect{

  Future<String> init(Function callback) async {
    try {
      final String result = await platform.invokeMethod('startRecord');

      platform.setMethodCallHandler((call) async {
        if (call.method == 'sendString') {

          var receivedString = call.arguments as String;
          callback(receivedString);
        }
      });
      return result;
    } on PlatformException catch (e) {
      return "Failed to init PitchDetect: '${e.message}'";
    }
  }

  Future<void> sendAudioData(Uint8List audioData) async {
    try {
      await platform.invokeMethod('receiveAudioData', {'audioData': audioData});
    } on PlatformException catch (e) {
      print('Failed to send audio data : ${e.message}');
    }
  }


}