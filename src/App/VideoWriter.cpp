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
    Str    title;
    size_t index           = 0;  ///< Index in chapter menu (starts at 1).
    uint64 start_frame     = 0;  ///< Frame where chapter starts.
    uint64 end_frame       = 0;  ///< Frame where chapter ends.
    uint64 start_timestamp = 0;  ///< Timestamp corresponding to #start_frame.
    uint64 end_timestamp   = 0;  ///< Timestamp corresponding to #end_frame.
    Chapter_() {}
    Chapter_(const Str &t, uint32 i, uint64 f) :
        title(t), index(i), start_frame(f) {}
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
    const char *out_file = real_path.c_str();

    // Stifle annoying info messages.
    av_log_set_level(AV_LOG_FATAL);

    // Open the output context.
    avformat_alloc_output_context2(&data_->out_context, nullptr,
                                   extension_.c_str(), nullptr);
    if (! data_->out_context)
        Error_("Could not allocate output format context");

    // Find the encoder.
    if (format_ == Format::kWEBM)
        data_->codec = avcodec_find_encoder(AV_CODEC_ID_VP9);
    else
        data_->codec = avcodec_find_encoder_by_name("libx264rgb");
    if (! data_->codec)
        Error_("Codec not found");

    // Allocate a packet for writing data.
    data_->packet = av_packet_alloc();

    // Create an output stream.
    data_->stream = avformat_new_stream(data_->out_context, nullptr);
    if (! data_->stream)
        Error_("Could not allocate output stream");
    data_->stream->id         = 0;
    data_->stream->time_base  = { 1, fps};

    // Create a codec context.
    InitCodecContext_(resolution, fps);
    ASSERT(data_->codec_context);

    // Open the codec.
    auto ret = avcodec_open2(data_->codec_context, data_->codec, nullptr);
    if (ret < 0)
        Error_("Could not open video codec: " + ErrorToString_(ret));

    // Set up a context for RGB -> YUV conversion if necessary.
    if (format_ == Format::kWEBM) {
        data_->sws_context = sws_getContext(resolution[0], resolution[1],
                                            AV_PIX_FMT_RGB24,
                                            resolution[0], resolution[1],
                                            AV_PIX_FMT_YUV420P,
                                            0, nullptr, nullptr, nullptr);
        if (! data_->sws_context)
            Error_("Could not allocate sws context");
    }

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

    auto frame = data_->frame;
    ASSERT(frame);
    ASSERT(image.GetDataSize() == 3U * frame->width * frame->height);

    if (av_frame_make_writable(frame) < 0)
        Error_("Could not make the video frame data writable");

    if (format_ == Format::kMP4) {
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

void VideoWriter::AddChapterTag(const Str &title) {
    const size_t index = data_->chapters.size() + 1;  // Start at 1.
    data_->chapters.push_back(Chapter_(title, index, data_->cur_frame));
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

void VideoWriter::InitCodecContext_(const Vector2i &resolution, int fps) {
    auto cc = avcodec_alloc_context3(data_->codec);
    if (! cc)
        Error_("Could not allocate video codec context");

    cc->codec_id  = data_->out_context->oformat->video_codec;
    cc->bit_rate  = 400000;
    cc->width     = resolution[0];
    cc->height    = resolution[1];
    cc->time_base = data_->stream->time_base;

    // Set a reasonable quality (constant rate factor).
    av_opt_set(cc->priv_data, "crf", "31", 0);

    // Choose a pixel format for the Format.
    cc->pix_fmt = format_ == Format::kWEBM ?
        AV_PIX_FMT_YUV420P : AV_PIX_FMT_RGB24;

    data_->codec_context = cc;
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
    for (const auto &ch: data_->chapters) {
        AVChapter *avch =
            reinterpret_cast<AVChapter *>(av_mallocz(sizeof(AVChapter)));
        avch->id        = ch.index;
        avch->time_base = data_->stream->time_base;
        avch->start     = ch.start_timestamp;
        avch->end       = ch.end_timestamp;

        // Store title in metadata.
        av_dict_set(&avch->metadata, "title", ch.title.c_str(), 0);

        oc->chapters[ch.index - 1] = avch;
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

    for (const auto [index, ch]: std::views::enumerate(data_->chapters)) {
        out << (index + 1) << "\n"
            << format_ts(ch.start_timestamp) << " --> "
            << format_ts(ch.end_timestamp) << "\n"
            << ch.title << "\n\n";
    }
#if XXXX
    const size_t count = data_->chapters.size();
        print_time("start_time", chapter->start, &chapter->time_base);

    auto convert_frame = [&](uint64 frame){
        return av_rescale_q(frame, data_->codec_context->time_base,
                            data_->stream->time_base);
    };

    for (const auto [index, ch]: std::views::enumerate(data_->chapters)) {
        const uint64 end_frame = static_cast<size_t>(index + 1) < count ?
            data_->chapters[index + 1].start_frame : data_->cur_frame;

        AVChapter *avch =
            reinterpret_cast<AVChapter *>(av_mallocz(sizeof(AVChapter)));
        avch->id        = index;
        avch->time_base = data_->stream->time_base;
        avch->start     = convert_frame(ch.start_frame);
        avch->end       = convert_frame(end_frame);

        // Store title in metadata.
        av_dict_set(&avch->metadata, "title", ch.title.c_str(), 0);

        oc->chapters[index] = avch;
    }
#endif

    return true;
}
