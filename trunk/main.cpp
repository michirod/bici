#include <stdio.h>
#include "opencv/cv.h"
#include "opencv/highgui.h"
#include <vector>
#include <sstream>
#include <iomanip>
#include <math.h>
#include "funzioniLinea.h"


// variables for acquisition from file
typedef struct 
{
	char * file_name;
	IplImage* frame;
} AVI_READER;	

struct lineaTrapasso 
{
	lineaTrapasso() : A(cvPoint(-1,-1)), B(cvPoint(-1,-1)), stato(0) {};
	CvPoint A, B;
	bool stato;
};


int open_avi(AVI_READER *);
char * get_next_frame();
int display_image(int delay);
int close_avi();
void release();
void elab(IplImage* inputImage);
void onMouseClick(int event, int x, int y, int flags, void* p);
IplImage * CreaMaschera(CvSize size, lineaTrapasso puntilinea);
void DisegnaLineaTrapasso(lineaTrapasso puntilinea);

CvCapture* cap=0;

char *win=0;
char *winOut=0;
char via=0;

IplImage *currentImageGray=0;
IplImage *previous_frame=0;
IplImage *frame_diff=0;
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
	lineaTrapasso puntilinea;
	ArrayCampioni campioni;
	CvSize size;

	frame_number=0;
	
	printf("Blabbla\n");	
	/*
	printf("Insert file name : ");
	scanf("%s",filename);*/
	sprintf(filename,"./VideoNostri/AO1.avi");
	
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
	cvSetMouseCallback(filename, onMouseClick, &puntilinea);
	display_image(1);

	while(via==0)
	{
		cvWaitKey(10);
	}
		
	//inizializzazione strutture
	size.height = avi.frame->height;
	size.width = avi.frame->width;
	maschera = CreaMaschera(size, puntilinea);
	linea = cvCreateImage(size, IPL_DEPTH_8U,1);
	initArrayCampioni(&campioni, size);
	cvNamedWindow("Line");

	printf("Insert threshold value : ");
	res=scanf("%d",&th);

	if(res==1)
		thresh=(double)th;
	else
		printf("threshold value not valid; using defaults: %f\n",thresh);

	while((im=get_next_frame())!=0 && retcode!='q')
	{
		elab(avi.frame);
		addCampione(frame_diff, &campioni);
		puntilinea.stato = findObjectsInLine((&campioni)->andCampioni, maschera, linea);
		displayLineStatus(linea, "Line");
		DisegnaLineaTrapasso(puntilinea);
		retcode=(char)(display_image(30));
		frame_number++;
	}

	printf("Press a key to exit..\n");
	cvWaitKey(0);
	close_avi();
	release();

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
		winOut="two frame difference";
		cvNamedWindow(winOut, 0);
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
	cvResizeWindow(winOut,frame_diff->width,frame_diff->height);
	cvShowImage(winOut, frame_diff);
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
		cvDestroyWindow(winOut);
}



//cvCvtColor(inputImage,currentImageGray,CV_BGR2GRAY); //convert color image into an other type (in this case to gray level)
//cvAbsDiff(currentImageGray,previous_frame,frame_diff); //difference between two frame
//cvThreshold( frame_diff,frame_diff,thresh,255, CV_THRESH_BINARY );
//cvErode(frame_diff,frame_diff,NULL,1);
//cvDilate(frame_diff,frame_diff,NULL,1);

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

int display_image(int delay)
{
	cvResizeWindow(win,avi.frame->width,avi.frame->height);
	cvShowImage(win, avi.frame);
	return cvWaitKey(delay);
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
	}
}

IplImage * CreaMaschera(CvSize in_size, lineaTrapasso puntilinea)
{
	IplImage * result;
	result = cvCreateImage(in_size,IPL_DEPTH_8U,1);
	cvZero(result);
	cvLine(result, puntilinea.A, puntilinea.B, CV_RGB(255, 255, 255),3,8,0);

	//Visualize
	cvResizeWindow(win,result->width,result->height);
	cvShowImage(win, result);
	cvWaitKey(1000);
	return result;
}

void DisegnaLineaTrapasso(lineaTrapasso puntilinea)
{
	if (puntilinea.stato)
		cvLine(avi.frame, puntilinea.A, puntilinea.B, CV_RGB(255, 0, 0),3,8,0);
	else
		cvLine(avi.frame, puntilinea.A, puntilinea.B, CV_RGB(0, 255, 0),3,8,0);
}

void lineControl()
{

}
