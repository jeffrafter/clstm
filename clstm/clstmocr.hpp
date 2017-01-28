//
//  clstmocr.hpp
//  clstm
//
//  Created by Jeffrey Rafter and Tyler Davis on 1/24/17.
//  Copyright © 2017 Jeffrey Rafter and Tyler Davis. All rights reserved.
//

#ifndef clstmocr_hpp
#define clstmocr_hpp

#include <stdio.h>

extern "C" const char * clstm_recognize(const char * model_file_name, const char * image_file_name);

#endif /* clstmocr_hpp */
