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

bool findObjectsInLine(IplImage * andCampioni, IplImage * lineMask, IplImage * result, int active_points[EXCITED_POINTS][2], int *num_active_points)
{
	bool status = false;
	int ws = andCampioni->widthStep;
	int contatemp=0;  //ancora non si aggiorna, sto provando il tutto, poi iniziamo a stoccarli
	int temp_active_points[EXCITED_POINTS][2];
	cvAnd(andCampioni, lineMask, result, NULL);
	/*for(int i = 0; i < result->imageSize; i++)
		if (result->imageData[i] != 0)
			return true;*/
	for (int i = 0; i < result->height; i++)
	{
		for (int j = 0; j < result->width; j++)
		{
			if (result->imageData[i*ws + j] != 0)
			{	
				
				temp_active_points[contatemp][0] = i;	//salvo il punto attivo per il prossimo ciclo
				temp_active_points[contatemp][1] = j;
				contatemp++;															   				
				if(eccitazionePrecedente(result,i,j,5,active_points, *num_active_points) == 0)  //qui controlliamo se c'era già un oggetto al ciclo precedente. 5 = "raggio" della zona quadrata in cui cerchiamo se è già stato attivato un pixel al frame precedente
				{
					status = true; //NUOVO OGGETTO??? bisogna guardare sia le connessioni interne alla linea di trapasso attuale, sia eventualmenet anche alla connessione esterna
				}
			}
		}
	}
	*num_active_points = contatemp;						//aggiornamento linea attiva e numero campioni al suo interno
	for(int i = 0; i < EXCITED_POINTS; i++)
	{
		active_points[i][0] = temp_active_points[i][0];
		active_points[i][1] = temp_active_points[i][1];
	}
	return status;
}

bool eccitazionePrecedente(IplImage * img, int row, int column, int dimension, int active_points[EXCITED_POINTS][2], int numPunti) 
{
	bool excitation = 0;
	int ws = img->widthStep;		// magari non conviene farla come funzione nuova ma inglobarla?
	for(int c = 0; c < numPunti; c++)
	{
		for (int i = 0 - dimension; i < dimension; i++)
		{
			for (int j = 0 - dimension; j < dimension; j++)
			{
				if(img->imageData[(active_points[c][0] + i) * ws + (active_points[c][1] + j)] != 0)
				{
					excitation = 1;
					return excitation;
				}
			}

		}
	}
	return excitation;
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
	cvDilate(line,line,NULL,13); //questa dilate andrà poi fatta nell'immagine in cui cerchiamo i blob, non solo nella linea!
	cvResizeWindow(winName,line->width,line->height);
	cvShowImage(winName, line);
	//return cvWaitKey(delay);
}


