#pragma once
#include <string>

class Texture {
public:
    unsigned int id = 0;
    int width = 0, height = 0, channels = 0;

    Texture() = default;
    ~Texture();
    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;
    Texture(Texture&& o) noexcept;
    Texture& operator=(Texture&& o) noexcept;

    // Laduje PNG/JPG/BMP z dysku. Zwraca false przy bledzie.
    bool load(const std::string& path);

    // Binduje na podany unit tekstury (domyslnie 0)
    void bind(int unit = 0) const;

    bool isLoaded() const { return id != 0; }
};