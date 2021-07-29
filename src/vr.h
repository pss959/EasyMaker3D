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
    void CreateSession(const GFX &gfx);
    bool PollEvents();  // Returns false if program should exit.
    void Draw(const GFX &gfx);

  private:
    class Helper_;
    std::unique_ptr<Helper_> helper_;
};
