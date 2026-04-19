# Image Processor

C++ image processing engine that reads and writes real `.ppm` image files and applies filters using OOP design. No external libraries — pure C++17 standard library.

## Filters implemented

| Filter | Operation |
|---|---|
| **Grayscale** | Luminance formula: `0.299·R + 0.587·G + 0.114·B` |
| **Invert** | Negative: `255 - value` per channel |
| **Sepia** | Vintage tone via matrix transformation |
| **Sharpen** | 3×3 convolution kernel (same operation used in CNNs) |
| **Brightness** | Per-pixel offset with clamping to `[0, 255]` |

Filters can be chained — apply Grayscale + Sharpen + Brightness in sequence, in any order.

## OOP concepts demonstrated

- **Abstraction** — `ImageFilter` is an abstract base class with pure virtual `apply()`
- **Polymorphism** — `FilterPipeline` stores `ImageFilter*` pointers and applies them generically
- **Inheritance** — five derived filter classes extend `ImageFilter`
- **Encapsulation** — `Image` class hides pixel buffer access behind getters/setters
- **Composition** — `FilterPipeline` contains a vector of filters (has-a)

## The Sharpen filter — convolution

The sharpening filter uses this 3×3 kernel:

```
  0  -1   0
 -1   5  -1
  0  -1   0
```

For each pixel, the kernel is applied across its 3×3 neighborhood. The center pixel is amplified (×5) while the 4 direct neighbors are subtracted. This emphasizes regions where a pixel differs from its neighbors — i.e. edges — making the image appear sharper.

The same convolution operation is the building block of CNNs (Convolutional Neural Networks) used in modern computer vision.

## Build & run

```bash
mkdir build && cd build
cmake ..
cmake --build .
./ImageProcessor
```

Or open in CLion — it reads `CMakeLists.txt` automatically.

The program prompts for:
1. A `.ppm` image path (or Enter for a generated test image)
2. A list of filter numbers (e.g. `1 4` for Grayscale then Sharpen)

Output is saved as `output_filtered.ppm` in the working directory.

## PPM format

The `.ppm` (Portable PixMap) format is plain text — header followed by RGB triplets. Chosen because it can be read/written with the C++ standard library alone, no external image libraries needed.

To use real photos, convert your `.jpg`/`.png` to `.ppm` with [IrfanView](https://www.irfanview.com) or GIMP (Save As → format PPM ASCII / P3).

## Test files

- `test_input.ppm` — 16×16 image with 4 colored quadrants (red, green, blue, white)
- `expected_invert.ppm` — what the output should look like after applying the Invert filter

Use them to verify the program produces the correct output.

## Filters in action

![Filters comparison](screenshots/filters_comparison.png)

Same source image processed through each filter: Grayscale collapses RGB into luminance, Invert produces a photo negative, Sepia applies a warm vintage tone, and Sharpen accentuates edges via the 3x3 convolution kernel.
