#pragma once

#include <vector>

#include <ion/gfx/image.h>

#include "Math/Types.h"
#include "Util/ExceptionBase.h"

class FilePath;
struct AVCodecContext;
struct AVFrame;
struct AVPacket;

/// The VideoWriter class can be used to incrementally add frames of a video
/// and write the result to a file.
///
/// \ingroup App
class VideoWriter {
  public:
    /// Exception thrown when anything goes wrong.
    class Exception : public ExceptionBase {
      public:
        Exception(const Str &msg) : ExceptionBase(msg) {}
    };

    /// The constructor is passed the video resolution and frames per second.
    VideoWriter(const Vector2i &resolution, int fps);

    /// Adds an image to the video.
    void AddImage(const ion::gfx::Image &image);

    /// Writes the resulting video to the given path.
    void WriteToFile(const FilePath &path);

  private:
    AVCodecContext *context_ = nullptr;
    AVFrame        *frame_   = nullptr;
    AVPacket       *packet_  = nullptr;

    /// Stores the current video data.
    std::vector<char>  data_;

    /// Current image count, used for timestamp in frame.
    uint64             frame_count_ = 0;

    /// Encodes the current frame, adding to the #data_ vector. If \p end_frame
    /// is true, this finishes up encoding instead of adding the frame.
    void EncodeFrame_(bool end_frame);

    /// Processes an error, throwing an exception.
    void Error_(const Str &message) { throw Exception(message); }
};
