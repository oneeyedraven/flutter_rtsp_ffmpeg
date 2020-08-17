#include <jni.h>
#include <string>
#include <android/log.h>
#include <android/native_activity.h>
#include <android/native_window_jni.h>
#include <android/rect.h>
#include <android/surface_control.h>
#include <android/window.h>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <pthread.h>
}

const char *TAG = __FILE__;

jobject gCallback;
ANativeWindow *aNativeWindow;
jmethodID gCallbackMethodId;
bool isStop = false;

void callback(JNIEnv *env, uint8_t *buf, int channel, int width, int height);

void* playBackThread(void *args) {
    SwsContext *img_convert_ctx;
    AVFormatContext* context = avformat_alloc_context();
    AVCodecContext* ccontext = avcodec_alloc_context3(NULL);
    int video_stream_index = -1;

    av_register_all();
    avcodec_register_all();
    avformat_network_init();

//    AVCodec *current_codec = av_codec_next(current_codec);
//    while (current_codec != NULL)
//    {
//        std::string name(current_codec->name);
//        if(name.find("mediacodec"))
//            __android_log_print(ANDROID_LOG_ERROR, TAG,"codec:%s",current_codec->name);
//        current_codec = av_codec_next(current_codec);
//    }


//    AVDictionary *option = NULL;
    //av_dict_set(&option, "rtsp_transport", "tcp", 0);

    // Open RTSP
    char *rtspUrl = static_cast<char *>(args);
    if (int err = avformat_open_input(&context, rtspUrl, NULL, nullptr) != 0) {
        __android_log_print(ANDROID_LOG_ERROR, TAG, "Cannot open input %s, error : %s", rtspUrl, av_err2str(err));
        free(rtspUrl);
        return (void *)JNI_ERR;
    }
    free(rtspUrl);

    //av_dict_free(&option);

    if (avformat_find_stream_info(context, NULL) < 0){
        __android_log_print(ANDROID_LOG_ERROR, TAG, "Cannot find stream info");
        return (void *)JNI_ERR;
    }

    // Search video stream
    for (int i = 0; i < context->nb_streams; i++) {
        if (context->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
            video_stream_index = i;
    }

    if (video_stream_index == -1) {
        __android_log_print(ANDROID_LOG_ERROR, TAG, "Video stream not found");
        return (void *)JNI_ERR;
    }

    AVPacket packet;
    av_init_packet(&packet);

    // Open output file
    AVFormatContext *oc = avformat_alloc_context();
    AVStream *stream = NULL;

    // Start reading packets from stream and write them to file
    av_read_play(context);

    AVCodec *codec = NULL;
    codec = avcodec_find_decoder(AV_CODEC_ID_H264);
    if (!codec) {
        __android_log_print(ANDROID_LOG_ERROR, TAG, "Cannot find decoder H264");
        return (void *)JNI_ERR;
    }

    avcodec_get_context_defaults3(ccontext, codec);
    avcodec_copy_context(ccontext, context->streams[video_stream_index]->codec);
    int ret = avcodec_open2(ccontext, codec, NULL);
    if (ret < 0) {
        __android_log_print(ANDROID_LOG_ERROR, TAG, "Cannot open codec, %s", av_err2str(ret));
        return (void *)JNI_ERR;
    }

    img_convert_ctx = sws_getContext(ccontext->width, ccontext->height, ccontext->pix_fmt, ccontext->width, ccontext->height,
                                     AV_PIX_FMT_RGBA, SWS_BICUBIC, NULL, NULL, NULL);

    size_t size = (size_t) avpicture_get_size(AV_PIX_FMT_YUV420P, ccontext->width, ccontext->height);
    uint8_t *picture_buf = (uint8_t*)(av_malloc(size));
    AVFrame *pic = av_frame_alloc();
    AVFrame *picrgb = av_frame_alloc();
    size_t size2 = (size_t) avpicture_get_size(AV_PIX_FMT_RGBA, ccontext->width, ccontext->height);
    uint8_t *picture_buf2 = (uint8_t*)(av_malloc(size2));
    avpicture_fill( (AVPicture*) pic, picture_buf, AV_PIX_FMT_YUV420P, ccontext->width, ccontext->height );
    avpicture_fill( (AVPicture*) picrgb, picture_buf2, AV_PIX_FMT_RGBA, ccontext->width, ccontext->height );

    isStop = false;
    ANativeWindow_Buffer buffer;
    while (!isStop && av_read_frame(context, &packet) >= 0) {
        if (packet.stream_index == video_stream_index) { // Packet is video
            if (stream == NULL) {
                stream = avformat_new_stream(oc, context->streams[video_stream_index]->codec->codec);
                avcodec_copy_context(stream->codec, context->streams[video_stream_index]->codec);
                stream->sample_aspect_ratio = context->streams[video_stream_index]->codec->sample_aspect_ratio;
            }

            int check = 0;
            packet.stream_index = stream->id;
            avcodec_decode_video2(ccontext, pic, &check, &packet);
            sws_scale(img_convert_ctx, (const uint8_t * const *)pic->data, pic->linesize, 0,
                      ccontext->height, picrgb->data, picrgb->linesize);
            ANativeWindow_setBuffersGeometry(aNativeWindow, ccontext->width,ccontext->height, WINDOW_FORMAT_RGBA_8888);
            if(ANativeWindow_lock(aNativeWindow, &buffer, NULL) == 0) {
                // Draw the image onto the interface
                // Note: The pixel lengths of rgba_frame row and window_buffer row may not be the same here.
                // Need to convert well or maybe screen
                uint8_t *bits = (uint8_t *) buffer.bits;
                for (int h = 0; h < ccontext->height; h++) {
                    memcpy(bits + h * buffer.stride * 4,
                           picture_buf2 + h * picrgb->linesize[0],
                           picrgb->linesize[0]);
                }
                ANativeWindow_unlockAndPost(aNativeWindow);
            }
            //callback(env, picture_buf2, 3, ccontext->width, ccontext->height);
        }
        av_free_packet(&packet);
        av_init_packet(&packet);
    }

    av_free(pic);
    av_free(picrgb);
    av_free(picture_buf);
    av_free(picture_buf2);

    av_read_pause(context);
    avio_close(oc->pb);
    avformat_free_context(oc);
    avformat_close_input(&context);

    return (void *) (isStop ? JNI_OK : JNI_ERR);
}

extern "C"
jint
Java_com_potterhsu_rtsplibrary_RtspClient_initialize(
        JNIEnv *env,
        jobject,
        jobject callback) {
    gCallback = env->NewGlobalRef(callback);
    jclass clz = env->GetObjectClass(gCallback);
    if (clz == NULL) {
        return JNI_ERR;
    } else {
        gCallbackMethodId = env->GetMethodID(clz, "onFrame", "([BIII)V");
        return JNI_OK;
    }
}

extern "C"
jint
Java_com_potterhsu_rtsplibrary_RtspClient_play(
        JNIEnv *env,
        jobject,
        jstring endpoint) {
    const char *rtspUrl= env->GetStringUTFChars(endpoint, JNI_FALSE);
    char *copyUrl = (char *)malloc(sizeof(char)* strlen(rtspUrl));
    env->ReleaseStringUTFChars(endpoint, rtspUrl);
    strcpy(copyUrl, rtspUrl);
    pthread_t playThread;
    pthread_create(&playThread, nullptr, playBackThread, (void *)copyUrl);
    return JNI_OK;
}

extern "C"
void
Java_com_potterhsu_rtsplibrary_RtspClient_stop(
        JNIEnv *env,
        jobject) {
    isStop = true;
}

extern "C"
void
Java_com_potterhsu_rtsplibrary_RtspClient_dispose(
        JNIEnv *env,
        jobject) {
    env->DeleteGlobalRef(gCallback);
    ANativeWindow_release(aNativeWindow);
    aNativeWindow = nullptr;
}

void callback(JNIEnv *env, uint8_t *buf, int nChannel, int width, int height) {
    int len = nChannel * width * height;
    jbyteArray gByteArray = env->NewByteArray(len);
    env->SetByteArrayRegion(gByteArray, 0, len, (jbyte *) buf);
    env->CallVoidMethod(gCallback, gCallbackMethodId, gByteArray, nChannel, width, height);
    env->DeleteLocalRef(gByteArray);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_potterhsu_rtsplibrary_RtspClient_setHolder(JNIEnv *env, jobject thiz, jobject surface, int width, int height) {
    ANativeWindow* pWindow(ANativeWindow_fromSurface(env, surface));
    aNativeWindow = pWindow;
}