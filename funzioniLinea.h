/*
 * funzioniLinea.h
 *
 *  Created on: 24/gen/2015
 *      Author: michele
 */

#ifndef FUNZIONILINEA_H_
#define FUNZIONILINEA_H_

#include "opencv/cv.h"
#include "opencv/highgui.h"
#include "definitions.h"
#ifdef BG_SUB
	#define NUMERO_CAMPIONI 2
#else
	#define NUMERO_CAMPIONI 5
#endif
#define LINE_THICKNESS 1
#define EXCITED_POINTS 500
#define RAGGIO_RICERCA 1

typedef struct
{
	IplImage * listaCampioni[NUMERO_CAMPIONI];
	IplImage * andCampioni;
	int indiceListaCampioni;
} ArrayCampioni;

struct lineaTrapasso 
{
	lineaTrapasso() : A(cvPoint(-1,-1)), B(cvPoint(-1,-1)), stato(0) {};
	CvPoint A, B;
	bool stato;
};

void initArrayCampioni(ArrayCampioni * c, CvSize size);
void addCampione(IplImage * img, ArrayCampioni * campioni);
IplImage * ANDiamo(ArrayCampioni * c);
void destroyArrayCampioni(ArrayCampioni * c);
int findObjectsInLine(IplImage * andCampioni, IplImage * lineMask, IplImage * result, int active_points[EXCITED_POINTS][2], int *num_active_points, lineaTrapasso puntilinea);
bool AroundExcitation(int row, int column, int dimension, int active_points[EXCITED_POINTS][2], int numPunti, char type);
void displayImage(IplImage * image, char * winName);
int DetectObject(int row, int column, IplImage *inputImage, lineaTrapasso puntilinea);
//void Search(int rowIndex, int columnIndex, int height, int width, IplImage * input, IplImage * output);
void SeedResearch(int startRow, int startCol, int height, int width, IplImage * input, IplImage * output);
int AnalyzeObject(IplImage *OBJ, int height, int width, lineaTrapasso puntilinea);

#endif /* FUNZIONILINEA_H_ */
