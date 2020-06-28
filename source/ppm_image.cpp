#include "ppm_image.hpp"
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>

PPMImage::PPMImage() { }
PPMImage::~PPMImage() {}

PPMImage::PPMImage(int w, int h)
    : width(w)
    , height(h)
{ }

PPMImage::PPMImage(std::vector<char> data, int w, int h)
    : pixel(data)
    , width(w)
    , height(h)
{ }

PPMImage::PPMImage(const PPMImage &other) :
    width(other.width)
    , height(other.height)
    , pixel(other.pixel)
{ }

PPMImage &PPMImage::operator=(const PPMImage &other)
{
    if(this != &other) {
        width = other.width;
        height = other.height;
        pixel = other.pixel;
    }

    return *this;
}

PPMImage PPMImage::load(const std::string &path)
{
    std::string header;
    int width, height, maxColor;
    std::ifstream in (path, std::ios::binary);

    if(in.fail()) {
        throw std::invalid_argument("[ppm]: failed to load");
    }

    in >> header;

    if(header != "P6") {
        throw std::invalid_argument("[ppm]: wrong format");
    }

while(true) {
        getline(in, header);

        if(header.empty()) {
            continue;
        }

        if(header [0] != '#') {
            break;
        }
    }

    std::stringstream prpps(header);
    prpps >> width >> height;
    in >> maxColor;

    if(maxColor != 255) {
        throw std::invalid_argument("[ppm]: wrong format");
    }
    
    std::string tmp;
    getline(in, tmp);
    std::vector<char> data(width * height * 3);
    in.read(reinterpret_cast<char *>(data.data()), data.size());
    in.close();
    PPMImage img(data, width, height);
    return img;
}

void PPMImage::save(const PPMImage &input, std::string path)
{
    std::ofstream out(path, std::ios::binary);

    if(out.fail()) {
        throw std::invalid_argument("[ppm]: failed to save");
    }

    out << "P6\n";
    out << input.width << " " << input.height << "\n";
    out << "255\n";
    out.write(input.pixel.data(), input.pixel.size());
    out.close();
}

PPMImage PPMImage::toRGB(const PPMImage &input)
{
    PPMImage result(input.width, input.height);

    for(std::size_t i = 0; i < input.pixel.size(); i += 3) {
        result.pixel.push_back(input.pixel [i + 0]);
        result.pixel.push_back(input.pixel [i + 1]);
        result.pixel.push_back(input.pixel [i + 2]);
    }

    return result;
}

int PPMImage::packData(unsigned int **packed)
{
    *packed = new unsigned int[pixel.size() / 3];

    for(int i = 0, j = 0; i < pixel.size(); i += 3, ++j) {
        int r = (int)pixel[i+0];
        int g = (int)pixel[i+1];
        int b = (int)pixel[i+2];
        (*packed)[j] = (( r & 0xffu) << 16) | (( g & 0xffu) << 8) | ( b & 0xffu);
    }

    return pixel.size() / 3;
}

void PPMImage::unpackData(unsigned int *packed, int size)
{
    pixel.reserve(size * 3);

    for(int i = 0; i < size; ++i) {
        int rgba = packed[i];
        int r = ((rgba >> 16) & 0xff);   
        int g = ((rgba >> 8)  & 0xff);   
        int b = (rgba & 0xff);           
        pixel.push_back((char)r);
        pixel.push_back((char)g);
        pixel.push_back((char)b);
    }
}

void PPMImage::clear()
{
    pixel.clear();
}