#pragma once

#include <string>
#include <memory>

class GFX;

class VR {
  public:
    // Exception thrown when any OpenXR function fails.
    class VRException : public std::exception {
      public:
        VRException(const std::string &msg) : msg_(msg) {}
        const char * what() const throw() override { return msg_.c_str(); }
      private:
        std::string msg_;
    };

    VR();
    ~VR();
    void Init();
    int GetHeight();
    int GetWidth();

    void InitDraw(const std::shared_ptr<GFX> &gfx);
    void Draw();

    bool PollEvents();  // Returns false if program should exit.

  private:
    class Helper_;
    std::unique_ptr<Helper_> helper_;
};
