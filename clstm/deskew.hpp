//
//  deskew.hpp
//  clstm
//
//  Created by Jeffrey Rafter on 1/26/17.
//  Copyright © 2017 Jeffrey Rafter. All rights reserved.
//

#ifndef deskew_hpp
#define deskew_hpp

#include <stdio.h>

// Deskews the image specified by filename,
// extracts the first segment, then overwrites the file
void preProcessMiddleSegment(const char * input_filename, const char * output_filename);

#endif /* deskew_hpp */
