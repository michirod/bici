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
#define NUMERO_CAMPIONI 5
#define LINE_THICKNESS 1
#define EXCITED_POINTS 500
#define RAGGIO_RICERCA 1

typedef struct
{
	IplImage * listaCampioni[NUMERO_CAMPIONI];
	IplImage * andCampioni;
	int indiceListaCampioni;
} ArrayCampioni;

void initArrayCampioni(ArrayCampioni * c, CvSize size);
void addCampione(IplImage * img, ArrayCampioni * campioni);
IplImage * ANDiamo(ArrayCampioni * c);
void destroyArrayCampioni(ArrayCampioni * c);
bool findObjectsInLine(IplImage * andCampioni, IplImage * lineMask, IplImage * result, int active_points[EXCITED_POINTS][2], int *num_active_points);
bool AroundExcitation(int row, int column, int dimension, int active_points[EXCITED_POINTS][2], int numPunti, char type);
void displayImage(IplImage * image, char * winName);
int DetectObject(int row, int column, IplImage *inputImage);
void Search(int rowIndex, int columnIndex, int height, int width, IplImage * input, IplImage * output);


#endif /* FUNZIONILINEA_H_ */
