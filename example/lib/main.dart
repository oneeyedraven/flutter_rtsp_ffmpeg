import 'package:flutter/material.dart';
import 'package:rtsp_ffmpeg/rtsp_ffmpeg.dart';

void main() {
  runApp(MyApp());
}

class MyApp extends StatelessWidget {
  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      home: MyHomePage(),
    );
  }
}

class MyHomePage extends StatefulWidget {
  MyHomePage({Key key}) : super(key: key);

  @override
  _MyHomePageState createState() => _MyHomePageState();
}

class _MyHomePageState extends State<MyHomePage> {
  RtspController _controller;
  bool _isPlaying = false;

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        actions: [
          IconButton(
            onPressed: () {
              if (_controller != null) {
                if (_isPlaying) {
                  _controller.stop();
                } else {
                  _controller.play('rtsp://192.168.0.1:554');
                }

                setState(() => _isPlaying = !_isPlaying);
              }
            },
            icon: Icon(_isPlaying ? Icons.stop : Icons.play_arrow),
          ),
        ],
      ),
      body: RtspFFMpeg(
        createdCallback: (controller) {
          setState(() => _controller = controller);
        },
      ),
    );
  }
}
