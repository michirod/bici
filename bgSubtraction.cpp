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

//global variables
IplImage *bgGray;
IplImage * fg;
IplImage * Gray;
BgGrayHisto bgGrayHisto;
BgAverage bgAverage;
int frameCount;
int numInitFrame;
double alpha, th;

void initBgGray(IplImage * img, int numInit, double a, double thres)
{
	CvSize size = cvSize(img->width, img->height);
	bgGray = cvCreateImage(size, IPL_DEPTH_8U , 1);
	fg = cvCreateImage(size,IPL_DEPTH_8U , 1);
	Gray = cvCreateImage(size, IPL_DEPTH_8U , 1);
	frameCount = 0;
	numInitFrame = numInit;
	alpha = a;
	th = thres;

	bgGrayHisto.matHeight = img->height;
	bgGrayHisto.matWidth = img->width;
	int len = pow(2.0, (double)img->depth);//pow(2, img->depth);
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
			bgGrayHisto.mat[i][j].tot = 0;
			bgAverage.mat[i][j].bigAccumulator = 0;
			bgAverage.mat[i][j].bigCount = 0;
			bgAverage.mat[i][j].littleAccumulator = 0;
			bgAverage.mat[i][j].littleCount = 0;
			bgAverage.mat[i][j].last = 0;
		}
	}

	//create GUI windows
	cvNamedWindow("Background", 0);
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
	cvReleaseImage(&bgGray);
	cvReleaseImage(&fg);
	cvReleaseImage(&Gray);

}

void createBgGray(IplImage *img, IplImage *mask)
{
	cvCvtColor(img, Gray, CV_BGR2GRAY);
	int ws = Gray->widthStep;
	bool isMasked = mask != NULL;
	for(int i = 0; i < Gray->height; i++)
	{
		for(int j = 0; j < Gray->width; j++)
		{
			if (isMasked) // se uso una maschera conto il pixel solo se non in foreground
			{
				if (mask->imageData[i*ws + j] == 0)
				{
					unsigned char val =  Gray->imageData[i*ws + j];
					bgGrayHisto.mat[j][i].val[val]++;
					bgGrayHisto.mat[j][i].tot++;
				}
			}
			else // se non uso maschera conto sempre
			{
				unsigned char val =  Gray->imageData[i*ws + j];
				bgGrayHisto.mat[j][i].val[val]++;
				bgGrayHisto.mat[j][i].tot++;
			}
		}
	}
	if (frameCount == numInitFrame - 1) //arrivo all'ultimo frame per l'inizializzazione del background
	{
		for(int i = 0; i < Gray->height; i++)
		{
			for(int j = 0; j < Gray->width; j++)
			{
				unsigned char moda;
				moda = calcolaModa(bgGrayHisto.mat[j][i]); // calcolo la moda per ogni pixel
				bgGray->imageData[i*ws + j] = moda;
				bgAverage.mat[j][i].last = moda;
				bgAverage.mat[j][i].trueBg = moda;
			}
		}
	}
}



void updateBgGray(IplImage * img, IplImage *mask)
{
	cvCvtColor(img, Gray, CV_BGR2GRAY);
	int ws = Gray->widthStep;
	int index;
	int diff;
	//double bg, cur;
	//unsigned char res;
	unsigned char bgVal, curVal;
	bool isMasked = mask != NULL;
	for(int i = 0; i < img->height; i++)
	{
		for(int j = 0; j < img->width; j++)
		{
			index = i*ws +j;
			bgVal = bgGray->imageData[index];
			curVal = Gray->imageData[index];
			//bg = alpha * bgVal;
			//cur = (1.0 - alpha) * curVal;
			//res = (unsigned char) rint(bg + cur);
			//res = (unsigned char) bg + cur;
			diff = abs(curVal - bgVal);
			if(diff > th) // creo la change mask
			{
				fg->imageData[index] = 255u;
			}
			else
			{
				fg->imageData[index] = 0;
			}

			// procedo con l'aggiornamento del background
			if (!isMasked || (isMasked && mask->imageData[index] == 0))
			{
				//if(abs(curVal - bgAverage.mat[j][i].last) < 3)
				if(diff > th)
				{
					//se pixel appare nel foreground aumento il contatore di pixel diverso
					bgAverage.mat[j][i].bigCount++;
					bgAverage.mat[j][i].bigAccumulator += curVal;
					bgAverage.mat[j][i].last = curVal;
					bgAverage.mat[j][i].littleCount = 0;
					bgAverage.mat[j][i].littleAccumulator = 0;
				}
				else // se la variazione del pixel rispetto al BG è minore della soglia
				{
					if (bgAverage.mat[j][i].bigCount != 0)
					{// se il pixel in questo frame è il primo simile allo sfondo dopo una serie di frame in cui era foreground
						if(abs(curVal - bgAverage.mat[j][i].last) < 3)
							bgGray->imageData[index] = bgAverage.mat[j][i].last;
					}
					else // il pixel non era foreground nei frame precedenti
					{
						bgAverage.mat[j][i].littleCount++; //aumento il contatore delle variazioni piccole
						bgAverage.mat[j][i].littleAccumulator += curVal;
						bgAverage.mat[j][i].last = curVal;
					}
					bgAverage.mat[j][i].bigCount = 0;
					bgAverage.mat[j][i].bigAccumulator = 0;
				}
				if (bgAverage.mat[j][i].bigCount > 100)
				{
					// se il pixel è diverso dal background per N volte consecutive allora diventa parte del background
					bgAverage.mat[j][i].last = bgVal;
					// il nuovo valore è la media degli N valori precendenti
					bgGray->imageData[index] = (char) (bgAverage.mat[j][i].bigAccumulator / bgAverage.mat[j][i].bigCount);
					bgAverage.mat[j][i].bigCount = 0;
					bgAverage.mat[j][i].bigAccumulator = 0;
				}
				if (bgAverage.mat[j][i].littleCount > 1)
				{
					// se la variazione del pixel è minore della soglia, l'aggiornamento è più veloce (per star dietro ai cambiamenti di luce)
					bgGray->imageData[index] = (char) (bgAverage.mat[j][i].littleAccumulator / bgAverage.mat[j][i].littleCount);
					//bgGray->imageData[index] = bgAverage.mat[j][i].last;
					bgAverage.mat[j][i].littleCount = 0;
					bgAverage.mat[j][i].littleAccumulator = 0;
				}
				// se il pixel ha un valore molto vicino a quello del background, si aggiorna istantaneamente
				if (abs(curVal - bgAverage.mat[j][i].trueBg) < 3)
				{
					//bgGray->imageData[index] = bgAverage.mat[j][i].trueBg;
					bgGray->imageData[index] = curVal;
					bgAverage.mat[j][i].trueBg = curVal;
					bgAverage.mat[j][i].littleCount = 0;
					bgAverage.mat[j][i].littleAccumulator = 0;
					bgAverage.mat[j][i].bigCount = 0;
					bgAverage.mat[j][i].bigAccumulator = 0;
				}
			}
			else
			{
				bgAverage.mat[j][i].littleCount = 0;
				bgAverage.mat[j][i].littleAccumulator = 0;
				bgAverage.mat[j][i].bigCount = 0;
				bgAverage.mat[j][i].bigAccumulator = 0;
			}
			//bgGray->imageData[index] = res;
			//bgGray->imageData[index] = rint(alpha * bgGray->imageData[index] + (1.0 - alpha) * Gray->imageData[index]);

		}
	}
}

void updateBgGrayModa(IplImage * img, IplImage *mask)
{
	cvCvtColor(img, Gray, CV_BGR2GRAY);
	int ws = Gray->widthStep;
	int index, diff;
	unsigned char bgVal, curVal, moda;
	bool isMasked = mask != NULL;
	for(int i = 0; i < Gray->height; i++)
	{
		for(int j = 0; j < Gray->width; j++)
		{
			index = i*ws +j;
			bgVal = bgGray->imageData[index];
			curVal = Gray->imageData[index];
			if (!isMasked || (isMasked && mask->imageData[index] == 0))
			{
				bgGrayHisto.mat[j][i].val[curVal]++;
				bgGrayHisto.mat[j][i].tot++;
				if (frameCount % numInitFrame == 0) // ogni N frame ricalcolo il background
				{
					//moda = calcolaModa(bgGrayHisto.mat[j][i]);
					moda = calcolaMediana(bgGrayHisto.mat[j][i]);
					bgGray->imageData[index] = moda;
					// azzero le strutture per il calcolo del background
					memset(bgGrayHisto.mat[j][i].val, 0, bgGrayHisto.mat[j][i].len * sizeof(int));
					bgGrayHisto.mat[j][i].tot = 0;
				}
			}
			// creo la change mask
			diff = abs(curVal - bgVal);
			if(diff > th)
			{
				fg->imageData[index] = 255u;
			}
			else
			{
				fg->imageData[index] = 0;
			}
		}
	}
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

unsigned char calcolaMediana(PixelHisto histo)
{
	unsigned char res = 0;
	int med = histo.tot / 2;
	int counter = 0;
	while (true)
	{
		counter += histo.val[res];
		if (counter >= med)
			break;
		res++;
	}
	return res;
}

bool bgSub(IplImage * img, IplImage ** foreground, IplImage *mask)
{
	bool result;
	//prima parte: background initialization
	if (frameCount < numInitFrame)
	{
		createBgGray(img, mask);
		char winName[] = "Background";
		display(winName, bgGray);
		result = false;
	}
	else //seconda parte: background updating
	{
#ifdef BG_SUB_MODA
		updateBgGrayModa(img, mask);
#else
		updateBgGray(img, mask);
#endif
		char winName[] = "Background";
		display(winName, bgGray);
		sprintf(winName, "Foreground");
		cvErode(fg,fg,NULL,1);
		//cvDilate(fg,fg,NULL,1);
		//cvErode(fg,fg,NULL,1);
		cvDilate(fg,fg,NULL,3);
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
