#include "App/VideoWriter.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>
#include <libswscale/swscale.h>
}

#include <chrono>
#include <cstring>
#include <format>
#include <fstream>
#include <ranges>
#include <vector>

#include "Util/Assert.h"
#include "Util/FilePath.h"

/// The VideoWriter::Chapter_ struct stores a title and starting frame for a
/// chapter.
struct VideoWriter::Chapter_ {
    Str    tag;                  ///< Unique tag for the chapter.
    Str    title;                ///< Title text.
    uint64 start_frame     = 0;  ///< Frame where chapter starts.
    uint64 end_frame       = 0;  ///< Frame where chapter ends.
    uint64 start_timestamp = 0;  ///< Timestamp corresponding to #start_frame.
    uint64 end_timestamp   = 0;  ///< Timestamp corresponding to #end_frame.
    Chapter_() {}
    Chapter_(const Str &tg, const Str &tt, uint64 f) :
        tag(tg), title(tt), start_frame(f) {}
};

/// The VideoWriter::Data_ struct stores all the FFMPEG data needed for writing
/// video files.
struct VideoWriter::Data_ {
    FilePath         path;
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

/// Calls a libav function and throws an exception with a useful error message
/// if it fails.
#define CHECK_(func)                                                \
    if (int ret = func; ret < 0) Error_(std::string(#func) + ": " + \
                                        ErrorToString_(ret))

VideoWriter::VideoWriter(Format format) {
    format_    = format;
    extension_ = format == Format::kWEBM ? "webm" : "mp4";
    data_.reset(new Data_);
}

VideoWriter::~VideoWriter() {}

static std::string ErrorToString_(int err) {
    char buf[1024];
    return std::string(av_make_error_string(buf, 1024, err));
};

void VideoWriter::Init(const FilePath &path,
                       const Vector2i &resolution, int fps) {
    ASSERT(data_);

    data_->path = path;
    const auto real_path = path.ToNativeString();

    // Stifle annoying info messages.
    av_log_set_level(AV_LOG_FATAL);

    InitOutputContext_();
    InitCodec_();
    InitPacket_();
    InitStream_(fps);
    InitCodecContext_(resolution[0], resolution[1], fps);
    if (format_ != Format::kRGBMP4)
        InitConversion_(resolution[0], resolution[1]);
    AllocFrame_();
    InitOutput_(real_path.c_str());
}

void VideoWriter::AddImage(const ion::gfx::Image &image) {
    ASSERT(data_);

    auto frame = data_->frame;
    ASSERT(frame);
    ASSERT(image.GetDataSize() == 3U * frame->width * frame->height);

    if (av_frame_make_writable(frame) < 0)
        Error_("Could not make the video frame data writable");

    if (format_ == Format::kRGBMP4) {
        // Copy the RGB image data.
        std::memcpy(&frame->data[0][0],
                    image.GetData()->GetData(), image.GetDataSize());
    }
    else {
        // Convert RGB24 => YUV420P. Use sws_scale() with identical source and
        // destination sizes.
        ASSERT(data_->sws_context);
        const uint8_t *slices[1]{
            reinterpret_cast<const uint8_t *>(image.GetData()->GetData()) };
        const int strides[1]{ 3 * frame->width };
        sws_scale(data_->sws_context, slices, strides, 0, frame->height,
                  frame->data, frame->linesize);
    }

    frame->pts = data_->cur_frame;

    SendFrame_(frame);

    ++data_->cur_frame;
}

void VideoWriter::AddChapter(const Str &tag, const Str &title) {
    data_->chapters.push_back(Chapter_(tag, title, data_->cur_frame));
}

size_t VideoWriter::GetImageCount() const {
    // cur_frame is incremented per image.
    return data_->cur_frame;
}

size_t VideoWriter::GetChapterCount() const {
    return data_->chapters.size();
}

void VideoWriter::WriteToFile() {
    ASSERT(data_);

    // Send a null frame to end the video stream.
    SendFrame_(nullptr);

    // Set up the chapters and write the chapter file. (Must be done before
    // writing the trailer.)
    if (! data_->chapters.empty()) {
        StoreChapters_();
        FilePath chapter_path = data_->path;
        chapter_path.ReplaceExtension(".vtt");
        if (! WriteChapterFile_(chapter_path))
            Error_("Unable to write chapter file: " + chapter_path);
    }

    // Write output trailer.
    CHECK_(av_write_trailer(data_->out_context));

    // Clean up.
    avcodec_free_context(&data_->codec_context);
    av_frame_free(&data_->frame);
    av_packet_free(&data_->packet);
    avio_closep(&data_->out_context->pb);
    avformat_free_context(data_->out_context);

    // Make subsequent uses fail.
    data_.reset();
}

void VideoWriter::InitOutputContext_() {
    avformat_alloc_output_context2(&data_->out_context, nullptr,
                                   extension_.c_str(), nullptr);
    if (! data_->out_context)
        Error_("Could not allocate output format context");
}

void VideoWriter::InitCodec_() {
    // Find the encoder.
    const std::string encoder =
        format_ == Format::kRGBMP4 ? "libx264rgb" : "libvpx-vp9";
    data_->codec = avcodec_find_encoder_by_name(encoder.c_str());
    if (! data_->codec)
        Error_(std::string("Codec '") + encoder + "' not found");
}

void VideoWriter::InitPacket_() {
    // Allocate a packet for writing data.
    data_->packet = av_packet_alloc();
}

void VideoWriter::InitStream_(int fps) {
    // Create an output stream.
    data_->stream = avformat_new_stream(data_->out_context, nullptr);
    if (! data_->stream)
        Error_("Could not allocate output stream");
    data_->stream->id         = 0;
    data_->stream->time_base  = { 1, fps};
}

void VideoWriter::InitCodecContext_(int width, int height, int fps) {
    auto cc = avcodec_alloc_context3(data_->codec);
    if (! cc)
        Error_("Could not allocate video codec context");

    cc->bit_rate  = 400000;
    cc->width     = width;
    cc->height    = height;
    cc->time_base = data_->stream->time_base;

    // Set a reasonable quality (constant rate factor).
    CHECK_(av_opt_set(cc->priv_data, "crf", "31", 0));

    // Choose a pixel format for the Format.
    cc->pix_fmt =
        format_ == Format::kRGBMP4 ? AV_PIX_FMT_RGB24 : AV_PIX_FMT_YUV420P;

    data_->codec_context = cc;

    // Open the codec now that there is a context.
    CHECK_(avcodec_open2(data_->codec_context, data_->codec, nullptr));
}

void VideoWriter::InitConversion_(int width, int height) {
    // Set up a context for RGB -> YUV conversion if necessary.
    ASSERT(format_ != Format::kRGBMP4);

    data_->sws_context = sws_getContext(width, height, AV_PIX_FMT_RGB24,
                                        width, height, AV_PIX_FMT_YUV420P,
                                        SWS_BICUBIC, nullptr, nullptr, nullptr);
    if (! data_->sws_context)
        Error_("Could not allocate sws context");
}

void VideoWriter::AllocFrame_() {
    data_->frame = av_frame_alloc();
    if (! data_->frame)
        Error_("Could not allocate video frame");

    data_->frame->format = data_->codec_context->pix_fmt;
    data_->frame->width  = data_->codec_context->width;
    data_->frame->height = data_->codec_context->height;

    CHECK_(av_frame_get_buffer(data_->frame, 0));
}

void VideoWriter::InitOutput_(const char *out_file) {
    // Copy the stream parameters to the muxer.
    CHECK_(avcodec_parameters_from_context(data_->stream->codecpar,
                                           data_->codec_context));

    av_dump_format(data_->out_context, 0, out_file, 1);

    // Open the output file.
    CHECK_(avio_open(&data_->out_context->pb, out_file, AVIO_FLAG_WRITE));

    // Write the stream header.
    CHECK_(avformat_write_header(data_->out_context, nullptr));
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

    // Store end frames.
    const size_t count = data_->chapters.size();
    for (size_t i = 1; i < count; ++i)
        data_->chapters[i - 1].end_frame = data_->chapters[i].start_frame;
    data_->chapters.back().end_frame = data_->cur_frame;

    // Compute start/end timestamps.
    auto to_timestamp = [&](uint64 frame){
        return av_rescale_q(frame, data_->codec_context->time_base,
                            data_->stream->time_base);
    };
    for (auto &ch: data_->chapters) {
        ch.start_timestamp = to_timestamp(ch.start_frame);
        ch.end_timestamp   = to_timestamp(ch.end_frame);
    }

    // Store AVChapter instances in the AVFormatContext.
    auto oc = data_->out_context;
    oc->chapters = reinterpret_cast<AVChapter **>(
        av_realloc_f(oc->chapters, count, sizeof(*oc->chapters)));
    oc->nb_chapters = count;
    for (const auto [index, ch]: std::views::enumerate(data_->chapters)) {
        AVChapter *avch =
            reinterpret_cast<AVChapter *>(av_mallocz(sizeof(AVChapter)));
        avch->id        = index + 1;
        avch->time_base = data_->stream->time_base;
        avch->start     = ch.start_timestamp;
        avch->end       = ch.end_timestamp;

        // Store title in metadata.
        CHECK_(av_dict_set(&avch->metadata, "title", ch.title.c_str(), 0));

        oc->chapters[index] = avch;
    }
}

bool VideoWriter::WriteChapterFile_(const FilePath &path) {
    ASSERT(! data_->chapters.empty());

    std::ofstream out(path.ToNativeString());
    if (! out)
        return false;

    out << "WEBVTT\n\n";

    auto format_ts = [&](uint64 ts){
        // Convert timestamp to seconds.
        auto seconds = ts * av_q2d(data_->stream->time_base);
        // Convert to a duration in milliseconds.
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::duration<double>(seconds));
        // Format as a string.
        return std::format("{:%T}", ms);
    };

    for (const auto &ch: data_->chapters) {
        out << ch.tag << "\n"
            << format_ts(ch.start_timestamp) << " --> "
            << format_ts(ch.end_timestamp) << "\n"
            << ch.title << "\n\n";
    }

    return true;
}

#undef CHECK_
