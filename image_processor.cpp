// Image Processor - aplica filtre pe imagini .ppm
// proiect OOP - clase abstracte, mostenire, compozitie

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <sstream>

struct Pixel {
    unsigned char r, g, b;
};

class Image {
private:
    int width;
    int height;
    std::vector<std::vector<Pixel>> pixels;

public:
    Image(int w, int h)
        : width(w), height(h),
          pixels(h, std::vector<Pixel>(w, {0, 0, 0})) {}

    // citeste .ppm format P3 (ASCII)
    static Image loadPPM(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open())
            throw std::runtime_error("Nu pot deschide: " + filename);

        std::string magic;
        file >> magic;
        if (magic != "P3")
            throw std::runtime_error("Format necunoscut, trebuie PPM P3");

        // sar peste comentarii
        std::string line;
        std::getline(file, line);
        while (file.peek() == '#') std::getline(file, line);

        int w, h, maxVal;
        file >> w >> h >> maxVal;

        Image img(w, h);
        for (int y = 0; y < h; y++) {
            for (int x = 0; x < w; x++) {
                int r, g, b;
                file >> r >> g >> b;
                img.pixels[y][x] = {
                    (unsigned char)(r * 255 / maxVal),
                    (unsigned char)(g * 255 / maxVal),
                    (unsigned char)(b * 255 / maxVal)
                };
            }
        }
        return img;
    }

    void savePPM(const std::string& filename) const {
        std::ofstream file(filename);
        if (!file.is_open())
            throw std::runtime_error("Nu pot salva: " + filename);

        file << "P3\n";
        file << "# Output from Image Processor\n";
        file << width << " " << height << "\n";
        file << "255\n";

        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                const Pixel& p = pixels[y][x];
                file << (int)p.r << " " << (int)p.g << " " << (int)p.b;
                if (x < width - 1) file << "  ";
            }
            file << "\n";
        }
    }

    int getWidth()  const { return width; }
    int getHeight() const { return height; }

    Pixel getPixel(int x, int y) const { return pixels[y][x]; }
    void  setPixel(int x, int y, Pixel p) { pixels[y][x] = p; }

    // preview ASCII in consola (consola nu poate afisa imagini reale)
    void printPreview() const {
        int step = std::max(1, std::max(width / 60, height / 30));
        for (int y = 0; y < height; y += step * 2) {
            for (int x = 0; x < width; x += step) {
                Pixel p = pixels[y][x];
                int brightness = (p.r + p.g + p.b) / 3;
                char c = ' ';
                if (brightness > 200) c = '@';
                else if (brightness > 150) c = '#';
                else if (brightness > 100) c = '+';
                else if (brightness > 50)  c = '.';
                std::cout << c;
            }
            std::cout << "\n";
        }
    }
};

// clasa abstracta - orice filtru trebuie sa implementeze apply()
class ImageFilter {
public:
    virtual ~ImageFilter() = default;
    virtual Image apply(const Image& input) const = 0;
    virtual std::string getName() const = 0;
};

// alb/negru - formula luminanta (ochiul e mai sensibil la verde)
class GrayscaleFilter : public ImageFilter {
public:
    Image apply(const Image& input) const override {
        Image output(input.getWidth(), input.getHeight());
        for (int y = 0; y < input.getHeight(); y++) {
            for (int x = 0; x < input.getWidth(); x++) {
                Pixel p = input.getPixel(x, y);
                unsigned char gray = (unsigned char)(0.299 * p.r + 0.587 * p.g + 0.114 * p.b);
                output.setPixel(x, y, {gray, gray, gray});
            }
        }
        return output;
    }
    std::string getName() const override { return "Grayscale"; }
};

// inversare culori - negativ foto
class InvertFilter : public ImageFilter {
public:
    Image apply(const Image& input) const override {
        Image output(input.getWidth(), input.getHeight());
        for (int y = 0; y < input.getHeight(); y++) {
            for (int x = 0; x < input.getWidth(); x++) {
                Pixel p = input.getPixel(x, y);
                output.setPixel(x, y, {
                    (unsigned char)(255 - p.r),
                    (unsigned char)(255 - p.g),
                    (unsigned char)(255 - p.b)
                });
            }
        }
        return output;
    }
    std::string getName() const override { return "Invert"; }
};

// sepia - efect vintage
class SepiaFilter : public ImageFilter {
public:
    Image apply(const Image& input) const override {
        Image output(input.getWidth(), input.getHeight());
        for (int y = 0; y < input.getHeight(); y++) {
            for (int x = 0; x < input.getWidth(); x++) {
                Pixel p = input.getPixel(x, y);
                int nr = std::min(255, (int)(p.r * 0.393 + p.g * 0.769 + p.b * 0.189));
                int ng = std::min(255, (int)(p.r * 0.349 + p.g * 0.686 + p.b * 0.168));
                int nb = std::min(255, (int)(p.r * 0.272 + p.g * 0.534 + p.b * 0.131));
                output.setPixel(x, y, {
                    (unsigned char)nr,
                    (unsigned char)ng,
                    (unsigned char)nb
                });
            }
        }
        return output;
    }
    std::string getName() const override { return "Sepia"; }
};

// sharpen - convolutie 3x3, evidentiaza muchiile
// kernel:  0 -1  0
//         -1  5 -1
//          0 -1  0
class SharpenFilter : public ImageFilter {
private:
    int strength;

public:
    SharpenFilter(int s = 1) : strength(s) {}

    Image apply(const Image& input) const override {
        int center = 1 + 4 * strength;
        int side   = -strength;
        int kernel[3][3] = {
            {0,    side,    0},
            {side, center,  side},
            {0,    side,    0}
        };

        Image output(input.getWidth(), input.getHeight());

        // marginile raman neschimbate (nu au vecini completi)
        for (int y = 1; y < input.getHeight() - 1; y++) {
            for (int x = 1; x < input.getWidth() - 1; x++) {
                int r = 0, g = 0, b = 0;

                for (int ky = -1; ky <= 1; ky++) {
                    for (int kx = -1; kx <= 1; kx++) {
                        Pixel p = input.getPixel(x + kx, y + ky);
                        int w = kernel[ky + 1][kx + 1];
                        r += p.r * w;
                        g += p.g * w;
                        b += p.b * w;
                    }
                }

                // clamp in [0, 255]
                output.setPixel(x, y, {
                    (unsigned char)std::max(0, std::min(255, r)),
                    (unsigned char)std::max(0, std::min(255, g)),
                    (unsigned char)std::max(0, std::min(255, b))
                });
            }
        }
        return output;
    }
    std::string getName() const override {
        return "Sharpen(s=" + std::to_string(strength) + ")";
    }
};

class BrightnessFilter : public ImageFilter {
private:
    int amount;

public:
    BrightnessFilter(int amount) : amount(amount) {}

    Image apply(const Image& input) const override {
        Image output(input.getWidth(), input.getHeight());
        for (int y = 0; y < input.getHeight(); y++) {
            for (int x = 0; x < input.getWidth(); x++) {
                Pixel p = input.getPixel(x, y);
                output.setPixel(x, y, {
                    (unsigned char)std::max(0, std::min(255, (int)p.r + amount)),
                    (unsigned char)std::max(0, std::min(255, (int)p.g + amount)),
                    (unsigned char)std::max(0, std::min(255, (int)p.b + amount))
                });
            }
        }
        return output;
    }
    std::string getName() const override {
        return "Brightness(" + std::to_string(amount) + ")";
    }
};

// lant de filtre - aplica mai multe in ordine
class FilterPipeline {
private:
    std::vector<ImageFilter*> filters;

public:
    ~FilterPipeline() {
        for (ImageFilter* f : filters) delete f;
    }

    void add(ImageFilter* filter) {
        filters.push_back(filter);
    }

    Image apply(const Image& input) const {
        Image current = input;
        for (const ImageFilter* f : filters) {
            std::cout << "  Aplic: " << f->getName() << "...\n";
            current = f->apply(current);
        }
        return current;
    }
};

// genereaza o imagine de test daca nu ai .ppm la indemana
Image generateTestImage(int width = 200, int height = 150) {
    Image img(width, height);
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            unsigned char r = (unsigned char)(x * 255 / width);
            unsigned char g = (unsigned char)(y * 255 / height);
            unsigned char b = (unsigned char)(128 + 127 * std::sin(x * 0.1));
            img.setPixel(x, y, {r, g, b});
        }
    }
    return img;
}

int main() {
    std::cout << "=== Image Processor ===\n\n";

    Image img = generateTestImage();
    bool loadedFromFile = false;

    std::cout << "Calea catre .ppm (Enter pt imagine de test): ";
    std::string path;
    std::getline(std::cin, path);

    if (!path.empty()) {
        try {
            img = Image::loadPPM(path);
            loadedFromFile = true;
            std::cout << "Imagine incarcata: " << img.getWidth()
                      << "x" << img.getHeight() << " px\n\n";
        } catch (const std::exception& e) {
            std::cout << "Eroare: " << e.what() << "\n";
            std::cout << "Folosesc imagine de test.\n\n";
        }
    } else {
        std::cout << "Folosesc imagine de test (" << img.getWidth()
                  << "x" << img.getHeight() << ").\n\n";
    }

    std::cout << "Alege filtre (ex: 1 3 4):\n";
    std::cout << "  1. Grayscale\n";
    std::cout << "  2. Invert\n";
    std::cout << "  3. Sepia\n";
    std::cout << "  4. Sharpen x1\n";
    std::cout << "  5. Sharpen x2\n";
    std::cout << "  6. Brightness +50\n";
    std::cout << "  7. Brightness -50\n\n";
    std::cout << "Alegere: ";

    std::string choiceLine;
    std::getline(std::cin, choiceLine);
    std::istringstream ss(choiceLine);

    FilterPipeline pipeline;
    int choice;
    bool anyFilter = false;
    while (ss >> choice) {
        anyFilter = true;
        switch (choice) {
            case 1: pipeline.add(new GrayscaleFilter());      break;
            case 2: pipeline.add(new InvertFilter());         break;
            case 3: pipeline.add(new SepiaFilter());          break;
            case 4: pipeline.add(new SharpenFilter(1));       break;
            case 5: pipeline.add(new SharpenFilter(2));       break;
            case 6: pipeline.add(new BrightnessFilter(50));   break;
            case 7: pipeline.add(new BrightnessFilter(-50));  break;
            default: std::cout << "  Ignorat: " << choice << "\n";
        }
    }

    if (!anyFilter) {
        std::cout << "Nimic ales, fac un demo cu cateva filtre.\n";
        pipeline.add(new SharpenFilter(1));
        pipeline.add(new SepiaFilter());
        pipeline.add(new BrightnessFilter(20));
    }

    std::cout << "\nProcesez...\n";
    Image result = pipeline.apply(img);

    std::string outputPath = loadedFromFile ? "output_filtered.ppm" : "test_output.ppm";
    try {
        result.savePPM(outputPath);
        std::cout << "\nSalvat: " << outputPath << "\n";
        std::cout << "Deschide-l cu IrfanView sau GIMP.\n";
    } catch (const std::exception& e) {
        std::cout << "Eroare la salvare: " << e.what() << "\n";
    }

    std::cout << "\n--- Preview ASCII ---\n";
    result.printPreview();

    std::cout << "\nDONE\n";
    return 0;
}
