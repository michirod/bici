#include "definitions.h"
#include <stdio.h>
#include "opencv/cv.h"
#include "opencv/highgui.h"
#include <vector>
#include <sstream>
#include <iomanip>
#include <math.h>
//#include <unistd.h>
#include "funzioniLinea.h"
#include "bgSubtraction.h"


// variables for acquisition from file
typedef struct 
{
	char * file_name;
	IplImage* frame;
} AVI_READER;	




int open_avi(AVI_READER *);
char * get_next_frame();
int close_avi();
void release();
void elab(IplImage* inputImage);
void onMouseClick(int event, int x, int y, int flags, void* p);
IplImage * CreaMaschera(CvSize size, lineaTrapasso puntilinea);
void DisegnaLineaTrapasso(lineaTrapasso puntilinea);
void displayImage(IplImage * line, char * winName);

CvCapture* cap=0;

char *win=0;
char *winOut=0;
char via=0;

IplImage *currentImageGray=0;
IplImage *previous_frame=0;
IplImage *frame_diff=0;
IplImage *bg_diff=0;
IplImage* ipl=0;
AVI_READER avi;



double  thresh=13.0;
int frame_number;

int main(int argc, char** argv)
{
	char filename[128];
	int res=0;
	int th;
	char* im=0;
	char retcode=-1;
	IplImage * maschera;
	IplImage * linea;
	lineaTrapasso puntilinea;	//conterr� i punti con cui costruiamo la linea di trapasso
	ArrayCampioni campioni;		//CAMPIONI DI IMMAGINE
	int excited_points[EXCITED_POINTS][2];
	int num_excited_points=0;
	CvSize size;
	int contatoreBici = 0;
	char contatoreStringa[100];
	CvPoint contatorePoint;
	CvFont contatoreFont;

	frame_number=0;
	for(int i = 0; i < EXCITED_POINTS; i++)
	{
		excited_points[EXCITED_POINTS][0] = 0;
		excited_points[EXCITED_POINTS][1] = 0;
	}

	printf("Blabbla\n");	
	/*
	printf("Insert file name : ");
	scanf("%s",filename);*/
	sprintf(filename,"./Video/AO1.avi");
	
	while(res!=1)
	{
		avi.file_name=filename;
		res=open_avi(&avi);
		
		if(res!=1)
		{
			printf("Insert file name (q to exit): ");
			scanf("%s",filename);
			if(strcmp(filename,"q")==0)
				return -1;
		}
	}
	//Mostriamo la maschera contenente la linea di trapasso
	//cvSetMouseCallback(filename, onMouseClick, &puntilinea);
	puntilinea.A.x = 123;
	puntilinea.A.y = 502;
	puntilinea.B.x = 285;
	puntilinea.B.y = 533;
	via = 1;
	displayImage(avi.frame,win);

	while(via==0)
	{
		//cvWaitKey(10);
	}
		
	//inizializzazione strutture
	size.height = avi.frame->height;
	size.width = avi.frame->width;
	maschera = CreaMaschera(size, puntilinea);
	linea = cvCreateImage(size, IPL_DEPTH_8U,1);
	initArrayCampioni(&campioni, size);
	cvNamedWindow("Line");
	contatorePoint = cvPoint(10, 50);
	cvInitFont(&contatoreFont, CV_FONT_HERSHEY_SIMPLEX, 1, 1, 0, 3);

#ifdef BG_SUB
	initBgGray(avi.frame, 100, 0.95, 15);
	th = 8;
#else
	th = 3;
#endif

	//printf("Insert threshold value : ");
	//res=scanf("%d",&th);

	if(res==1)
		thresh=(double)th;
	else
		printf("threshold value not valid; using defaults: %f\n",thresh);

	while((im=get_next_frame())!=0 && retcode!='q')
	{

#ifdef BG_SUB
	#ifdef BG_SUB_MODA
	#endif
		elab(avi.frame);
		if (bgSub(avi.frame, &bg_diff, frame_diff))
#else
		elab(avi.frame);
		if(true)
#endif
		{
#ifdef BG_SUB
			addCampione(bg_diff, &campioni);
#else
			addCampione(frame_diff, &campioni);
#endif
			if(frame_number>1)
				//contatoreBici += findObjectsInLine(frame_diff, maschera, linea, excited_points, &num_excited_points, puntilinea);
				contatoreBici += findObjectsInLine((&campioni)->andCampioni, maschera, linea, excited_points, &num_excited_points, puntilinea);  //And con la maschera e mette il risultato in linea (Non ha molto senso chiamare linea questa immagine!!)
			displayImage((&campioni)->andCampioni, "Line"); //visualizza i pixel eccitati della linea
			//displayImage(frame_diff, "Line");
			DisegnaLineaTrapasso(puntilinea);
			sprintf(contatoreStringa, "Bicycles: %d", contatoreBici);
			cvPutText(avi.frame, contatoreStringa, contatorePoint, &contatoreFont, cvScalar(255, 0 ,0));
			displayImage(avi.frame,win);
			//printf("Frame %d\n", frame_number);
			// stop to take photo
			if (frame_number == 1313 || frame_number == 1540 || frame_number == 2330 || frame_number == 2855 || frame_number == 3310)
			{
				//sleep(5);
			}
		}
		retcode = (char)cvWaitKey(2);
		//retcode=(char)(display_image(5));
		frame_number++;
	}

	printf("Press a key to exit..\n");
	cvWaitKey(0);
	close_avi();
	release();
#ifdef BG_SUB
	destroyBgGrayInitStructures();
#endif

	return 1;
}

void elab(IplImage* inputImage)
{
	
	if(frame_number==0){
		CvSize in_size;
		in_size.height=inputImage->height;
		in_size.width=inputImage->width;
		previous_frame=cvCreateImage(in_size,IPL_DEPTH_8U,1);
		currentImageGray=cvCreateImage(in_size,IPL_DEPTH_8U,1);
		frame_diff=cvCreateImage(in_size,IPL_DEPTH_8U,1);
		//winOut="two frame difference";
		//cvNamedWindow(winOut, 0);
	}
	cvCvtColor(inputImage,currentImageGray,CV_BGR2GRAY);
	if(frame_number==0){//swap
		IplImage *temp=previous_frame;
		previous_frame=currentImageGray;
		currentImageGray=temp;
		return;
	}

	cvAbsDiff(currentImageGray,previous_frame,frame_diff);
	cvThreshold( frame_diff,frame_diff,thresh,255, CV_THRESH_BINARY );
	
	cvErode(frame_diff,frame_diff,NULL,1);
	cvDilate(frame_diff,frame_diff,NULL,1);
	cvErode(frame_diff,frame_diff,NULL,1);
	cvDilate(frame_diff,frame_diff,NULL,3);

	//Visualize
	//cvResizeWindow(winOut,frame_diff->width,frame_diff->height);
	//cvShowImage(winOut, frame_diff);
	{//swap
		IplImage *temp=previous_frame;
		previous_frame=currentImageGray;
		currentImageGray=temp;
	}
}

void release(){
		cvReleaseImage(&previous_frame);
		cvReleaseImage(&currentImageGray);
		cvReleaseImage(&frame_diff);
		if (winOut != 0)
			cvDestroyWindow(winOut);
}


int open_avi(AVI_READER * avi_h)
{
	cap = cvCaptureFromAVI(avi_h->file_name);
	if(cap==0){
		printf("File %s was not found\n",avi_h->file_name);
		return -1;
	}
	win=avi_h->file_name;
	cvNamedWindow(win, 0);
	cvGrabFrame(cap);
	ipl = cvRetrieveFrame(cap);
	//avi_h->frame=ipl;
	if(ipl->nChannels!=3) 
	{
		printf("File %s contains images with nChannels!=3\n",avi_h->file_name);
		cvReleaseCapture(&cap);
		return -2;
	}
	avi.frame=cvCloneImage(ipl);
	if (ipl->origin)
		cvFlip(avi.frame);
	avi.frame->origin=0;
	return 1;
}

char * get_next_frame()
{
	int res=0;
	char* imag=0;
	if(ipl==0) {
		res= cvGrabFrame(cap);
		if(res>0){
			ipl= cvRetrieveFrame(cap);
			if (ipl->origin)
				cvFlip(ipl, avi.frame);
			else
				cvCopy(ipl, avi.frame);
			imag=avi.frame->imageData;
		}
	}
	else
		imag=avi.frame->imageData;
	ipl=0;
	return imag;
}


int close_avi()
{
	if(cap!=0) cvReleaseCapture( &cap );
	if(avi.frame!=0)cvReleaseImage(&(avi.frame));
	if(win!=0){
		cvDestroyWindow(win);
		win=0;
	}
	return 1;
}

void onMouseClick(int event, int x, int y, int flags, void* p)
{
	lineaTrapasso* puntilinea = reinterpret_cast<lineaTrapasso*>(p);

	if (event & CV_EVENT_LBUTTONUP)
	{
		if (puntilinea->A.x == -1)
		{
			puntilinea->A.x = x;
			puntilinea->A.y = y;
		}
		else if (puntilinea->B.x == -1)
		{
			puntilinea->B.x = x;
			puntilinea->B.y = y;
			via = 1;
		}
	printf("Clicked point (x,y) = (%d,%d)\n", x,y);
	}
}

IplImage * CreaMaschera(CvSize in_size, lineaTrapasso puntilinea)
{
	IplImage * result;
	result = cvCreateImage(in_size,IPL_DEPTH_8U,1);
	cvZero(result);
	cvLine(result, puntilinea.A, puntilinea.B, CV_RGB(255, 255, 255),LINE_THICKNESS,8,0);

	//Visualize
	cvResizeWindow(win,result->width,result->height);
	cvShowImage(win, result);
	cvWaitKey(10);
	return result;
}

void DisegnaLineaTrapasso(lineaTrapasso puntilinea)
{
	if (puntilinea.stato)
		cvLine(avi.frame, puntilinea.A, puntilinea.B, CV_RGB(255, 0, 0),LINE_THICKNESS,8,0);
	else
		cvLine(avi.frame, puntilinea.A, puntilinea.B, CV_RGB(0, 255, 0),LINE_THICKNESS,8,0);
}

