#pragma once

#include <memory>

class VR {
  public:
    VR();
    ~VR();
    bool Init();
    int GetHeight();
    int GetWidth();

  private:
    class Helper_;
    std::unique_ptr<Helper_> helper_;
};
