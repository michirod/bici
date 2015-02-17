/*
 * bgSubtraction.cpp
 *
 *  Created on: 26/gen/2015
 *      Author: michele
 */

#include "bgSubtraction.h"

//opencv
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/video/background_segm.hpp>
//C
#include <stdio.h>
//C++
#include <iostream>
#include <sstream>

using namespace cv;
using namespace std;

//global variables
IplImage * bg, * bgR, * bgG, * bgB;
IplImage * fg;
IplImage * R, * G, * B;
//IplImage ** totR, ** totG, ** totB;
BgHisto bgHisto;
std::vector<std::vector<cv::Point> > contours;
int frameCount;
int numInitFrame;
double alpha;

void initBg(IplImage * img, int numInit, double a)
{
	CvSize size = cvSize(img->width, img->height);
	bg = cvCloneImage(img);
	bgR = cvCreateImage(size, img->depth , 1);
	bgB = cvCreateImage(size, img->depth , 1);
	bgG = cvCreateImage(size, img->depth , 1);
	fg = cvCreateImage(size,IPL_DEPTH_8U , 1);
	R = cvCreateImage(size, img->depth , 1);
	G = cvCreateImage(size, img->depth , 1);
	B = cvCreateImage(size, img->depth , 1);
	frameCount = 0;
	numInitFrame = numInit;
//	totR = (IplImage **) malloc(sizeof(IplImage *) * numInitFrame);
//	totG = (IplImage **) malloc(sizeof(IplImage *) * numInitFrame);
//	totB = (IplImage **) malloc(sizeof(IplImage *) * numInitFrame);
//	for (int i = 0; i < numInitFrame; i++)
//	{
//		totR[i] = cvCreateImage(size, img->depth , 1);
//		totG[i] = cvCreateImage(size, img->depth , 1);
//		totB[i] = cvCreateImage(size, img->depth , 1);
//	}
	alpha = a;

	bgHisto.matHeight = img->height;
	bgHisto.matWidth = img->width;
	int len = pow(2, img->depth);
	bgHisto.mat = (ColorPixelHisto **) malloc(sizeof(ColorPixelHisto*) * bgHisto.matWidth);
	for(int i = 0; i < bgHisto.matWidth; i++)
	{
		bgHisto.mat[i] = (ColorPixelHisto *) malloc(sizeof(ColorPixelHisto) * bgHisto.matHeight);
		for (int j = 0; j < bgHisto.matHeight; j++)
		{
			bgHisto.mat[i][j].R.val = (int *) malloc(sizeof(int) * len);
			bgHisto.mat[i][j].R.len = len;
			memset(bgHisto.mat[i][j].R.val, 0, len * sizeof(int));
			bgHisto.mat[i][j].G.val = (int *) malloc(sizeof(int) * len);
			bgHisto.mat[i][j].G.len = len;
			memset(bgHisto.mat[i][j].G.val, 0, len * sizeof(int));
			bgHisto.mat[i][j].B.val = (int *) malloc(sizeof(int) * len);
			bgHisto.mat[i][j].B.len = len;
			memset(bgHisto.mat[i][j].B.val, 0, len * sizeof(int));
		}
	}

	//create GUI windows
	cvNamedWindow("Background", 0);
	//namedWindow("FG Mask MOG");
	//cvNamedWindow("FG Mask MOG 2", 0);
}

void createBg(IplImage * img)
{
	cvSplit(img, R, G, B, NULL);

	int ws = R->widthStep;
//	cvCopy(R, totR[frameCount], NULL);
//	cvCopy(G, totG[frameCount], NULL);
//	cvCopy(B, totB[frameCount], NULL);

	for(int i = 0; i < img->height; i++)
	{
		for(int j = 0; j < img->width; j++)
		{
			unsigned char Bval =  B->imageData[i*ws + j];
			unsigned char Gval =  G->imageData[i*ws + j];
			unsigned char Rval =  R->imageData[i*ws + j];

			bgHisto.mat[j][i].B.val[Bval]++;
			bgHisto.mat[j][i].G.val[Gval]++;
			bgHisto.mat[j][i].R.val[Rval]++;
			unsigned char * moda;
			moda = calcolaModaRGB(bgHisto.mat[j][i], NULL);
			bgR->imageData[i*ws + j] = moda[0];
			bgG->imageData[i*ws + j] = moda[1];
			bgB->imageData[i*ws + j] = moda[2];
//			bgR->imageData[i*ws + j] = calcolaModa(bgHisto.mat[j][i].R);
//			bgG->imageData[i*ws + j] = calcolaModa(bgHisto.mat[j][i].G);
//			bgB->imageData[i*ws + j] = calcolaModa(bgHisto.mat[j][i].B);
		}
	}
	cvMerge(bgR, bgG, bgB, NULL, bg);
}

unsigned char * calcolaModaRGB(ColorPixelHisto colorHisto, unsigned char * result)
{
	int maxR = 0, maxG = 0, maxB = 0;
	static unsigned char maxI[3];
	maxI[0] = 0;
	maxI[1] = 0;
	maxI[2] = 0;
	for (int i = 0; i < colorHisto.B.len; i++)
		{
			if (colorHisto.R.val[i] > maxR)
			{
				maxR = colorHisto.R.val[i];
				maxI[0] = (unsigned char) i;
			}
			if (colorHisto.G.val[i] > maxG)
			{
				maxG = colorHisto.G.val[i];
				maxI[1] = (unsigned char) i;
			}
			if (colorHisto.B.val[i] > maxB)
			{
				maxB = colorHisto.B.val[i];
				maxI[2] = (unsigned char) i;
			}
		}
		return maxI;
}

unsigned char calcolaModa(PixelHisto histo)
{
	int max = 0;
	unsigned char maxI = 0;
	for (int i = 0; i < histo.len; i++)
	{
		if (histo.val[i] > max)
		{
			max = histo.val[i];
			maxI = (unsigned char) i;
		}
	}
	return maxI;
}

bool bgSub(IplImage * img, IplImage ** foreground)
{
	if (frameCount < numInitFrame)
	{
		createBg(img);
		char winName[] = "Background";
		display(winName, bg);
		return false;
	}
	else
	{
		return true;
	}
}

void cleanMask(IplImage * fgMask)
{
	int ws = fgMask->widthStep;
	for (int i = 0; i < fgMask->height; i++)
		{
			for (int j = 0; j < fgMask->width; j++)
			{
				if (fgMask->imageData[i*ws + j] != 255)
				{
					fgMask->imageData[i*ws + j] = 0;
				}
			}
		}
}

void display(char winName[], IplImage * image)
{
	cvResizeWindow(winName,image->width,image->height);
	cvShowImage(winName, image);
}
