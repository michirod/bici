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
IplImage * bg, * bgR, * bgG, * bgB, *bgGray;
IplImage * fg;
IplImage * R, * G, * B, * Gray;
BgHisto bgHisto;
BgGrayHisto bgGrayHisto;
BgAverage bgAverage;
std::vector<std::vector<cv::Point> > contours;
int frameCount;
int numInitFrame;
double alpha, th;

void initBg(IplImage * img, int numInit, double a, double thres)
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
	alpha = a;
	th = thres;

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

void initBgGray(IplImage * img, int numInit, double a, double thres)
{
	CvSize size = cvSize(img->width, img->height);
	bg = cvCreateImage(size, IPL_DEPTH_8U, 1);
	bgGray = cvCreateImage(size, IPL_DEPTH_8U , 1);
	fg = cvCreateImage(size,IPL_DEPTH_8U , 1);
	Gray = cvCreateImage(size, IPL_DEPTH_8U , 1);
	frameCount = 0;
	numInitFrame = numInit;
	alpha = a;
	th = thres;

	bgGrayHisto.matHeight = img->height;
	bgGrayHisto.matWidth = img->width;
	int len = pow(2, img->depth);
	bgGrayHisto.mat = (PixelHisto **) malloc(sizeof(PixelHisto*) * bgGrayHisto.matWidth);
	bgAverage.matHeight = img->height;
	bgAverage.matWidth = img->width;
	bgAverage.mat = (PixelAverage **) malloc(sizeof(PixelAverage*) * bgAverage.matWidth);
	for(int i = 0; i < bgGrayHisto.matWidth; i++)
	{
		bgGrayHisto.mat[i] = (PixelHisto *) malloc(sizeof(PixelHisto) * bgGrayHisto.matHeight);
		bgAverage.mat[i] = (PixelAverage *) malloc(sizeof(PixelAverage) * bgAverage.matHeight);
		for (int j = 0; j < bgGrayHisto.matHeight; j++)
		{
			bgGrayHisto.mat[i][j].val = (int *) malloc(sizeof(int) * len);
			bgGrayHisto.mat[i][j].len = len;
			memset(bgGrayHisto.mat[i][j].val, 0, len * sizeof(int));
			bgAverage.mat[i][j].val = 0;
			bgAverage.mat[i][j].count = 0;
		}
	}

	//create GUI windows
	cvNamedWindow("Background", 0);
	//namedWindow("FG Mask MOG");
	//cvNamedWindow("FG Mask MOG 2", 0);
}

void destroyBgInitStructures()
{
	for(int i = 0; i < bgHisto.matWidth; i++)
	{
		for (int j = 0; j < bgHisto.matHeight; j++)
		{
			free(bgHisto.mat[i][j].R.val);
			free(bgHisto.mat[i][j].G.val);
			free(bgHisto.mat[i][j].B.val);
		}
		free(bgHisto.mat[i]);
	}
	free(bgHisto.mat);
}

void destroyBgGrayInitStructures()
{
	for(int i = 0; i < bgGrayHisto.matWidth; i++)
	{
		for (int j = 0; j < bgGrayHisto.matHeight; j++)
		{
			free(bgGrayHisto.mat[i][j].val);
		}
		free(bgGrayHisto.mat[i]);
	}
	free(bgGrayHisto.mat);
}

void createBg(IplImage * img)
{
	cvSplit(img, R, G, B, NULL);

	int ws = R->widthStep;

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
		}
	}
	cvMerge(bgR, bgG, bgB, NULL, bg);
}

void createBgGray(IplImage *img)
{
	cvCvtColor(img, Gray, CV_BGR2GRAY);
	int ws = Gray->widthStep;
	for(int i = 0; i < Gray->height; i++)
	{
		for(int j = 0; j < Gray->width; j++)
		{
			unsigned char val =  Gray->imageData[i*ws + j];
			bgGrayHisto.mat[j][i].val[val]++;
		}
	}
	if (frameCount == numInitFrame - 1)
	{
		for(int i = 0; i < Gray->height; i++)
		{
			for(int j = 0; j < Gray->width; j++)
			{
				unsigned char moda;
				moda = calcolaModa(bgGrayHisto.mat[j][i]);
				bgGray->imageData[i*ws + j] = moda;
			}
		}
	}
}

void updateBg(IplImage * img)
{
	cvSplit(img, R, G, B, NULL);

		int ws = R->widthStep;
		int index, diffR, diffG, diffB;

		for(int i = 0; i < img->height; i++)
		{
			for(int j = 0; j < img->width; j++)
			{
				index = i*ws +j;
				diffR = abs(R->imageData[index] - bgR->imageData[index]);
				diffG = abs(G->imageData[index] - bgG->imageData[index]);
				diffB = abs(B->imageData[index] - bgB->imageData[index]);
				if(diffR > th && diffG > th && diffB > th)
				{
					fg->imageData[index] = 255;
				}
				else
				{
					fg->imageData[index] = 0;
				}
					bgR->imageData[index] = alpha * bgR->imageData[index] + (1 - alpha) * R->imageData[index];
					bgG->imageData[index] = alpha * bgG->imageData[index] + (1 - alpha) * G->imageData[index];
					bgB->imageData[index] = alpha * bgB->imageData[index] + (1 - alpha) * B->imageData[index];
			}
		}
		cvMerge(bgR, bgG, bgB, NULL, bg);
}

void updateBgGray(IplImage * img)
{
	cvCvtColor(img, Gray, CV_BGR2GRAY);
	int ws = Gray->widthStep;
	int index;
	int diff;
	for(int i = 0; i < img->height; i++)
	{
		for(int j = 0; j < img->width; j++)
		{
			index = i*ws +j;
			double bg, cur;
			unsigned char bgVal, curVal,res;
			bgVal = bgGray->imageData[index];
			curVal = Gray->imageData[index];
			bg = alpha * bgVal;
			cur = (1.0 - alpha) * curVal;
			res = (unsigned char) rint(bg + cur);
			//res = (unsigned char) bg + cur;
//			if (res <= 0 || res >= 255)
//			{
//				//fg->imageData[index] = 255;
//				printf("index = %u, bg = %u, cur = %u\n", index, bgVal, curVal);
//				printf("bg = %f, cur = %f, res = %u\n", bg, cur, res);
//			}
//			else
//				//fg->imageData[index] = 0;
			diff = abs(curVal - bgVal);
			if(diff > th)
			{
				fg->imageData[index] = 255u;
			}
			else
			{
				fg->imageData[index] = 0;
			}
			if(diff > 5)
			{
				bgAverage.mat[j][i].count++;
				bgAverage.mat[j][i].val += curVal;
			}
			else
			{
				bgAverage.mat[j][i].count = 0;
				bgAverage.mat[j][i].val = 0;
			}
			if (bgAverage.mat[j][i].count > 100)
			{
				bgGray->imageData[index] = bgAverage.mat[j][i].val / bgAverage.mat[j][i].count;
				bgAverage.mat[j][i].count = 0;
				bgAverage.mat[j][i].val = 0;
			}
			//bgGray->imageData[index] = res;
			//bgGray->imageData[index] = rint(alpha * bgGray->imageData[index] + (1.0 - alpha) * Gray->imageData[index]);

		}
	}
}

void updateBgGrayModa(IplImage * img)
{
	cvCvtColor(img, Gray, CV_BGR2GRAY);
	int ws = Gray->widthStep;
	int index, diff;
	for(int i = 0; i < Gray->height; i++)
	{
		for(int j = 0; j < Gray->width; j++)
		{
			index = i*ws +j;
			unsigned char val =  Gray->imageData[index];
			bgGrayHisto.mat[j][i].val[val]++;
			if (frameCount % numInitFrame == 0)
			{
				unsigned char moda;
				moda = calcolaModa(bgGrayHisto.mat[j][i]);
				bgGray->imageData[index] = moda;
				memset(bgGrayHisto.mat[j][i].val, 0, bgGrayHisto.mat[j][i].len * sizeof(int));
			}
			diff = abs(Gray->imageData[index] - bgGray->imageData[index]);
			if(diff > th)
			{
				fg->imageData[index] = 255;
			}
			else
			{
				fg->imageData[index] = 0;
			}
		}
	}
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
	bool result;
	if (frameCount < numInitFrame)
	{
		//createBg(img);
		createBgGray(img);
		char winName[] = "Background";
		//display(winName, bg);
		display(winName, bgGray);
		result = false;
	}
	else
	{
		//if (frameCount == numInitFrame)
			//destroyBgInitStructures();
			//destroyBgGrayInitStructures();
		//updateBg(img);
		updateBgGray(img);
		//updateBgGrayModa(img);
		char winName[] = "Background";
		//display(winName, bg);
		display(winName, bgGray);
		sprintf(winName, "Foreground");
		display(winName, fg);
		*foreground = fg;
		result = true;
	}
	frameCount++;
	return result;
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
