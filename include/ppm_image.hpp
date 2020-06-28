/*!
  \file
  \brief PPM image io
  \author Ilya Shoshin (Galarius)
  \copyright (c) 2016, Research Institute of Instrument Engineering
*/

#ifndef __PPM_IMAGE__
#define __PPM_IMAGE__

#include <vector>
#include <string>

class PPMImage
{
public:
    PPMImage();
    ~PPMImage();
    PPMImage(int w, int h);
    PPMImage(std::vector<char> data, int w, int h);
    PPMImage(const PPMImage &other);
    PPMImage &operator=(const PPMImage &other);
public:
    /*!
     * \throws std::invalid_argument
     */
    static PPMImage load(const std::string &path);
    static void save(const PPMImage &input, std::string path);
    static PPMImage toRGB(const PPMImage &input);
    int packData(unsigned int **packed);
    void unpackData(unsigned int *packed, int size);
    void clear();
public:
    std::vector<char> pixel;
    int width, height;
};

#endif // __PPM_IMAGE__