#include "App/VideoWriter.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>
#include <libswscale/swscale.h>
}

#include <cstring>
#include <fstream>
#include <ranges>
#include <vector>

#include "Util/Assert.h"
#include "Util/FilePath.h"

/// The VideoWriter::Chapter_ struct stores a title and frame for a chapter.
struct VideoWriter::Chapter_ {
    Str    title;
    uint64 frame = 0;
    Chapter_() {}
    Chapter_(const Str &t, uint64 f) : title(t), frame(f) {}
};

/// The VideoWriter::Data_ struct stores all the FFMPEG data needed for writing
/// video files.
struct VideoWriter::Data_ {
    AVCodecContext  *codec_context = nullptr;
    AVFormatContext *out_context   = nullptr;
    const AVCodec   *codec         = nullptr;
    AVFrame         *frame         = nullptr;
    AVPacket        *packet        = nullptr;
    AVStream        *stream        = nullptr;
    SwsContext      *sws_context   = nullptr;
    uint64           cur_frame     = 0;

    /// Chapter tags.
    std::vector<VideoWriter::Chapter_> chapters;
};

VideoWriter::VideoWriter() : extension_("webm"), data_(new Data_) {}
VideoWriter::~VideoWriter() {}

static std::string ErrorToString_(int err) {
    char buf[1024];
    return std::string(av_make_error_string(buf, 1024, err));
};

void VideoWriter::Init(const FilePath &path,
                       const Vector2i &resolution, int fps) {
    ASSERT(data_);

    const auto real_path = path.ToNativeString();
    const char *out_file = real_path.c_str();

    // Stifle annoying info messages.
    av_log_set_level(AV_LOG_FATAL);

    // Open the output context.
    avformat_alloc_output_context2(&data_->out_context, nullptr,
                                   extension_.c_str(), nullptr);
    if (! data_->out_context)
        Error_("Could not allocate output format context");

    // Find the encoder.
    data_->codec = avcodec_find_encoder(AV_CODEC_ID_VP9);
    if (! data_->codec)
        Error_("Codec not found");

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
    data_->codec_context->width        = resolution[0];
    data_->codec_context->height       = resolution[1];
    data_->stream->time_base           = { 1, fps};
    data_->codec_context->time_base    = data_->stream->time_base;

    // Set a reasonable quality (constant rate factor).
    av_opt_set(data_->codec_context->priv_data, "crf", "31", 0);

    // Emit one intra frame every twelve frames at most.
    data_->codec_context->gop_size     = 12;
    data_->codec_context->pix_fmt      = AV_PIX_FMT_YUV420P;
    data_->codec_context->max_b_frames = 3;
    data_->codec_context->refs         = 3;

    // Open the codec.
    auto ret = avcodec_open2(data_->codec_context, data_->codec, nullptr);
    //if (avcodec_open2(data_->codec_context, data_->codec, nullptr) < 0)
    if (ret < 0)
        Error_("Could not open video codec " + ErrorToString_(ret));

    // Set up a context for RGB -> YUV conversion.
    data_->sws_context = sws_getContext(resolution[0], resolution[1],
                                        AV_PIX_FMT_RGB24,
                                        resolution[0], resolution[1],
                                        AV_PIX_FMT_YUV420P,
                                        0, nullptr, nullptr, nullptr);
    if (! data_->sws_context)
        Error_("Could not allocate sws context");

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
    ASSERT(image.GetDataSize() ==
           3U * data_->frame->width * data_->frame->height);

    if (av_frame_make_writable(data_->frame) < 0)
        Error_("Could not make the video frame data writable");

    // Convert RGB24 => YUV420P. Use sws_scale() with identical source and
    // destination sizes.
    const uint8_t *slices[1]{
        reinterpret_cast<const uint8_t *>(image.GetData()->GetData()) };
    const int strides[1]{ 3 * data_->frame->width };
    sws_scale(data_->sws_context, slices, strides, 0, data_->frame->height,
              data_->frame->data, data_->frame->linesize);

    data_->frame->pts = data_->cur_frame;

    SendFrame_(data_->frame);

    ++data_->cur_frame;
}

void VideoWriter::AddChapterTag(const Str &title) {
    std::cerr << "XXXX Adding tag '" << title
              << "' at frame " << data_->cur_frame << "\n";
    data_->chapters.push_back(Chapter_(title, data_->cur_frame));
}

void VideoWriter::WriteToFile() {
    ASSERT(data_);

    // Send a null frame to end the video stream.
    SendFrame_(nullptr);

    // Set up the chapters. (Must be done before writing the trailer.)
    if (! data_->chapters.empty())
        StoreChapters_();

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

void VideoWriter::StoreChapters_() {
    ASSERT(! data_->chapters.empty());

    const size_t count = data_->chapters.size();

    auto oc = data_->out_context;
    oc->chapters = reinterpret_cast<AVChapter **>(
        av_realloc_f(oc->chapters, count, sizeof(*oc->chapters)));
    oc->nb_chapters = count;

    auto convert_frame = [&](uint64 frame){
        return frame / av_q2d(data_->codec_context->time_base);
    };

    for (const auto [index, ch]: std::views::enumerate(data_->chapters)) {
        const uint64 end_frame = static_cast<size_t>(index + 1) < count ?
            data_->chapters[index + 1].frame : data_->cur_frame;

        AVChapter *avch =
            reinterpret_cast<AVChapter *>(av_mallocz(sizeof(AVChapter)));
        avch->id        = index;
        avch->time_base = data_->stream->time_base;
        avch->start     = convert_frame(ch.frame);
        avch->end       = convert_frame(end_frame);

        // Store title in metadata.
        av_dict_set(&avch->metadata, "title", ch.title.c_str(), 0);

        oc->chapters[index] = avch;
    }
}
