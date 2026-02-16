#ifndef _PPMIMAGEWRITER_H_
#define _PPMIMAGEWRITER_H_

#include "ImageWriter.h"
#include <fstream>
using namespace std;

class PPMImageWriter:public ImageWriter {
    public:
    void save(int *image,int width,int height,string filename) {
        ofstream out(filename);
        int i,j;

        if  (!out.is_open()) {
            throw std::invalid_argument("File could not be opened for writing!");
        }

        
        

        //write the word P3

        out << "P3" << endl;


        //now write the width and height

        out << width << " " << height << endl;

        //write 256 as  factor
        out << 256 << endl;

        
        for (i=0;i<height;i++)
        {
            for (j=0;j<width;j++)
            {
                out << image[3*((height-1-i)*width+j)] << " "
                    << image[3*((height-1-i)*width+j)+1] << " "
                    << image[3*((height-1-i)*width+j)+2] << endl;
            }
        }
        out.close();
    }
};
#endif