/*
 *  V4L2 video capture example
 *
 *  This program can be used and distributed without restrictions.
 */

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

#include <linux/videodev2.h>

#define CLEAR(x) memset (&(x), 0, sizeof (x))


#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/imgproc/imgproc_c.h>
#include <opencv2/legacy/compat.hpp>

#include <iostream>
using namespace std;
//test

#include "capture.h"

void
errno_exit                      (const char *           s)
{
        fprintf (stderr, "%s error %d, %s\n",
                 s, errno, strerror (errno));

        exit (EXIT_FAILURE);
}

int
xioctl                          (int                    fd,
                                 int                    request,
                                 void *                 arg)
{
        int r;

        do r = ioctl (fd, request, arg);
        while (-1 == r && EINTR == errno);

        return r;
}



void
grava_imagem(IplImage * tmp_img, IplImage* tmp_img2, int count)
{
	static char filename[255];
	sprintf(filename,"./data/teste/t1_%i.jpg",count);
	cvSaveImage(filename,tmp_img,0);
	cout << filename << " saved!!" << endl;
	sprintf(filename,"./data/teste/t0_%i.jpg",count);
	cvSaveImage(filename,tmp_img2,0);
	cout << filename << " saved!!" << endl;
}

void
grava_imagem_calibragem(IplImage* tmp_img, IplImage* tmp_img2, int count )
{
	//PADRÃO para posterior leitura com a função 
	//carrega_imagem_gravada_calib()
	//	 camId=1 => s_img
	//	 camId=0 => s_img2
	static char filename[255];
	FILE* list_calib;
	if(count == 0)
	{
		list_calib = fopen("./data/stereo_calib_m.txt","w");
		fputs("11 7\n",list_calib);
	}
	else
		list_calib = fopen("./data/stereo_calib_m.txt","a+");
	sprintf(filename,"./data/img_calib_m/Im_Calib1_%2i.jpg", count);
	cvSaveImage(filename,tmp_img,0);
	cout << filename << " saved!!" << endl;
	sprintf(filename,"./data/img_calib_m/Im_Calib1_%2i.jpg\n", count);
	fputs(filename,list_calib);

	sprintf(filename,"./data/img_calib_m/Im_Calib0_%2i.jpg", count);
	cvSaveImage(filename,tmp_img2,0);
	cout << filename << " saved!!" << endl;
	sprintf(filename,"./data/img_calib_m/Im_Calib0_%2i.jpg\n", count);
	fputs(filename,list_calib);

	fclose(list_calib);
}

	IplImage*
carrega_imagem_gravada(int& camId, CvSize size)
{
	static int count,flag;
	if(flag == 1) {count=0; flag=0;}
	static char filename[255];
	sprintf(filename,"./data/teste/t%i_%i.jpg",camId,count);

	IplImage* tmp_img = NULL;
	tmp_img = cvCreateImage(size,IPL_DEPTH_8U,3);
	tmp_img = cvLoadImage( filename, 0 );
	if ( !tmp_img ) exit (EXIT_FAILURE);
	else cout << filename << " loaded!!" << endl;

	if(camId) count++;
	return tmp_img;
}

	IplImage*
carrega_imagem_gravada_calib(int& camId, CvSize size)
{
	static int count,flag;
	if(flag == 1) {count=0; flag=0;}
	static char filename[255];
	IplImage* tmp_img = NULL;
	tmp_img = cvCreateImage(size,IPL_DEPTH_8U,3);
	sprintf(filename,"./data/img_calib/image%i_%i.jpg",camId,count);
	tmp_img = cvLoadImage( filename, 0 );
	if(camId) count++;
	return tmp_img;
}

int 
read_frame			(IplImage ** s_img, io_method io, int fd, buffer * buffers, unsigned int n_buffers)
{
        struct v4l2_buffer buf;
	unsigned int i;

	switch (io) {
	case IO_METHOD_READ:
    		if (-1 == read (fd, buffers[0].start, buffers[0].length)) {
            		switch (errno) {
            		case EAGAIN:
                    		return 0;

			case EIO:
				/* Could ignore EIO, see spec. */

				/* fall through */

			default:
				errno_exit ("read");
			}
		}
//		cout << "\t\tread_frame" << endl;
//    		process_image (buffers[0].start, buffers[0].length, s_img);
		//PEGA A IMAGEM DO BUFFER
		memcpy((*s_img)->imageData,buffers[0].start,buffers[0].length);
//		cout << "\t\tread_frame 1" << endl;

		break;

	case IO_METHOD_MMAP:
		CLEAR (buf);

            	buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            	buf.memory = V4L2_MEMORY_MMAP;

    		if (-1 == xioctl (fd, VIDIOC_DQBUF, &buf)) {
            		switch (errno) {
            		case EAGAIN:
                    		return 0;

			case EIO:
				/* Could ignore EIO, see spec. */

				/* fall through */

			default:
				errno_exit ("VIDIOC_DQBUF");
			}
		}

                assert (buf.index < n_buffers);

//	        process_image (buffers[buf.index].start, buffers[buf.index].length, s_img);
		//PEGA IMAGEM DO BUFFER		
		memcpy((*s_img)->imageData,buffers[buf.index].start,buffers[buf.index].length);

		if (-1 == xioctl (fd, VIDIOC_QBUF, &buf))
			errno_exit ("VIDIOC_QBUF");

		break;

	case IO_METHOD_USERPTR:
		CLEAR (buf);

    		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    		buf.memory = V4L2_MEMORY_USERPTR;

		if (-1 == xioctl (fd, VIDIOC_DQBUF, &buf)) {
			switch (errno) {
			case EAGAIN:
				return 0;

			case EIO:
				/* Could ignore EIO, see spec. */

				/* fall through */

			default:
				errno_exit ("VIDIOC_DQBUF");
			}
		}

		for (i = 0; i < n_buffers; ++i)
			if (buf.m.userptr == (unsigned long) buffers[i].start
			    && buf.length == buffers[i].length)
				break;

		assert (i < n_buffers);

    		//process_image ((void *) buf.m.userptr,buf.length, s_img);
		//PEGA IMAGEM DO BUFFER
		memcpy((*s_img)->imageData,(void *) buf.m.userptr,buf.length);
			
		if (-1 == xioctl (fd, VIDIOC_QBUF, &buf))
			errno_exit ("VIDIOC_QBUF");

		break;
	}

	return 1;
}

void
getIplImage                        (IplImage ** s_img, IplImage ** s_img2, io_method io, int fd, buffer * buffers, unsigned int n_buffers, int& camId)
{
	struct timeval tim;
	int vlocal=0;

	while(vlocal < 2){
		gettimeofday(&tim, NULL);
		double t1=tim.tv_sec+(tim.tv_usec/1000000.0);	
		for (;;) {
			fd_set fds;
			struct timeval tv;
			int r;
			int sai;

			FD_ZERO (&fds);
			FD_SET (fd, &fds);

			/* Timeout. */
			tv.tv_sec = 2;
			tv.tv_usec = 0;

			r = select (fd + 1, &fds, NULL, NULL, &tv);

			if (-1 == r) {
				if (EINTR == errno)
					continue;

				errno_exit ("select");
			}

			if (0 == r) {
				fprintf (stderr, "select timeout\n");
				exit (EXIT_FAILURE);
			}
//			cout << "\tgetIplImage 1" << endl;
			if (!camId)
				sai = read_frame ( s_img, io, fd, buffers, n_buffers);
			else
				sai = read_frame ( s_img2, io, fd, buffers, n_buffers);
			if (sai)
				break;
			/* EAGAIN - continue select loop. */
		}
		camId = !camId;
		//######esse é o comando para torcar de camera###############
		if (-1 == xioctl (fd, VIDIOC_S_INPUT, &camId)) {
			perror ("VIDIOC_S_INPUT");
			exit (EXIT_FAILURE);
		}
		vlocal++;
		gettimeofday(&tim, NULL);
		double t2=tim.tv_sec+(tim.tv_usec/1000000.0);
		printf("%.6lf seconds elapsed\n", t2-t1);
	}
}

void
stop_capturing                  (io_method io, int fd)
{
        enum v4l2_buf_type type;

	switch (io) {
	case IO_METHOD_READ:
		/* Nothing to do. */
		break;

	case IO_METHOD_MMAP:
	case IO_METHOD_USERPTR:
		type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

		if (-1 == xioctl (fd, VIDIOC_STREAMOFF, &type))
			errno_exit ("VIDIOC_STREAMOFF");

		break;
	}
}

void
start_capturing                 (io_method io, int fd, buffer * buffers, unsigned int n_buffers)
{
        unsigned int i;
        enum v4l2_buf_type type;

       //seleciona o padrão NTSC
        v4l2_std_id std_id;
        std_id = V4L2_STD_NTSC_M_JP;

        if (-1 == ioctl (fd, VIDIOC_S_STD, &std_id)) {
                perror ("VIDIOC_S_STD");
                exit (EXIT_FAILURE);
        }

	switch (io) {
	case IO_METHOD_READ:
		/* Nothing to do. */
		break;

	case IO_METHOD_MMAP:
		for (i = 0; i < n_buffers; ++i) {
            		struct v4l2_buffer buf;

        		CLEAR (buf);

        		buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        		buf.memory      = V4L2_MEMORY_MMAP;
        		buf.index       = i;

        		if (-1 == xioctl (fd, VIDIOC_QBUF, &buf))
                    		errno_exit ("VIDIOC_QBUF");
		}
		
		type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

		if (-1 == xioctl (fd, VIDIOC_STREAMON, &type))
			errno_exit ("VIDIOC_STREAMON");

		break;

	case IO_METHOD_USERPTR:
		for (i = 0; i < n_buffers; ++i) {
            		struct v4l2_buffer buf;

        		CLEAR (buf);

        		buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        		buf.memory      = V4L2_MEMORY_USERPTR;
			buf.index       = i;
			buf.m.userptr	= (unsigned long) buffers[i].start;
			buf.length      = buffers[i].length;

			if (-1 == xioctl (fd, VIDIOC_QBUF, &buf))
                    		errno_exit ("VIDIOC_QBUF");
		}

		type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

		if (-1 == xioctl (fd, VIDIOC_STREAMON, &type))
			errno_exit ("VIDIOC_STREAMON");

		break;
	}
}

void
uninit_device                   (io_method io, buffer * buffers, unsigned int n_buffers)
{
        unsigned int i;

	switch (io) {
	case IO_METHOD_READ:
		free (buffers[0].start);
		break;

	case IO_METHOD_MMAP:
		for (i = 0; i < n_buffers; ++i)
			if (-1 == munmap (buffers[i].start, buffers[i].length))
				errno_exit ("munmap");
		break;

	case IO_METHOD_USERPTR:
		for (i = 0; i < n_buffers; ++i)
			free (buffers[i].start);
		break;
	}

	free (buffers);
}

buffer *
init_read			(unsigned int		buffer_size, buffer * buffers)
{
        buffers = (buffer*) calloc (1, sizeof (*buffers));

        if (!buffers) {
                fprintf (stderr, "Out of memory\n");
                exit (EXIT_FAILURE);
        }

	buffers[0].length = buffer_size;
	buffers[0].start = malloc (buffer_size);

	if (!buffers[0].start) {
    		fprintf (stderr, "Out of memory\n");
            	exit (EXIT_FAILURE);
	}
	return buffers;
}

void
init_mmap			(char* dev_name, int fd, buffer * buffers, unsigned int n_buffers)
{
	struct v4l2_requestbuffers req;

        CLEAR (req);

        req.count               = 4;
        req.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        req.memory              = V4L2_MEMORY_MMAP;

	if (-1 == xioctl (fd, VIDIOC_REQBUFS, &req)) {
                if (EINVAL == errno) {
                        fprintf (stderr, "%s does not support "
                                 "memory mapping\n", dev_name);
                        exit (EXIT_FAILURE);
                } else {
                        errno_exit ("VIDIOC_REQBUFS");
                }
        }

        if (req.count < 2) {
                fprintf (stderr, "Insufficient buffer memory on %s\n",
                         dev_name);
                exit (EXIT_FAILURE);
        }

        buffers = (buffer*) calloc (req.count, sizeof (*buffers));

        if (!buffers) {
                fprintf (stderr, "Out of memory\n");
                exit (EXIT_FAILURE);
        }

        for (n_buffers = 0; n_buffers < req.count; ++n_buffers) {
                struct v4l2_buffer buf;

                CLEAR (buf);

                buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                buf.memory      = V4L2_MEMORY_MMAP;
                buf.index       = n_buffers;

                if (-1 == xioctl (fd, VIDIOC_QUERYBUF, &buf))
                        errno_exit ("VIDIOC_QUERYBUF");

                buffers[n_buffers].length = buf.length;
                buffers[n_buffers].start =
                        mmap (NULL /* start anywhere */,
                              buf.length,
                              PROT_READ | PROT_WRITE /* required */,
                              MAP_SHARED /* recommended */,
                              fd, buf.m.offset);

                if (MAP_FAILED == buffers[n_buffers].start)
                        errno_exit ("mmap");
        }
}

void
init_userp			(unsigned int buffer_size, char* dev_name, int fd, buffer * buffers, unsigned int n_buffers)
{
	struct v4l2_requestbuffers req;
        unsigned int page_size;

        page_size = getpagesize ();
        buffer_size = (buffer_size + page_size - 1) & ~(page_size - 1);

        CLEAR (req);

        req.count               = 4;
        req.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        req.memory              = V4L2_MEMORY_USERPTR;

        if (-1 == xioctl (fd, VIDIOC_REQBUFS, &req)) {
                if (EINVAL == errno) {
                        fprintf (stderr, "%s does not support "
                                 "user pointer i/o\n", dev_name);
                        exit (EXIT_FAILURE);
                } else {
                        errno_exit ("VIDIOC_REQBUFS");
                }
        }

        buffers = (buffer*) calloc (4, sizeof (*buffers));

        if (!buffers) {
                fprintf (stderr, "Out of memory\n");
                exit (EXIT_FAILURE);
        }

        for (n_buffers = 0; n_buffers < 4; ++n_buffers) {
                buffers[n_buffers].length = buffer_size;
                buffers[n_buffers].start = memalign (/* boundary */ page_size,
                                                     buffer_size);

                if (!buffers[n_buffers].start) {
    			fprintf (stderr, "Out of memory\n");
            		exit (EXIT_FAILURE);
		}
        }
}

buffer * 
init_device                     (CvSize cvsize, char* dev_name, io_method io, int fd, buffer * buffers, unsigned int n_buffers)
{
        struct v4l2_capability cap;
        struct v4l2_cropcap cropcap;
        struct v4l2_crop crop;
        struct v4l2_format fmt;
	unsigned int min;

        if (-1 == xioctl (fd, VIDIOC_QUERYCAP, &cap)) {
                if (EINVAL == errno) {
                        fprintf (stderr, "%s is no V4L2 device\n",
                                 dev_name);
                        exit (EXIT_FAILURE);
                } else {
                        errno_exit ("VIDIOC_QUERYCAP");
                }
        }

        if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
                fprintf (stderr, "%s is no video capture device\n",
                         dev_name);
                exit (EXIT_FAILURE);
        }

	switch (io) {
	case IO_METHOD_READ:
		if (!(cap.capabilities & V4L2_CAP_READWRITE)) {
			fprintf (stderr, "%s does not support read i/o\n",
				 dev_name);
			exit (EXIT_FAILURE);
		}

		break;

	case IO_METHOD_MMAP:
	case IO_METHOD_USERPTR:
		if (!(cap.capabilities & V4L2_CAP_STREAMING)) {
			fprintf (stderr, "%s does not support streaming i/o\n",
				 dev_name);
			exit (EXIT_FAILURE);
		}

		break;
	}


        /* Select video input, video standard and tune here. */


	CLEAR (cropcap);

        cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

        if (0 == xioctl (fd, VIDIOC_CROPCAP, &cropcap)) {
                crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                crop.c = cropcap.defrect; /* reset to default */

                if (-1 == xioctl (fd, VIDIOC_S_CROP, &crop)) {
                        switch (errno) {
                        case EINVAL:
                                /* Cropping not supported. */
                                break;
                        default:
                                /* Errors ignored. */
                                break;
                        }
                }
        } else {	
                /* Errors ignored. */
        }


        CLEAR (fmt);

        fmt.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        fmt.fmt.pix.width       = cvsize.width; 
        fmt.fmt.pix.height      = cvsize.height;
//	fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_BGR24;
	fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_GREY;
        fmt.fmt.pix.field       = V4L2_FIELD_INTERLACED;

        if (-1 == xioctl (fd, VIDIOC_S_FMT, &fmt))
                errno_exit ("VIDIOC_S_FMT");

        /* Note VIDIOC_S_FMT may change width and height. */

	/* Buggy driver paranoia. */
	min = fmt.fmt.pix.width * 2;
	if (fmt.fmt.pix.bytesperline < min)
		fmt.fmt.pix.bytesperline = min;
	min = fmt.fmt.pix.bytesperline * fmt.fmt.pix.height;
	if (fmt.fmt.pix.sizeimage < min)
		fmt.fmt.pix.sizeimage = min;

	switch (io) {
	case IO_METHOD_READ:
//		init_read (fmt.fmt.pix.sizeimage,buffers);
		buffers = init_read (cvsize.width*cvsize.height,buffers);
		break;

	case IO_METHOD_MMAP:
		init_mmap (dev_name,fd,buffers,n_buffers);
		break;

	case IO_METHOD_USERPTR:
		init_userp (fmt.fmt.pix.sizeimage, dev_name, fd, buffers, n_buffers);
		break;
	}

	
}

void
close_device                    (int& fd)
{
        if (-1 == close (fd))
	        errno_exit ("close");

        fd = -1;
}

void
open_device                     (char* dev_name, int& fd)
{
        struct stat st; 

        if (-1 == stat (dev_name, &st)) {
                fprintf (stderr, "Cannot identify '%s': %d, %s\n",
                         dev_name, errno, strerror (errno));
                exit (EXIT_FAILURE);
        }

        if (!S_ISCHR (st.st_mode)) {
                fprintf (stderr, "%s is no device\n", dev_name);
                exit (EXIT_FAILURE);
        }

        fd = open (dev_name, O_RDWR /* required */ | O_NONBLOCK, 0);

        if (-1 == fd) {
                fprintf (stderr, "Cannot open '%s': %d, %s\n",
                         dev_name, errno, strerror (errno));
                exit (EXIT_FAILURE);
        }
}

void
usage                           (FILE *                 fp,
                                 int                    argc,
                                 char **                argv)
{
        fprintf (fp,
                 "Usage: %s [options]\n\n"
                 "Options:\n"
                 "-d | --device name   Video device name [/dev/video]\n"
                 "-h | --help          Print this message\n"
                 "-m | --mmap          Use memory mapped buffers\n"
                 "-r | --read          Use read() calls\n"
                 "-u | --userp         Use application allocated buffers\n"
                 "-c | --calibra       Calibrate Stereo Cameras by default stored images\n"
		 "-a | --no_card_cap   Process the images previously recorded\n"
		 "-z | --calib_my_list Calibrate Stereo Cameras by user list of images\n",
		 argv[0]);
}


