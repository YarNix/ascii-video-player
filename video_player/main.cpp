extern "C"
{
#include <unistd.h>
#include <stdio.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}
#include "asciiPlayer.hpp"
#include <chrono>
#include <thread>
extern const char* TABLE_SIZES_MSG;

using namespace std::chrono;

inline void try_exit(int exit_code)
{
    if (exit_code < 0)
    {
        char buff[AV_ERROR_MAX_STRING_SIZE];
        printf_s("error: %s\n", av_make_error_string(buff, AV_ERROR_MAX_STRING_SIZE, exit_code));
        exit(EXIT_FAILURE);
    }
}
inline void try_exit(bool condition, int exit_code)
{
    if (condition)
        try_exit(exit_code);
}
void init_args(int argc, char const **argv, const char* &media_file, std::pair<int, const char*> &table, int& fntSize)
{
    if (argc == 1) goto lbUsage;
    int opt;
    while ((opt = getopt(argc, (char*const*)argv, "hc:f:")) != -1) // 
    {
        int size;
        const char* wStr; char* hStr;
        switch (opt)
        {
        case 'c': // character set
            size = atoi(optarg);
            if(size == 0){
                // No int conversion, maybe custom?
                int len = strlen(optarg);
                const char* custom;
                // Ensure {{ CHARACTERS }} format
                if (optarg[0] != '{' || optarg[1] != '{' ||
                    optarg[len - 2] != '}' || optarg[len - 1] != '}')
                    goto lbBadChar;
                // Is custom set
                optarg[len - 2] = '\0';
                table = {len, optarg + 2};
                break;
            }
            for (int i = 0; i <= 4; i++)
            {
                if (i == 4) goto lbBadChar;
                if (size == ASCII_TABLES[i].first)
                {
                    table = ASCII_TABLES[i];
                    break;
                }
            }
            break;
        case 'f': // font size
            fntSize = atoi(optarg);
            if (fntSize <= 0) goto lbBadScale;
            break;
        case '?':
            exit(EXIT_FAILURE);
        default:
        case 'h': // help
            goto lbUsage;
        }
    }
    if (optind >= argc) goto lbBadLastArg;
    media_file = argv[optind];
    return;
    lbUsage:
    printf( "usage: %s [options...] <media_file>\n"
            "   -h\tPrint this message\n"
            "   -c\tSet the characters to use when converting\n"
            "     \tAvailable: %s or define your own using {{}}\n"
            "     \tdefault is 14 or {{ .:-=;+?*E#0%@}}\n"
            "   -f\tChange the console's fontSize\n"
            "     \tthe program will guess the smallest size if not set\n"
            , argv[0], TABLE_SIZES_MSG);
    exit(EXIT_SUCCESS);
    lbBadLastArg:
    printf("error: Missing media, no file path supplied");
    exit(EXIT_FAILURE);
    lbBadChar:
    printf("error: Invalid character set: \"%s\"\n%s\n", optarg, TABLE_SIZES_MSG);
    exit(EXIT_FAILURE);
    lbBadScale:
    printf("error: Invalid size value: %s\n", optarg);
    exit(EXIT_FAILURE);
}
void init_fmt_cxt(const char *filename, AVFormatContext **format)
{
    // Open file
    try_exit(avformat_open_input(format, filename, NULL, NULL));
    // Fill stream info
    try_exit(avformat_find_stream_info(*format, NULL));
}
int init_stream(AVStream **stream, const AVCodec **decoder, AVFormatContext *format, AVMediaType media_type)
{
    // Open the selected stream
    int index = av_find_best_stream(format, media_type, -1, -1, NULL, 0);
    if (index < 0)
    {
        printf_s("Media type: %s\n", av_get_media_type_string(media_type));
        try_exit(index);
    }
    // Set the stream
    *stream = format->streams[index];
    // Create decoder from stream
    *decoder = avcodec_find_decoder((*stream)->codecpar->codec_id);
    try_exit(!(*decoder), AVERROR(EINVAL));
    return index;
}
void init_codec_cxt(AVCodecContext **codec, const AVCodec *decoder, AVCodecParameters *params)
{
    // Create a codec context from the decoder
    *codec = avcodec_alloc_context3(decoder);
    try_exit(!(*codec), AVERROR(ENOMEM));
    // Copy codec parameter from input stream to ouput codec context
    try_exit(avcodec_parameters_to_context(*codec, params));
    // Init the codec context
    try_exit(avcodec_open2(*codec, decoder, NULL));
}
void init_frame(AVFrame **frame)
{
    *frame = av_frame_alloc();
    try_exit(!(*frame), AVERROR(ENOMEM));
}
void init_packet(AVPacket **packet)
{
    *packet = av_packet_alloc();
    try_exit(!(*packet), AVERROR(ENOMEM));
}

void decode_to_grayscale(AsciiDisplay *display, AVFormatContext *format, AVCodecContext *codec, int stream_index)
{
    int width, height;
    AVFrame *frame = NULL;
    AVPacket *packet = NULL;
    width = codec->width;
    height = codec->height;
    init_frame(&frame);
    init_packet(&packet);

    int outW, outH;
    uint8_t *outBuffer[4] = {};
    int outPlane[4] = {};
    outW = display->getBufferWidth();
    outH = display->getBufferHeight();
    try_exit(av_image_alloc(outBuffer, outPlane, outW, outH, AV_PIX_FMT_GRAYF32, 4) < 0, AVERROR(ENOMEM));

    SwsContext *converter = NULL;
    converter = sws_getContext(width, height, codec->pix_fmt, outW, outH, AV_PIX_FMT_GRAYF32, SWS_SINC | SWS_ACCURATE_RND, NULL, NULL, NULL);
    nanoseconds sleep_duration;
    int64_t nano_ratio = av_q2d(format->streams[stream_index]->time_base) * 1e9;

    while (av_read_frame(format, packet) >= 0)
    {
        if (packet->stream_index == stream_index)
        {
            // submit the packet to the decoder
            try_exit(avcodec_send_packet(codec, packet));
            // get all the available frames from the decoder
            while (true)
            {
                auto start = high_resolution_clock::now();
                int code = avcodec_receive_frame(codec, frame);
                if (code < 0)
                {
                    // those two return values are special and mean there is no output
                    // frame available, but there were no errors during decoding
                    try_exit(code != AVERROR_EOF && code != AVERROR(EAGAIN), code);
                    av_frame_unref(frame);
                    break;
                }
                sws_scale(converter, frame->data, frame->linesize, 0, height, outBuffer, outPlane);
                // TODO: Add buffer image which make displaying image run asynchronous with extracting frames
                // Would be nice if we could also couple it with converting the image to ascii and make display just call WriteConsoleOutputCharacterA
                display->Display<float>(outBuffer[0], outPlane[0]);
                nanoseconds frame_duration{frame->duration * nano_ratio};
                av_frame_unref(frame);
                auto end = high_resolution_clock::now();
                sleep_duration = frame_duration - (end - start);
                if (sleep_duration.count() > 128)
                    std::this_thread::sleep_for(sleep_duration);
            }
        }
        av_packet_unref(packet);
    }

    sws_freeContext(converter);
    av_freep(&outBuffer[0]);
    av_packet_free(&packet);
    av_frame_free(&frame);
}

int main(int argc, char const **argv)
{
    const char *media_file = NULL;
    // Setting default argument
    std::pair<int, const char *> table = ASCII_TABLES[1];
    int fntSize = 0;
    init_args(argc, argv, media_file, table, fntSize);

    AVFormatContext *fmt_context = NULL;
    AVStream *video_stream = NULL;
    const AVCodec *decoder = NULL;
    int video_index;
    AVCodecContext *codec_context = NULL;
    
    init_fmt_cxt(media_file, &fmt_context);
    video_index = init_stream(&video_stream, &decoder, fmt_context, AVMEDIA_TYPE_VIDEO);
    init_codec_cxt(&codec_context, decoder, video_stream->codecpar);

    AsciiDisplay as_display(codec_context->width, codec_context->height, media_file, table, fntSize);
    decode_to_grayscale(&as_display, fmt_context, codec_context, video_index);

    avcodec_close(codec_context);
    avcodec_free_context(&codec_context);
    avformat_close_input(&fmt_context);
}
