#include "utils.h"
#include <string>
#include <cctype>
#include <vector>
#include <numbers>
#include <algorithm>
#include <fstream>
//#include <libpng16/png.h>

// https://github.com/facebook/folly/blob/main/folly/Range.h#L973
bool starts_with(const std::string& str, const std::string& start) {
    return str.size() >= start.size() && str.substr(0, start.size()) == start;
}

// https://github.com/facebook/folly/blob/main/folly/Range.h#L1006
bool ends_with(const std::string& str, const std::string& end) {
    return str.size() >= end.size() && str.substr(str.size() - end.size()) == end;
}

std::string read_file(const std::string& filename)
{
    std::ifstream fs(filename, std::ifstream::binary);
    std::vector<char> raw_data;
    if (fs) {
        int f_len;
        fs.seekg(0, fs.end);
        f_len = static_cast<int>(fs.tellg());
        fs.seekg(0, fs.beg);

        raw_data.resize(f_len);
        fs.read(raw_data.data(), f_len);
    }
    return std::string(raw_data.begin(), raw_data.end());
}


// https://gist.github.com/niw/5963798
//std::vector<unsigned char> read_png_file(const char* filename, int& width, int& height, int& bit_depth)
//{
//    FILE* fp = fopen(filename, "rb");
//
//    png_structp png =
//        png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
//    if (!png) abort();
//
//    png_infop info = png_create_info_struct(png);
//    if (!info) abort();
//
//    if (setjmp(png_jmpbuf(png))) abort();
//
//    png_init_io(png, fp);
//
//    png_read_info(png, info);
//
//    width = png_get_image_width(png, info);
//    height = png_get_image_height(png, info);
//    bit_depth = png_get_bit_depth(png, info);
//
//    png_byte color_type = png_get_color_type(png, info);
//
//    // Read any color_type into 8bit depth, RGBA format.
//    // See http://www.libpng.org/pub/png/libpng-manual.txt
//
//    if (bit_depth == 16) png_set_strip_16(png);
//
//    if (color_type == PNG_COLOR_TYPE_PALETTE) png_set_palette_to_rgb(png);
//
//    // PNG_COLOR_TYPE_GRAY_ALPHA is always 8 or 16bit depth.
//    if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
//        png_set_expand_gray_1_2_4_to_8(png);
//
//    if (png_get_valid(png, info, PNG_INFO_tRNS)) {
//        png_set_tRNS_to_alpha(png);
//    }
//
//    // These color_type don't have an alpha channel then fill it with 0xff.
//    if (color_type == PNG_COLOR_TYPE_RGB || color_type == PNG_COLOR_TYPE_GRAY ||
//        color_type == PNG_COLOR_TYPE_PALETTE)
//        png_set_filler(png, 0xFF, PNG_FILLER_AFTER);
//
//    if (color_type == PNG_COLOR_TYPE_GRAY ||
//        color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
//        png_set_gray_to_rgb(png);
//
//    png_read_update_info(png, info);
//
//    //if (row_pointers) abort();
//    std::vector<unsigned char> out;
//    png_bytep* row_pointers = (png_bytep*)malloc(sizeof(png_bytep) * height);
//    for (int y = 0; y < height; y++) {
//        int nb = png_get_rowbytes(png, info);
//        row_pointers[y] = (png_byte*)malloc(png_get_rowbytes(png, info));
//    }
//
//    png_read_image(png, row_pointers);
//
//    for (int y = 0; y < height; y++) {
//        //std::cout << y << std::endl;
//        for (int x = 0; x < width; ++x) {
//            auto r = row_pointers[y][4 * x + 0];
//            auto g = row_pointers[y][4 * x + 1];
//            auto b = row_pointers[y][4 * x + 2];
//            auto a = row_pointers[y][4 * x + 3];
//            out.push_back(r);
//            out.push_back(g);
//            out.push_back(b);
//            //out.push_back(a);
//        }
//
//        for (int x = width * 3; x < (width * 3 + 3) / 4 * 4; ++x)
//        {
//            out.push_back(0);
//        }
//    }
//
//    fclose(fp);
//
//    png_destroy_read_struct(&png, &info, NULL);
//    return out;
//}
