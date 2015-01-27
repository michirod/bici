#include "funzioniLinea.h"

int countertemporaneo = 0;

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

bool findObjectsInLine(IplImage * andCampioni, IplImage * lineMask, IplImage * result, int active_points[EXCITED_POINTS][2], int *num_active_points)
{
	bool status = false;
	int ws = andCampioni->widthStep;
	int contatemp=0;  //ancora non si aggiorna, sto provando il tutto, poi iniziamo a stoccarli
	int actual_active_points[EXCITED_POINTS][2];
	cvDilate(andCampioni,andCampioni,NULL,2);		//importante perchè anche se l'and è risultato di immagini già dilatate risulta essere molto ben definita e con apertureblabla
	cvAnd(andCampioni, lineMask, result, NULL);
	for (int i = 0; i < result->height; i++)
	{
		for (int j = 0; j < result->width; j++)
		{
			if (result->imageData[i*ws + j] != 0)
			{	
				
				actual_active_points[contatemp][0] = i;	//salvo il punto attivo per il prossimo ciclo
				actual_active_points[contatemp][1] = j;
				contatemp++;															   				
				if(AroundExcitation(i,j,30,active_points, *num_active_points, 0) == 0)  //qui controlliamo se c'era già un oggetto al ciclo precedente. 5 = "raggio" della zona quadrata in cui cerchiamo se è già stato attivato un pixel al frame precedente
				{
					//qui invece dobbiamo valutare quanti nuovi oggetti sono presenti, si potrebbe usare lo stesso raggio di eccitazionePrecedente magari facciamo una define
					if(AroundExcitation(i,j,30,actual_active_points, contatemp, 1) == 0)  //parametro tipo = 1, significa frame attuale
					{
						DetectObject(i,j,andCampioni);
						status = true; 
					}
				}
			}
		}
	}
	*num_active_points = contatemp;						//aggiornamento linea attiva e numero campioni al suo interno
	for(int i = 0; i < EXCITED_POINTS; i++)
	{
		active_points[i][0] = actual_active_points[i][0];
		active_points[i][1] = actual_active_points[i][1];
	}
	return status;
}

bool AroundExcitation(int row, int column, int dimension, int active_points[EXCITED_POINTS][2], int numPunti, char type) 
{
	bool excitation = 0;
	for(int c = 0; c < numPunti; c++)
	{
		for (int i = 0 - dimension; i < dimension; i++)
		{
			for (int j = 0 - dimension; j < dimension; j++)
			{
				if((active_points[c][0] + i) == row && (active_points[c][1] + j) == column)
				{
					if(type == 1)
					{
						if(i != 0 || j != 0)
						{
							excitation = 1;
							return excitation;
						}
					}
					else
					{
						excitation = 1;
						return excitation;
					}
				}
			}

		}
	}
	return excitation;
}

int DetectObject(int row, int column, IplImage *inputImage)
{
	IplImage *object;
	CvSize size;
	size.height = inputImage->height;
	size.width = inputImage->width;
	object = cvCreateImage(size,IPL_DEPTH_8U,1);
	cvSetZero(object);
	//ora troviamo i punti connessi
	object->imageData[(row)*size.width + column] = 255;
	countertemporaneo = 0;
	//Search(row, column, size.height, size.width, inputImage, object);
	SeedResearch(row, column, size.height, size.width, inputImage, object);
	cvNamedWindow("Object");
	displayImage(object, "Object");
	cvWaitKey(10);
	return 0;
}


void SeedResearch(int startRow, int startCol, int height, int width, IplImage * input, IplImage * output)
{
	bool Stop = 0;
	while(Stop == 0)
	{
		Stop=1;
		for(int i=0; i<(height-1); i++)
			for(int j=0; j<(width-1); j++)
				if(output->imageData[i*width + j] != 0)
					for(int k=i-1;k<=i+1;k++)
						for(int l=j-1;l<=j+1;l++)
						{
							if(input->imageData[k*width + l] != 0)
								if(output->imageData[k*width + l] == 0)
								{
									output->imageData[k*width + l] = 255;
									if(Stop==1)
										Stop=0;
								}
						}
		for(int i=(height-2); i>0; i--)
			for(int j=(width-2); j>0; j--)
				if(output->imageData[i*width + j] != 0)
					for(int k=i-1;k<=i+1;k++)
						for(int l=j-1;l<=j+1;l++)
							if(input->imageData[k*width + l] != 0)
								if(output->imageData[k*width + l] == 0)
								{
									output->imageData[k*width + l] = 255;
									if(Stop==1)
										Stop=0;
								}
	}
}

//ricerca ricorsiva: funzionante ma va in stack overflow per grandi oggetti!

/*void Search(int rowIndex, int columnIndex, int height, int width, IplImage * input, IplImage * output)
{	
	for(int i = -RAGGIO_RICERCA; i <= RAGGIO_RICERCA; i++)
	{
		for(int j = -RAGGIO_RICERCA; j <= RAGGIO_RICERCA; j++)
		{
			if((i+rowIndex) < (height-1) && (j+columnIndex) < (width-1)) 
				if(input->imageData[(rowIndex+i)*width + columnIndex + j] != 0 && output->imageData[(rowIndex+i)*width + columnIndex + j] == 0)
				{
					output->imageData[(rowIndex+i)*width + columnIndex + j] = 255;
					countertemporaneo++;
					Search(rowIndex+i, columnIndex+j, height, width, input, output);
				}
		}
	}
}*/



void destroyArrayCampioni(ArrayCampioni * c)
{
	for (int i = 0; i < NUMERO_CAMPIONI; i++)
		{
			cvReleaseImage(&(c->listaCampioni[i]));
		}
		cvReleaseImage(&(c->andCampioni));
}


void displayImage(IplImage * image, char * winName)
{
	cvResizeWindow(winName,image->width,image->height);
	cvShowImage(winName, image);
	//cvWaitKey(5);  //qui fermiamo tutto per 5ms!!!!! magari andrà tolto ma serve per visualizzare subito l'immagine durante il debug
}
