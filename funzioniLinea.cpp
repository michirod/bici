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

int findObjectsInLine(IplImage * andCampioni, IplImage * lineMask, IplImage * result, int active_points[EXCITED_POINTS][2], int *num_active_points, lineaTrapasso puntilinea)
{
	int numBici = 0;
	int ws = andCampioni->widthStep;
	int contatemp=0;  //ancora non si aggiorna, sto provando il tutto, poi iniziamo a stoccarli
	int actual_active_points[EXCITED_POINTS][2];
	cvDilate(andCampioni,andCampioni,NULL,2);		//importante perchè anche se l'and è risultato di immagini già dilatate risulta essere molto ben definita e con apertureblabla
	cvAnd(andCampioni, lineMask, result, NULL);
	for (int i = 0; i < result->height; i++)
		for (int j = 0; j < result->width; j++)
			if (result->imageData[i*ws + j] != 0)
			{	
				
				actual_active_points[contatemp][0] = i;	//salvo il punto attivo per il prossimo ciclo
				actual_active_points[contatemp][1] = j;
				contatemp++;															   				
				if(AroundExcitation(i,j,30,active_points, *num_active_points, 0) == 0)  //qui controlliamo se c'era già un oggetto al ciclo precedente. 5 = "raggio" della zona quadrata in cui cerchiamo se è già stato attivato un pixel al frame precedente
				{
					//qui invece dobbiamo valutare quanti nuovi oggetti sono presenti, si potrebbe usare lo stesso raggio di eccitazionePrecedente magari facciamo una define
					if(AroundExcitation(i,j,30,actual_active_points, contatemp, 1) == 0)  //parametro tipo = 1, significa frame attuale
						numBici = numBici + DetectObject(i,j,andCampioni,puntilinea);
				}
			}

	*num_active_points = contatemp;						//aggiornamento linea attiva e numero campioni al suo interno
	for(int i = 0; i < EXCITED_POINTS; i++)
	{
		active_points[i][0] = actual_active_points[i][0];
		active_points[i][1] = actual_active_points[i][1];
	}
	return numBici;
}

bool AroundExcitation(int row, int column, int dimension, int active_points[EXCITED_POINTS][2], int numPunti, char type) 
{
	bool excitation = 0;
	for(int c = 0; c < numPunti; c++)
		for (int i = 0 - dimension; i < dimension; i++)
			for (int j = 0 - dimension; j < dimension; j++)
				if((active_points[c][0] + i) == row && (active_points[c][1] + j) == column)
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
	return excitation;
}

int DetectObject(int row, int column, IplImage *inputImage, lineaTrapasso puntilinea)
{
	IplImage *object;
	int ObjectIdentifier = 0;
	CvSize size;
	size.height = inputImage->height;
	size.width = inputImage->width;
	object = cvCreateImage(size,IPL_DEPTH_8U,1);
	cvSetZero(object);
	//ora troviamo i punti connessi
	object->imageData[(row)*size.width + column] = 255;
	
	//Search(row, column, size.height, size.width, inputImage, object);
	SeedResearch(row, column, size.height, size.width, inputImage, object);
	
	ObjectIdentifier = AnalyzeObject(object, size.height, size.width, puntilinea);

	cvNamedWindow("Object");
	displayImage(object, "Object");
	cvWaitKey(10);
	return ObjectIdentifier;
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

int AnalyzeObject(IplImage *OBJ, int height, int width, lineaTrapasso puntilinea)   //facciamo anche estrarre il perimetro e lo sostituiamo all'oggetto completo
{
	int Area=0, Perimeter=0, BaryRow=0, BaryCol=0, negLarg, posLarg, negLung, posLung, temp;
	bool flagPer;
	float aParallel, bParallel, cParallel, aPerpendicular, bPerpendicular, cPerpendicular, DenParallel, DenPerpendicular, cl1, cl2, cw1, cw2, floatemp;
	CvPoint interSup, interInf, interLeft, interRight, boundL1, boundL2, boundW1, boundW2, CornTop, CornBottom, CornLeft, CornRight;
	IplImage *OBJPer;
	CvSize size;
	size.height = OBJ->height;
	size.width = OBJ->width;
	OBJPer = cvCreateImage(size,IPL_DEPTH_8U,1);
	cvSetZero(OBJPer);
	for(int i=0;i<(height-1);i++)
		for(int j=0; j<(width-1);j++)
		{
			if(OBJ->imageData[i*width+j]!=0)
			{
				Area = Area + 1;		//Area calculator
				BaryRow = BaryRow + i;
				BaryCol = BaryCol + j;
				flagPer=0;
				for(int k=i-1;k<=i+1;k++)
					for(int l=j-1;l<=j+1;l++)
						if(OBJ->imageData[k*width+l]==0)
						{
							flagPer=1;		//Perimeter calculator
							l=j+2;
							k=i+2;
							Perimeter=Perimeter+1;
							OBJPer->imageData[i*width+j]=255;
						}
			}
		}
	
	BaryRow = (int)(BaryRow/Area);		//Barycenter extraction
	BaryCol = (int)(BaryCol/Area);
	
	bParallel = (float)1;			//calcolo coefficienti non è necessario farlo ogni volta, si potrebbe spostare oppure farlo eseguire solo la prima volta
	aParallel = (float)(puntilinea.B.y - puntilinea.A.y)/(puntilinea.A.x - puntilinea.B.x);
	cParallel = -aParallel*BaryCol - bParallel*BaryRow;
	temp = aParallel*BaryCol + bParallel*BaryRow + cParallel;
	aPerpendicular = bParallel;
	bPerpendicular = -aParallel;
	cPerpendicular = -aPerpendicular*BaryCol - bPerpendicular*BaryRow;
	
	//calcolo intersezioni asse parallelo con limiti superiore immagine
	interSup.y = 0;
	interSup.x = -cPerpendicular/aPerpendicular;
	interInf.y = height;
	interInf.x = - (bPerpendicular*interInf.y+cPerpendicular)/aPerpendicular;
	interLeft.y = - cParallel/bParallel;
	interLeft.x = - (bParallel*interLeft.y+cParallel)/aParallel;
	interRight.y = - (cParallel + width*aParallel)/bParallel;
	interRight.x = - (bParallel*interRight.y+cParallel)/aParallel;
	
	//troviamo i punti alla massima distanza dalle linee incrocianti nel baricentro
	DenParallel = sqrt(aParallel*aParallel+bParallel*bParallel);
	DenPerpendicular = sqrt(aPerpendicular*aPerpendicular+bPerpendicular*bPerpendicular);
	negLung = 1000;
	posLung = -1000;
	negLarg = 1000;
	posLarg = -1000;
	for(int i=0;i<height;i++)
		for(int j=0; j<width;j++)
			if(OBJPer->imageData[i*width+j] != 0)
			{
				temp = (int)(aPerpendicular*j+bPerpendicular*i+cPerpendicular);
				if(temp > posLarg)
				{
					posLarg = temp;
					boundL2.x = j;
					boundL2.y = i;
				}
				if(temp < negLarg)
				{
					negLarg = temp;
					boundL1.x = j;
					boundL1.y = i;
				}
				temp = (int)(aParallel*j+bParallel*i+cParallel);
				if(temp > posLung)
				{
					posLung = temp;
					boundW2.x = j;
					boundW2.y = i;
				}
				if(temp < negLung)
				{
					negLung = temp;
					boundW1.x = j;
					boundW1.y = i;
				}
			}
	posLung = posLung / DenPerpendicular;
	negLung = negLung / DenPerpendicular;
	posLarg = posLarg / DenParallel;
	negLarg = negLarg / DenParallel;
	//troviamo i punti agli angoli della bounding box
	cl1 = -(aPerpendicular*boundL1.x+bPerpendicular*boundL1.y);
	cl2 = -(aPerpendicular*boundL2.x+bPerpendicular*boundL2.y);
	cw1 = -(aParallel*boundW1.x+bParallel*boundW1.y);
	cw2 = -(aParallel*boundW2.x+bParallel*boundW2.y);

	floatemp = (aPerpendicular*bParallel - bPerpendicular*aParallel);
	CornTop.x = (int)((bPerpendicular*cw1 - bParallel*cl1)/floatemp);
	CornTop.y = (int)((aParallel*cl1 - aPerpendicular*cw1)/floatemp);
	CornBottom.x = (int)((bPerpendicular*cw2 - bParallel*cl2)/floatemp);
	CornBottom.y = (int)((aParallel*cl2 - aPerpendicular*cw2)/floatemp);
	CornLeft.x = (int)((bPerpendicular*cw2 - bParallel*cl1)/floatemp);
	CornLeft.y = (int)((aParallel*cl1 - aPerpendicular*cw2)/floatemp);
	CornRight.x = (int)((bPerpendicular*cw1 - bParallel*cl2)/floatemp);
	CornRight.y = (int)((aParallel*cl2 - aPerpendicular*cw1)/floatemp);

	//disegno linee parallele e perpendicolare alla linea di trapasso passanti per il baricentro
	cvLine(OBJPer,interSup,interInf,CV_RGB(255, 255, 255),1,8,0);
	cvLine(OBJPer,interLeft,interRight,CV_RGB(255, 255, 255),1,8,0);

	//disegnamo la bounding box
	cvLine(OBJPer,CornTop,CornLeft,CV_RGB(255, 255, 255),1,8,0);
	cvLine(OBJPer,CornLeft,CornBottom,CV_RGB(255, 255, 255),1,8,0);
	cvLine(OBJPer,CornBottom,CornRight,CV_RGB(255, 255, 255),1,8,0);
	cvLine(OBJPer,CornRight,CornTop,CV_RGB(255, 255, 255),1,8,0);

	//calcolo larghezza e lunghezza rispetto alla perpendicolare e parallela
	printf("\nArea: %d\nPerimeter: %d\nRow of barycenter: %d\nColumn of barycenter: %d",Area,Perimeter,BaryRow,BaryCol);
	printf("\nA Parallel: %f\nB Parallel: %f\nC Parallel: %f\n\nA Perpendicular: %f\nB Perpendicular: %f\nC Perpendicular: %f\n",aParallel,bParallel,cParallel,aPerpendicular,bPerpendicular,cPerpendicular);
	
	cvCopyImage(OBJPer,OBJ);
	if(Area>=1000 && Area<=1800)		//parametri temporanei
		return 1;
	else 
		return 0;
}


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
