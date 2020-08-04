#import "RtspFfmpegPlugin.h"
#if __has_include(<rtsp_ffmpeg/rtsp_ffmpeg-Swift.h>)
#import <rtsp_ffmpeg/rtsp_ffmpeg-Swift.h>
#else
// Support project import fallback if the generated compatibility header
// is not copied when this plugin is created as a library.
// https://forums.swift.org/t/swift-static-libraries-dont-copy-generated-objective-c-header/19816
#import "rtsp_ffmpeg-Swift.h"
#endif

@implementation RtspFfmpegPlugin
+ (void)registerWithRegistrar:(NSObject<FlutterPluginRegistrar>*)registrar {
  [SwiftRtspFfmpegPlugin registerWithRegistrar:registrar];
}
@end
