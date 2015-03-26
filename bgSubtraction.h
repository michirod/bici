/*
 * bgSubtraction.h
 *
 *  Created on: 26/gen/2015
 *      Author: michele
 */

#ifndef BGSUBTRACTION_H_
#define BGSUBTRACTION_H_

#include "opencv/cv.h"
#include "opencv/highgui.h"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/video/background_segm.hpp"

//bg init structure
typedef struct
{
	int * val;
	int len;
	int tot;
} PixelHisto;
typedef struct
{
	PixelHisto ** mat;
	int matHeight;
	int matWidth;
} BgGrayHisto;

typedef struct
{
	int bigCount;
	int bigAccumulator;
	int bigFail;
	int littleCount;
	int littleAccumulator;
	char last;
	char trueBg;
} PixelAverage;
typedef struct
{
	PixelAverage ** mat;
	int matHeight;
	int matWidth;
} BgAverage;

void initBgGray(IplImage * img, int numInit, double a, double thres);
void destroyBgGrayInitStructures();
void createBgGray(IplImage *img, IplImage *mask);
bool bgSub(IplImage * img, IplImage ** foreground, IplImage * mask);
void cleanMask(IplImage * fgMask);
unsigned char calcolaModa(PixelHisto histo);
unsigned char calcolaMediana(PixelHisto histo);
void display(char winName[], IplImage * image);

#endif /* BGSUBTRACTION_H_ */
