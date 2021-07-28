#pragma once

#include <memory>

class GFX;

class VR {
  public:
    VR();
    ~VR();
    bool Init();
    int GetHeight();
    int GetWidth();
    bool CreateSession(const GFX &gfx);

  private:
    class Helper_;
    std::unique_ptr<Helper_> helper_;
};
