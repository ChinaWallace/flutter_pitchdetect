import 'package:flutter/material.dart';
import 'package:flutter/services.dart';
import 'package:pitchdetect_flutter/pitch_detect.dart';
import 'package:flutter_sound/flutter_sound.dart';
import 'dart:math' as math;
import 'package:path_provider/path_provider.dart';
import 'dart:typed_data';
import 'package:permission_handler/permission_handler.dart';
import 'dart:convert';
import 'dart:async';

void main() {
  runApp(const MyApp());
}

class MyApp extends StatelessWidget {
  const MyApp({super.key});

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'Pitch Detector',
      theme: ThemeData(
        colorScheme: ColorScheme.fromSeed(seedColor: Colors.blue),
        useMaterial3: true,
      ),
      home: const MyHomePage(title: 'Pitch Detector'),
    );
  }
}

class MyHomePage extends StatefulWidget {
  const MyHomePage({super.key, required this.title});
  final String title;

  @override
  State<MyHomePage> createState() => _MyHomePageState();
}

class _MyHomePageState extends State<MyHomePage> {
  final _audioRecorder = FlutterSoundRecorder();
  bool _isRecording = false;
  String _noteName = '--';
  double _currentPitch = 0.0;
  double _currentDecibel = 0.0;
  final List<double> _audioBuffer = [];
  PitchDetect myPitchDetect = PitchDetect();
  StreamController<Uint8List> _audioStreamController = StreamController<Uint8List>();

  @override
  void initState() {
    super.initState();
    _initRecorder();
    myPitchDetect.init((message) {
      try {
        final data = json.decode(message);
        setState(() {
          _noteName = data['noteName'] ?? '--';
          _currentPitch = (data['pitch'] ?? 0.0).toDouble();
          _currentDecibel = (data['decibel'] ?? 0.0).toDouble();
        });
      } catch (e) {
        print('Error parsing pitch detection data: $e');
      }
    });
  }

  Future<void> _initRecorder() async {
    await _requestPermissions();
    await _audioRecorder.openRecorder();
    await _audioRecorder.setSubscriptionDuration(const Duration(milliseconds: 50));
  }

  Future<void> _requestPermissions() async {
    final status = await Permission.microphone.request();
    if (status != PermissionStatus.granted) {
      // Handle permission denied
      print('Microphone permission denied');
    }
  }

  Future<void> _sendAudioDataToSwift(Uint8List audioData) async {
    try {
      await platform.invokeMethod('receiveAudioData', {'audioData': audioData});
    } on PlatformException catch (e) {
      print('Failed to send audio data to Swift: ${e.message}');
    }
  }

  Future<void> _startRecording() async {
    try {
      if (await Permission.microphone.isGranted) {

        // 设置音频配置
        await _audioRecorder.startRecorder(
          codec: Codec.pcm16,
          sampleRate: 44100,
          toStream: _audioStreamController.sink,
        );

        _audioStreamController.stream.listen((Uint8List audioData) {
          _sendAudioDataToSwift(audioData);
        });

        _isRecording = true;
      }
    } catch (e) {
      print('Error starting recording: $e');
    }
  }

  Future<void> _stopRecording() async {
    try {
      _audioStreamController.close();
      await _audioRecorder.stopRecorder();
      setState(() {
        _isRecording = false;
        _audioBuffer.clear();
      });
    } catch (e) {
      print('Error stopping recording: $e');
    }
  }

  @override
  void dispose() {
    _audioStreamController.close();
    _audioRecorder.closeRecorder();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        backgroundColor: Theme.of(context).colorScheme.inversePrimary,
        title: Text(widget.title),
      ),
      body: Center(
        child: Column(
          mainAxisAlignment: MainAxisAlignment.center,
          children: <Widget>[
            Text(
              'Note: $_noteName',
              style: Theme.of(context).textTheme.headlineMedium,
            ),
            const SizedBox(height: 20),
            Text(
              'Pitch: ${_currentPitch.toStringAsFixed(2)} Hz',
              style: Theme.of(context).textTheme.headlineMedium,
            ),
            const SizedBox(height: 20),
            Text(
              'Decibel: ${_currentDecibel.toStringAsFixed(2)} dB',
              style: Theme.of(context).textTheme.headlineMedium,
            ),
            const SizedBox(height: 40),
            FloatingActionButton(
              onPressed: _isRecording ? _stopRecording : _startRecording,
              child: Icon(_isRecording ? Icons.stop : Icons.mic),
            ),
          ],
        ),
      ),
    );
  }
}