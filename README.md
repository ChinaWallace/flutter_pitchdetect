flutter下android和ios通过麦克风的音频解析出音高分贝赫兹

调用PitchDetect.dart的init方法初始化解析器
调用sendAudioData传递音频字节流给c++去解析
通过回调获取解析数据


![效果显示](https://github.com/ChinaWallace/flutter_pitchdetect/blob/main/IMG_6486.PNG)

