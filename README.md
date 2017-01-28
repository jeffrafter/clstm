# clstm


# Making the project

Putting this together took a lot of trial-and-error. Here are the steps... just in case.

1. Create a static cocoa touch lib (iOS)
2. Add all of the `*.cc` and `*.h` files from clstm
3. Add the `clstm.proto` from clstm
4. Add in the headers by adding `Eigen` and `unsupported` folders into the project (these came from my `brew` installation at `/usr/local/include/eigen3/`)
5. Add `$(PROJECT_ROOT)` to the header search paths (so it can find those Eigen headers)
6. Add `macros.h`

    ```cpp
    #ifndef macros_h
    #define macros_h

    #define THROW throw
    #define TRY try
    #define CATCH catch
    #define NODISPLAY 1
    #define CLSTM_ALL_TENSOR 1

    #endif /* macros_h */
    ```
7. Set a return value for `print_usage` in `clstmocr.cc` and `clstmtrain.cc` 
    
    ```cpp
    return 0;
    ```
        
8. include `macros.h` in all of the failing files (`pstring.h`, `tensor.h`, `utils.h`)
9. In `extras.cc` you need to change:

    ```cpp
    #include <png.h>
    ```
    
    to
    
    ```cpp
    #include "png.h"
    ```

10. Include libpng via cocoapods, and in pngpriv.h comment out:
    
    ```cpp
    // #ifndef PNG_ARM_NEON_OPT
    // ...
    // #endif
    ```
    
    And add in:
    
    ```cpp
    #define PNG_ARM_NEON_OPT 0
    ```
11. Set `ENABLE_BITCODE` to `NO` in the project settings

## Compiling Protobuf

Follow this guide: https://gist.github.com/jeffrafter/e00601fdbce17add0f9e0a06d4e69aec

You'll endup with a compiled protobuf library (2.6.1) which is compatible with `clstm` (it is not built for 3.x):

![](https://rpl.cat/uploads/m8IkLRap_DkKKB6fBqn3qphDUYpfpKvbrOOLqDbmle8/public.png)

1. Copy the `google` folder from `include` into your project.
2. Copy the `protoc` binary (in `bin`) into your project
3. Copy the `libprotobuf.a` into your project under a Frameworks group. 
4. Make sure that the lib is added to `Build Phases`.
5. Add `clstm.proto` to the compile sources: ![](https://rpl.cat/uploads/txJ9vpen__XrhCGhJPVXSuyiSf2S4IKmjb3VaAaHgRc/public.png)
6. Add in the `protoc` build rule:

![](https://rpl.cat/uploads/3S_udRfLnw1Ui4PvawFgZ8hd5ZjUP58WVZmFHFtot1g/public.png)

```bash
echo ${SRCROOT}/protoc
cd ${INPUT_FILE_DIR}
${SRCROOT}/clstm/protoc --proto_path=${INPUT_FILE_DIR} ${INPUT_FILE_PATH} --cpp_out=${SRCROOT}/clstm
```

## Make it usable from Swift

In order to use this from a briding header, it is easiest to create a C function that calls the C++. Add a `clstmocr.cpp` and `clstmocr.hpp`. This also allows us to hide the internals of protobufs, Eigen and friends from the project that includes the build lib.

```c
//
//  clstmocr.cpp
//  clstm
//
//  Created by Jeffrey Rafter and Tyler Davis on 1/24/17.
//  Copyright © 2017 Jeffrey Rafter and Tyler Davis. All rights reserved.
//

#include "clstmocr.hpp"

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
```

This is essentially a copy of the `clstmocr` calls. We've changed all of the input calls to use `const char *` so it is easier to interact with externally.

```c
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
```

## Project layout

![](https://rpl.cat/uploads/MO-SasYM-cPjAyX-pBZM16ZfQRPGyBBbCtF3BaNu8gE/public.png)
