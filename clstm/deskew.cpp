//
//  deskew.cpp
//  clstm
//
//  Created by Jeffrey Rafter on 1/26/17.
//  Copyright © 2017 Jeffrey Rafter. All rights reserved.
//
#include "deskew.hpp"

#include <stdio.h>
#include <leptonica/allheaders.h>
#include <string>

static const l_int32 DEFAULT_BINARY_THRESHOLD = 130;
static const l_int32 MIN_SEGMENT_HEIGHT = 3;
static const l_float32 DEG_2_RAD = 3.1415926535 / 180.0;

double getMean(double *data, int size) {
  double sum = 0.0;

  for(int i = 0; i < size; i++) {
    double a = data[i];
    sum += a;
  }

  return sum/size;
 }

double getVariance(double *data, int size) {
  double mean = getMean(data, size);
  double temp = 0;

  for(int i = 0; i < size; i++) {
    double a = data[i];
    temp += (a-mean)*(a-mean);
  }
  return temp/size;
}

// could use better naming
double getLinesBitVariance(Pix *image, l_int32 w, l_int32 h) {
  l_int32 i, j;
  double line_means[h];
  void **lines;
  lines = pixGetLinePtrs(image, NULL);

  for (i = 0; i < h; i++) {  /* scan over image */
    double line_total = 0.0;

    for (j = 0; j < w; j++) {
      line_total += GET_DATA_BIT(lines[i], j);
    }
    line_means[i] = line_total / w;
  }

  return getVariance(line_means, h);
}

void pixWiteMiddleSegment(Pix *image, const char *output, l_int32 w, l_int32 h) {
  l_int32 i, j;
  void **lines;
  Box **segments;

  int vert_middle = h / 2;

  lines = pixGetLinePtrs(image, NULL);

  int top = -1;

  for (i = 0; i < h; i++) {  /* scan over image */
    int black = 0;

    for (j = 0; j < w; j++) {
      if (GET_DATA_BIT(lines[i], j) > 0) {
        black = 1;
        break;
      }
    }

    // current line is all whitespace
    if (black == 0) {
      // If we have a segment running, we need to end it
      if (top >= 0 && i - top >= MIN_SEGMENT_HEIGHT) {
        printf("\nSegment (top:%d, height:%d)", top, (i - 1) - top);
        int segment_height = (i - 1) - top;

        if (vert_middle >= top && vert_middle <= (top + segment_height)) {
          Box *cropBox = boxCreate(0, top, w, (i - 1) - top);
          Pix *cropImage = pixClipRectangle(image, cropBox, NULL);
          pixWrite(output, cropImage, IFF_PNG);

          boxDestroy(&cropBox);
          pixDestroy(&cropImage);
          break;
        }
        top = -1;
      }
    } else {
      // If we are in white space, start a new segment
      if (top == -1) {
        top = i;
      }
    }
  }
}

Pix *binarizeImage(Pix *image) {
  Pix *otsuImage = pixConvertRGBToGray(image, 0.0f, 0.0f, 0.0f);
  l_int32 status = pixOtsuAdaptiveThreshold(otsuImage, 2000, 2000, 0, 0, 0.0f, NULL, &otsuImage);

  if (status == 1) {
    printf("Error in binarizeImage: pixOtsuAdaptiveThreshold() failed.\n");
  }

  Pix *binImage = pixConvertTo1(otsuImage, DEFAULT_BINARY_THRESHOLD);
  pixDestroy(&otsuImage);

  return binImage;
}

double getRotationAngle(Pix *image) {

  l_int32 w, h, d;
  pixGetDimensions(image, &w, &h, &d);

  // Brute force, rotate and check variance from -45 deg to 45 deg
  double angle = 0;
  double min = -45;
  double max = 45;
  double accuracy = 1; // Fine tune the final angle accuracy
  while (max > min + accuracy) {
    double minAngle = angle - ((angle - min) / 2);
    double maxAngle = max - ((max - angle ) / 2);

    Pix *rotMin = pixRotate(image, DEG_2_RAD * minAngle, L_ROTATE_AREA_MAP, L_BRING_IN_WHITE, 0, 0);
    double varMin = getLinesBitVariance(rotMin, w, h);
    pixDestroy(&rotMin);

    Pix *rotMax = pixRotate(image, DEG_2_RAD * maxAngle, L_ROTATE_AREA_MAP, L_BRING_IN_WHITE, 0, 0);
    double varMax = getLinesBitVariance(rotMax, w, h);
    pixDestroy(&rotMax);

    if (varMin > varMax) {
      max = angle;
    } else if (varMax > varMin) {
      min = angle;
    } else {
      break;
    }

    printf("\ndegrees: %f, varMin: %f, varMax: %f\n", angle, varMin, varMax);
    angle = max - ((max - min) / 2);
  }

  // We found the rotation with the most variance
  printf("\n----------------\n");
  printf("Angle: %f\n", angle);

  return angle;
}

Pix * deskewImage(Pix *image) {
  // Binarize using Otsu Threshold
  Pix *binImage = binarizeImage(image);

  // Scale that image (for quicker finding of rotation angle)
  Pix *scaleImage = pixScale(binImage, 0.2, 0.2);

  // TODO: add in a connected component analysis to despeckle/denoise before this step
  double bestAngle = getRotationAngle(scaleImage);

  // TODO: could save this rotation by using the final angle in the variance lookup
  Pix *newImage = pixRotate(binImage, DEG_2_RAD * bestAngle, L_ROTATE_AREA_MAP, L_BRING_IN_WHITE, 0, 0);

  pixDestroy(&binImage);
  pixDestroy(&scaleImage);

  return newImage;
}

void preProcessMiddleSegment(const char * input_filename, const char * output_filename) {
  Pix *image = pixRead(input_filename);
  Pix *deskewedImage = deskewImage(image);

  l_int32 w, h, d;
  pixGetDimensions(deskewedImage, &w, &h, &d);
  // TODO: could run a gaussian filter before this step
  // printSegments(deskewedImage, w, h);
  pixWiteMiddleSegment(deskewedImage, output_filename, w, h);

  // destroy stuff
  pixDestroy(&image);
  pixDestroy(&deskewedImage);
}
