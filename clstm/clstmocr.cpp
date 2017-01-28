//
//  clstmocr.cpp
//  clstm
//
//  Created by Jeffrey Rafter and Tyler Davis on 1/24/17.
//  Copyright © 2017 Jeffrey Rafter and Tyler Davis. All rights reserved.
//

#include "clstmocr.hpp"
#include "deskew.hpp"

#include "clstm.h"
#include "clstmhl.h"
#include <time.h>


// extern "C" will cause the C++ compiler
// (remember, this is still C++ code!) to
// compile the function in such a way that
// it can be called from C
// (and Swift).
extern "C" const char * clstm_recognize(const char * model_file_name, const char * image_file_name)
{

    ocropus::Tensor2 raw;

    ocropus::CLSTMOCR clstm;
    
    clock_t start, end;
    double cpu_load_time, cpu_png_time, cpu_predict_time;

    // Load the model
    start = clock();
    clstm.load(std::string(model_file_name));
    end = clock();
    cpu_load_time = ((double) (end - start)) / CLOCKS_PER_SEC;
    
    // Deskew and grab the center line
    deskew(image_file_name);
    
    // Read the image
    start = clock();
    read_png(raw, image_file_name);
    raw() = -raw() + ocropus::Float(1.0);
    end = clock();
    cpu_png_time = ((double) (end - start)) / CLOCKS_PER_SEC;

    
    // Convert to text
    start = clock();
    std::string out = clstm.predict_utf8(raw());
    end = clock();
    cpu_predict_time = ((double) (end - start)) / CLOCKS_PER_SEC;

    // If you want timing
    // out = out + "(load: " + std::to_string(cpu_load_time) + ", png: " + std::to_string(cpu_png_time) + ", predict: " + std::to_string(cpu_predict_time) + ")";

    char * ret = new char[out.length() + 1];
    std:strcpy(ret, out.c_str());
    
    return ret;
}
