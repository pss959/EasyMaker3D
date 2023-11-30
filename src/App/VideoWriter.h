#pragma once

#include <memory>

#include <ion/gfx/image.h>

#include "Math/Types.h"
#include "Util/ExceptionBase.h"

class FilePath;
struct AVFrame;

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

    /// The constructor is passed the path to the file to write to, the video
    /// resolution, and the number of frames per second.
    VideoWriter(const FilePath &path, const Vector2i &resolution, int fps);
    ~VideoWriter();

    /// Adds an image to the video.
    void AddImage(const ion::gfx::Image &image);

    /// Writes the resulting video to the path passed to the constructor.
    void WriteToFile();

  private:
    // This struct stores all the FFMPEG data needed for writing video files.
    struct Data_;

    std::unique_ptr<Data_> data_;

    /// Initializes FFMPEG for writing video files.
    void Init_(const FilePath &path, const Vector2i &resolution, int fps);

    /// Sends the given frame. If it is null, this finishes the video stream.
    void SendFrame_(AVFrame *frame);

    /// Processes an error, throwing an exception.
    void Error_(const Str &message) { throw Exception(message); }
};
