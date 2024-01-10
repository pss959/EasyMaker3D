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
    /// Available video formats.
    enum class Format {
        /// MP4 file containing RGB images. This is the fastest to create (no
        /// conversion necessary), but most browsers cannot display this.
        kRGBMP4,
        /// WEBM file. Most browsers can display this, but conversion is slow.
        kWEBM,
        /// MP4 file containing YUV420p images. This can be handled by most
        /// browsers, but the conversion is as slow as for kWEBM.
        kYUVMP4,
    };

    /// Exception thrown when anything goes wrong.
    class Exception : public ExceptionBase {
      public:
        Exception(const Str &message) : ExceptionBase(message) {}
    };

    /// The constructor is passed the video format to use.
    explicit VideoWriter(Format format);
    ~VideoWriter();

    /// Returns the format passed to the constructor.
    Format GetFormat() const { return format_; }

    /// Returns the extension to use for the resulting video (with no dot).
    const Str & GetExtension() const { return extension_; }

    /// Initializes the VideoWriter, given the path to the file to write to,
    /// the video resolution, and the number of frames per second.
    void Init(const FilePath &path, const Vector2ui &resolution, int fps);

    /// Adds an image to the video.
    void AddImage(const ion::gfx::Image &image);

    /// Adds a chapter with the given tag and title at the current frame in the
    /// video.
    void AddChapter(const Str &tag, const Str &title);

    /// Returns the number of images added.
    size_t GetImageCount() const;

    /// Returns the current number of chapters.
    size_t GetChapterCount() const;

    /// Writes the resulting video to the path passed to Init() and writes a
    /// WebVTT chapter file to the same path with a ".vtt" extension.
    void WriteToFile();

  private:
    // Struct representing a chapter in the video.
    struct Chapter_;
    // This struct stores all the FFMPEG data needed for writing video files.
    struct Data_;

    Format format_;     ///< Format passed to the constructor.
    Str    extension_;  ///< Extension used for video files (no dot).

    std::unique_ptr<Data_> data_;

    /// \name Initialization and setup functions
    ///@{
    void InitOutputContext_();
    void InitCodec_();
    void InitPacket_();
    void InitStream_(int fps);
    void InitCodecContext_(int width, int height, int fps);
    void InitConversion_(int width, int height);
    void AllocFrame_();
    void InitOutput_(const char *out_file);
    ///@}

    /// Sends the given frame. If it is null, this finishes the video stream.
    void SendFrame_(AVFrame *frame);

    /// Stores chapter data in the appropriate place for output.
    void StoreChapters_();

    /// Writes a WebVTT chapter file to the given path. Returns false on
    /// failure to open the file.
    bool WriteChapterFile_(const FilePath &path);

    /// Processes an error, throwing an exception.
    void Error_(const Str &message) { throw Exception(message); }
};
