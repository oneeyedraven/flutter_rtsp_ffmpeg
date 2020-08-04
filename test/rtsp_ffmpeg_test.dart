import 'package:flutter/services.dart';
import 'package:flutter_test/flutter_test.dart';
import 'package:rtsp_ffmpeg/rtsp_ffmpeg.dart';

void main() {
  const MethodChannel channel = MethodChannel('rtsp_ffmpeg');

  TestWidgetsFlutterBinding.ensureInitialized();

  setUp(() {
    channel.setMockMethodCallHandler((MethodCall methodCall) async {
      return '42';
    });
  });

  tearDown(() {
    channel.setMockMethodCallHandler(null);
  });

  test('getPlatformVersion', () async {
    expect('42', '42');
  });
}
