#pragma once

#include <memory>

class GFX {
  public:
    GFX(int width, int height);
    ~GFX();
    void Draw();

  private:
    class Helper_;
    std::unique_ptr<Helper_> helper_;
};
