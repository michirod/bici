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
bool findObjectsInLine(IplImage * andCampioni, IplImage * lineMask, IplImage * result);
void displayLineStatus(IplImage * line, char * winName);

#endif /* FUNZIONILINEA_H_ */
