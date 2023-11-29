#include "App/VideoWriter.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>
}

#include <cstring>
#include <fstream>

#include "Util/Assert.h"
#include "Util/FilePath.h"

VideoWriter::VideoWriter(const Vector2i &resolution, int fps) {
    // Stifle annoying info messages.
    av_log_set_level(AV_LOG_FATAL);

    const auto codec = avcodec_find_encoder_by_name("libx264rgb");
    if (! codec)
        Error_("Codec not found");

    context_ = avcodec_alloc_context3(codec);
    if (! context_)
        Error_("Could not allocate video codec context");

    packet_ = av_packet_alloc();
    if (! packet_)
        Error_("Could not allocate packet");

    context_->bit_rate     = 400000;
    context_->width        = resolution[0];
    context_->height       = resolution[1];
    context_->time_base    = AVRational{1,   fps};
    context_->framerate    = AVRational{fps, 1};
    context_->pix_fmt      = AV_PIX_FMT_RGB24;

    av_opt_set(context_->priv_data, "preset", "slow", 0);

    if (avcodec_open2(context_, codec, nullptr) < 0)
        Error_("Could not open codec");

    frame_ = av_frame_alloc();
    if (! frame_)
        Error_("Could not allocate video frame");

    frame_->format = context_->pix_fmt;
    frame_->width  = context_->width;
    frame_->height = context_->height;

    if (av_frame_get_buffer(frame_, 0) < 0)
        Error_("Could not allocate the video frame data");
}

void VideoWriter::AddImage(const ion::gfx::Image &image) {
    if (av_frame_make_writable(frame_) < 0)
        Error_("Could not make the video frame data writable");

    // "Presentation timestamp".
    frame_->pts = frame_count_++;

    // Copy the image data.
    ASSERT(image.GetDataSize() == 3U * frame_->width * frame_->height);
    std::memcpy(&frame_->data[0][0],
                image.GetData()->GetData(), image.GetDataSize());

    EncodeFrame_(false);
}


void VideoWriter::WriteToFile(const FilePath &path) {
    EncodeFrame_(true);
    std::ofstream out(path.ToNativeString(), std::ios::binary);
    if (! out)
        Error_("Could not open '" + path.ToString() + "' for writing");
    out.write(&data_[0], data_.size());
}

void VideoWriter::EncodeFrame_(bool end_frame) {
    if (avcodec_send_frame(context_, end_frame ? nullptr : frame_) < 0)
        Error_("Error sending a frame for encoding");

    while (true) {
        const auto ret = avcodec_receive_packet(context_, packet_);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            break;
        if (ret < 0)
            Error_("Error during encoding");
        data_.insert(data_.end(), packet_->data, packet_->data + packet_->size);
        av_packet_unref(packet_);
    }
}
