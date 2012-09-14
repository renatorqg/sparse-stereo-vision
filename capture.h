#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <fcntl.h>              /* low-level i/o */
#include <unistd.h>
#include <errno.h>
#include <malloc.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#include <asm/types.h>          /* for videodev2.h */

typedef enum {
	IO_METHOD_READ,
	IO_METHOD_MMAP,
	IO_METHOD_USERPTR,
} io_method;

struct buffer {
        void *                  start;
        size_t                  length;
};

extern void
errno_exit                      (const char *           );

extern int
xioctl                          (int                    ,
                                 int                    ,
                                 void *                 );

extern void
grava_imagem(IplImage * , IplImage * , int);

extern void
grava_imagem_calibragem(IplImage* , IplImage*, int );

extern IplImage*
carrega_imagem_gravada(int& , CvSize );

extern IplImage*
carrega_imagem_gravada_calib(int& , CvSize );

extern int 
read_frame( IplImage ** , io_method , int , buffer * , unsigned int );

extern void  
getIplImage(IplImage ** , IplImage ** , io_method , int , buffer * , unsigned int , int& );

extern void
stop_capturing(io_method , int );

extern void
start_capturing(io_method , int , buffer * , unsigned int );

extern void
uninit_device(io_method , buffer * , unsigned int );

extern buffer *
init_read(unsigned int , buffer * );

extern void
init_mmap(char* , int , buffer * , unsigned int );

extern void
init_userp(unsigned int , char* , int , buffer * , unsigned int );

extern buffer * 
init_device(CvSize , char* , io_method , int , buffer * , unsigned int );

extern void
close_device(int& );

extern void
open_device(char* , int& );

extern void
usage(FILE * , int ,char ** );
