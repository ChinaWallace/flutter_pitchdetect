import 'package:flutter/material.dart';
import 'package:flutter/services.dart';
import 'package:pitchdetect_flutter/pitch_detect.dart';
import 'package:record/record.dart';
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
      debugShowCheckedModeBanner: false,
      title: 'Pitch Detector (Record)',
      theme: ThemeData(
        colorScheme: ColorScheme.fromSeed(seedColor: Colors.blue),
        useMaterial3: true,
      ),
      home: const MyHomePage(title: 'Pitch Detector (Record)'),
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
  final _audioRecorder = AudioRecorder();
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
    // _initRecorder();
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
  }

  Future<void> _requestPermissions() async {
    final status = await Permission.microphone.request();
    if (status != PermissionStatus.granted) {
      print('Microphone permission denied');
    }
  }

  Future<void> _sendAudioDataToSwift(Uint8List audioData) async {
    try {
      myPitchDetect.sendAudioData(audioData);
    } on PlatformException catch (e) {
      print('Failed to send audio data to Swift: ${e.message}');
    }
  }

  Future<void> _startRecording() async {
    try {
      // if (await Permission.microphone.isGranted) {
        final tempDir = await getTemporaryDirectory();
        final path = '${tempDir.path}/temp_audio.wav';

        // Start recording with record package
        await _audioRecorder.start(
          RecordConfig(
            encoder: AudioEncoder.wav,
            sampleRate: 44100,
            numChannels: 1,
          ),
          path: path,
        );


        // Listen to the raw audio data
        _audioRecorder.onAmplitudeChanged(const Duration(microseconds: 10)).listen(
              (amp) {
            // Here we can process the audio data
            // For now, we'll just send the amplitude data
            final data = Uint8List.fromList([amp.current.toInt()]);
            _sendAudioDataToSwift(data);
          },
        );

        setState(() {
          _isRecording = true;
        });
      // }
    } catch (e) {
      print('Error starting recording: $e');
    }
  }

  Future<void> _stopRecording() async {
    try {
      await _audioRecorder.stop();
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
    _audioRecorder.dispose();
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