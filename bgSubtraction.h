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
} PixelHisto;
typedef struct
{
	PixelHisto R;
	PixelHisto G;
	PixelHisto B;

} ColorPixelHisto;
typedef struct
{
	ColorPixelHisto ** mat; //da inizializzare come array bidimensionale [height][width]
	int matHeight;
	int matWidth;
} BgHisto;
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

void initBg(IplImage * img, int numInit, double a, double thres);
void initBgGray(IplImage * img, int numInit, double a, double thres);
void destroyBgInitStructures();
void destroyBgGrayInitStructures();
void createBg(IplImage * img);
void createBgGray(IplImage *img);
bool bgSub(IplImage * img, IplImage ** foreground);
void cleanMask(IplImage * fgMask);
unsigned char calcolaModa(PixelHisto histo);
unsigned char * calcolaModaRGB(ColorPixelHisto colorHisto, unsigned char * result);
void display(char winName[], IplImage * image);

#endif /* BGSUBTRACTION_H_ */
