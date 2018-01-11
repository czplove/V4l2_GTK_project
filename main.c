/*
 *  V4L2 video capture based on gnome/gtk2.0+
 *  made by zhenguoyao 2011.11.18 at UESTC chengdu
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

#include <gtk/gtk.h>
#include <asm/types.h>          /* for videodev2.h */
#include "v4l2.h"
#include "yuv422_rgb.h"

unsigned char image_ready = 0;
unsigned int quit_flag = 0;

extern GtkWidget *drawingarea;

extern void draw_thread(GtkWidget *widget);

void fb_draw_point(void *memp, 
	  			   unsigned int xres, unsigned int yres, 
				   unsigned int x, unsigned int y, 
				   unsigned int color)
{
   *((unsigned int *)memp+xres*y+x)=color;	//-整个屏幕映射成了一块内存,实际操作驱动去完成
}

void process_image (guchar *p, struct camera *cam)
{
	
	//-yuv422_rgb24(p, cam->rgbbuf, cam->width, cam->height);
	int k=3, i=0, j=0;
	//-int col[] = {0xffffffff,0x00000000,~0x1f,0x0000f800,0x7e0,0x1f};
	unsigned int col[] = {0xffff,0x0000,~0x1f,0xf800,0x7e0,0x1f};
	unsigned char *rgb_temp;
	
	
	   //-printf("%d:col[%d] = 0x%x",k,k%5,col[k%5]);
	   /*
			0:col[0] = 0xffffffff		白色
			1:col[1] = 0x0				黑色
			2:col[2] = 0xffffffe0		黄色
			3:col[3] = 0xf800			红色
			4:col[4] = 0x7e0			绿纯色
			
			//-unsigned short int
			0:col[0] = 0xffff
			1:col[1] = 0x0
			2:col[2] = 0xffe0
			3:col[3] = 0xf800
			4:col[4] = 0x7e0
	   */

	   //-color = 1<<k;
	   rgb_temp = cam->rgbbuf;
	   for(j=0; j<cam->height - 1; j++){	//-全屏底色是黑的
		   for(i=0; i<cam->width - 1 ; i++) {
				//-fb_draw_point(cam->rgbbuf,cam->width,cam->height,i,j,col[3]);
			*(rgb_temp++) = 0xff;	//-R
			*(rgb_temp++) = 0xff;	//-G
			*(rgb_temp++) = 0xff;	//-B
		   }
	   }

#if 0
	   for(j=0; j<vinfo.yres/2 - 1; j++){	//-验证屏幕和内存之间的对应关系
		   for(i=0; i<vinfo.xres/2 - 1 ; i++) {
				fb_draw_point(FrameBuffer,vinfo.xres,vinfo.yres,i,j,col[0]);
		   }
	   }
#endif

#if 1
		rgb_temp = cam->rgbbuf;
		for(i = cam->width / 2 - 50; i < cam->width / 2 + 50; i++) {
	        for(j=cam->height / 2 - 50; j<cam->height / 2 + 50; j++) {
	            //-fb_draw_point(cam->rgbbuf,cam->width,cam->height,i,j,col[k%5]);
			rgb_temp[(cam->width*j + i)*3] = 0x00;
			rgb_temp[(cam->width*j + i)*3 + 1] = 0x00;
			rgb_temp[(cam->width*j + i)*3 + 2] = 0xff;
			}
		}
#endif

	
	image_ready = 1;
}



static void capture_thread(struct camera *cam)	//视频采集线程
{

	for (;;) {
		g_usleep(10000);

		if (quit_flag == 1) break;
			
		//-测试窗口显示功能
		process_image (cam->buffers[0].start, cam);
		g_usleep(1000000);
		continue;

		gdk_threads_enter();
		fd_set fds;
		struct timeval tv;
		int r;

		FD_ZERO (&fds);
		FD_SET (cam->fd, &fds);

		/* Timeout. */
		tv.tv_sec = 2;
		tv.tv_usec = 0;

		r = select (cam->fd + 1, &fds, NULL, NULL, &tv);	//-判断设备是否可读，这是非阻塞的读方式。

		if (-1 == r) {
        		if (EINTR == errno)
                	continue;

        		errno_exit ("select");
		}

		if (0 == r) {
        		fprintf (stderr, "select timeout\n");
        		exit (EXIT_FAILURE);
		}
		//-如果设备可读那么select就会返回1,从而执行read_frame(cam),进行视频数据的读取。
		if (read_frame (cam))
            		gdk_threads_leave();
		/* EAGAIN - continue select loop. */
	}	
}






int main(int argc, char **argv)
{
	/*
	 * init struct camera 
	 */
	struct camera *cam;
	
	cam = malloc(sizeof(struct camera));	//-申请的内存在不同线程中可以使用
	if (!cam) {
		printf("malloc camera failure!\n");
		exit(1);
	}
	cam->device_name = "/dev/video0";
	cam->buffers = NULL;
	cam->width = 320;	//我的摄像头质量比较差，最大分辨率只有320*240
	cam->height = 240;
	cam->display_depth = 3;  /* RGB24 */
	cam->rgbbuf = malloc(cam->width * cam->height * cam->display_depth);	//-提取出来的纯数据就放在这里

	if (!cam->rgbbuf) {
		printf("malloc rgbbuf failure!\n");
		exit(1);
	}
#ifdef RUN_CAM
	open_camera(cam);	//打开设备
	//-get_cam_cap(cam);	//得到设备信息，如果定义了DEBUG_CAM，则会打印视频信息
	//-get_cam_pic(cam);	//得到图形信息，同样如果定义了DEBUG_CAM，则会打印信息  
	//-get_cam_win(cam);	//得到视频显示信息
	//-cam->video_win.width = cam->width;
	//-cam->video_win.height = cam->height;
	//-set_cam_win(cam);	//设置图像大小，视频显示信息中包括摄像头支持的最大分辨率以及最小分辨率，这个可以设置，我设置的是320×240,当然也可以设置成其他，不过只能设置成特定的一些值  
	//-get_cam_win(cam);	//显示设置之后的视频显示信息，确定设置成功
        init_camera(cam);	//初始化设备，这个函数包括很多有关v4l2的操作 
        start_capturing (cam);	//打开视频采集 
#endif
	gtk_window_init(argc,argv,cam);	//初始化图形显示
	//建立线程 
	g_thread_create((GThreadFunc)draw_thread, drawingarea, FALSE, NULL);
#ifdef RUN_CAM	
	g_thread_create((GThreadFunc)capture_thread, cam, FALSE, NULL);
#endif

	gdk_threads_enter();	//-是一宏定义，占领临界区，这样使Gdk和Gtk+函数在多线程中被安全的调用而不会造成竞争条件，保证数据安全，一次只能有一线程访问临界区。
	gtk_main();	//进入主循环之后，两个线程开始工作
	gdk_threads_leave();
  	return 0;
}


