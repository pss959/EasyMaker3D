#include "App/VideoWriter.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
}

#include <cstring>
#include <fstream>

#include "Util/Assert.h"
#include "Util/FilePath.h"

/// The VideoWriter::Data_ struct stores all the FFMPEG data needed for writing
/// video files.
struct VideoWriter::Data_ {
    AVCodecContext  *codec_context = nullptr;
    AVFormatContext *out_context   = nullptr;
    const AVCodec   *codec         = nullptr;
    AVFrame         *frame         = nullptr;
    AVPacket        *packet        = nullptr;
    AVStream        *stream        = nullptr;
    uint64           cur_frame     = 0;
};

VideoWriter::VideoWriter(const FilePath &path,
                         const Vector2i &resolution, int fps) {
    data_.reset(new Data_);
    Init_(path, resolution, fps);
}

VideoWriter::~VideoWriter() {}

void VideoWriter::Init_(const FilePath &path,
                        const Vector2i &resolution, int fps) {
    ASSERT(data_);

    const auto real_path = path.ToNativeString();
    const char *out_file = real_path.c_str();

    // Stifle annoying info messages.
    av_log_set_level(AV_LOG_FATAL);

    // Open the output context.
    avformat_alloc_output_context2(&data_->out_context, nullptr,
                                   "mp4", nullptr);
    if (! data_->out_context)
        Error_("Could not allocate output format context");

    // Find the encoder.
    data_->codec = avcodec_find_encoder_by_name("libx264rgb");
    if (! data_->codec)
        Error_("Codec 'libx264rgb' not found");

    // Allocate a packet for writing data.
    data_->packet = av_packet_alloc();

    // Create an output stream.
    data_->stream = avformat_new_stream(data_->out_context, nullptr);
    if (! data_->stream)
        Error_("Could not allocate output stream");
    data_->stream->id = 0;

    // Create a codec context.
    data_->codec_context = avcodec_alloc_context3(data_->codec);
    if (! data_->codec_context)
        Error_("Could not allocate video codec context");
    data_->codec_context->codec_id   = data_->out_context->oformat->video_codec;
    data_->codec_context->bit_rate   = 6000000;  // Produces good quality.
    data_->codec_context->width      = resolution[0];
    data_->codec_context->height     = resolution[1];
    data_->stream->time_base = { 1, fps};
    data_->codec_context->time_base  = data_->stream->time_base;
    // Emit one intra frame every twelve frames at most.
    data_->codec_context->gop_size   = 12;
    data_->codec_context->pix_fmt    = AV_PIX_FMT_RGB24;

    // Open the codec.
    if (avcodec_open2(data_->codec_context, data_->codec, nullptr) < 0)
        Error_("Could not open video codec");

    // Allocate a frame.
    data_->frame = av_frame_alloc();
    if (! data_->frame)
        Error_("Could not allocate video frame");
    data_->frame->format = data_->codec_context->pix_fmt;
    data_->frame->width  = data_->codec_context->width;
    data_->frame->height = data_->codec_context->height;
    if (av_frame_get_buffer(data_->frame, 0) < 0)
        Error_("Could not allocate frame data");

    // Copy the stream parameters to the muxer.
    if (avcodec_parameters_from_context(data_->stream->codecpar,
                                        data_->codec_context) < 0)
        Error_("Could not copy codec parameters to output stream");

    av_dump_format(data_->out_context, 0, out_file, 1);

    // Open the output file.
    if (avio_open(&data_->out_context->pb, out_file, AVIO_FLAG_WRITE) < 0)
        Error_(std::string("Could not open output file: ") + out_file);

    // Write the stream header.
    if (avformat_write_header(data_->out_context, nullptr) < 0)
        Error_("Could not write stream header");
}

void VideoWriter::AddImage(const ion::gfx::Image &image) {
    ASSERT(data_);
    if (av_frame_make_writable(data_->frame) < 0)
        Error_("Could not make the video frame data writable");

    // Copy the image data.
    ASSERT(image.GetDataSize() ==
           3U * data_->frame->width * data_->frame->height);
    std::memcpy(&data_->frame->data[0][0],
                image.GetData()->GetData(), image.GetDataSize());

    data_->frame->pts = data_->cur_frame;

    SendFrame_(data_->frame);

    ++data_->cur_frame;
}

void VideoWriter::WriteToFile() {
    ASSERT(data_);

    // Send a null frame to end the video stream.
    SendFrame_(nullptr);

    // Write output trailer.
    av_write_trailer(data_->out_context);

    // Clean up.
    avcodec_free_context(&data_->codec_context);
    av_frame_free(&data_->frame);
    av_packet_free(&data_->packet);
    avio_closep(&data_->out_context->pb);
    avformat_free_context(data_->out_context);

    // Make subsequent uses fail.
    data_.reset();
}

void VideoWriter::SendFrame_(AVFrame *frame) {
    // Send the frame to the encoder
    if (avcodec_send_frame(data_->codec_context, frame) < 0)
        Error_("Error: Could not send frame to output codec");

    int ret = 0;
    while (ret >= 0) {
        ret = avcodec_receive_packet(data_->codec_context, data_->packet);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            break;
        else if (ret < 0)
            Error_("Could not receive packet");

        // Rescale output packet timestamp values from codec to stream
        // timebase.
        av_packet_rescale_ts(data_->packet, data_->codec_context->time_base,
                             data_->stream->time_base);
        data_->packet->stream_index = data_->stream->index;

        if (av_interleaved_write_frame(data_->out_context, data_->packet) < 0)
            Error_("Could not write packet");
    }
}
