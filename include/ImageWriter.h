#ifndef _IMAGEWRITER_H_
#define _IMAGEWRITER_H_

class ImageWriter {
    public:
    virtual void save(int *image,int width, int height,string filename) = 0;
};

#endif