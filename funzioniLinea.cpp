#include "funzioniLinea.h"

void initArrayCampioni(ArrayCampioni * c, CvSize size)
{
	for (int i = 0; i < NUMERO_CAMPIONI; i++)
	{
		c->listaCampioni[i] = cvCreateImage(size, IPL_DEPTH_8U,1);
		cvSet(c->listaCampioni[i], cvScalar(255), NULL);
	}
	c->andCampioni = cvCreateImage(size, IPL_DEPTH_8U,1);
	c->indiceListaCampioni = 0;
}

void addCampione(IplImage * img, ArrayCampioni * campioni)
{
	if (campioni->indiceListaCampioni + 1 == NUMERO_CAMPIONI)
		campioni->indiceListaCampioni = 0;
	else
		campioni->indiceListaCampioni++;
	cvCopy(img, campioni->listaCampioni[campioni->indiceListaCampioni], NULL);
	ANDiamo(campioni);
}

IplImage * ANDiamo(ArrayCampioni * c)
{	
	cvSet(c->andCampioni, cvScalar(255), NULL);
	for(int i = 0; i < NUMERO_CAMPIONI; i++)
	{
		cvAnd(c->andCampioni, c->listaCampioni[i], c->andCampioni, NULL);
	}
	return c->andCampioni;
}

bool findObjectsInLine(IplImage * andCampioni, IplImage * lineMask, IplImage * result)
{
	bool status = false;
	int ws = andCampioni->widthStep;
	cvAnd(andCampioni, lineMask, result, NULL);
	for(int i = 0; i < result->imageSize; i++)
		if (result->imageData[i] != 0)
			return true;
//	for (int i = 0; i < result->height; i++)
//	{
//		for (int j = 0; j < result->width; j++)
//		{
//			if (result->imageData[j*ws + i] != 0)
//				return true;
//		}
//	}
	return status;
}

void destroyArrayCampioni(ArrayCampioni * c)
{
	for (int i = 0; i < NUMERO_CAMPIONI; i++)
		{
			cvReleaseImage(&(c->listaCampioni[i]));
		}
		cvReleaseImage(&(c->andCampioni));
}

void displayLineStatus(IplImage * line, char * winName)
{
	cvDilate(line,line,NULL,13);
	cvResizeWindow(winName,line->width,line->height);
	cvShowImage(winName, line);
	//return cvWaitKey(delay);
}


