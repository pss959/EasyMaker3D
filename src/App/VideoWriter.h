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

    VideoWriter();
    ~VideoWriter();

    /// Returns the extension to use for the resulting video (with no dot).
    const Str & GetExtension() const { return extension_; }

    /// Initializes the VideoWriter, given the path to the file to write to,
    /// the video resolution, and the number of frames per second.
    void Init(const FilePath &path, const Vector2i &resolution, int fps);

    /// Adds an image to the video.
    void AddImage(const ion::gfx::Image &image);

    /// Adds a chapter tag with the given title at the current frame in the
    /// video.
    void AddChapterTag(const Str &title);

    /// Writes the resulting video to the path passed to the constructor.
    void WriteToFile();

  private:
    // Struct representing a chapter in the video.
    struct Chapter_;
    // This struct stores all the FFMPEG data needed for writing video files.
    struct Data_;

    const Str extension_; ///< Extension used for video files (no dot).

    std::unique_ptr<Data_> data_;

    /// Initializes FFMPEG for writing video files.
    void Init_(const FilePath &path, const Vector2i &resolution, int fps);

    /// Sends the given frame. If it is null, this finishes the video stream.
    void SendFrame_(AVFrame *frame);

    /// Stores chapter data in the appropriate place for output.
    void StoreChapters_();

    /// Processes an error, throwing an exception.
    void Error_(const Str &message) { throw Exception(message); }
};
