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

static unsigned char image_ready = 0;
unsigned int quit_flag = 0;

static GtkWidget *window;
static GtkWidget *vbox;
static GtkWidget *drawingarea;

static gboolean on_darea_expose (GtkWidget *widget,
                          GdkEventExpose *event,
                          gpointer user_data);
static gboolean quit_capture (GtkWidget *widget,
                          GdkEventExpose *event,
                          gpointer user_data);

void process_image (guchar *p, struct camera *cam)
{
	
	yuv422_rgb24(p, cam->rgbbuf, cam->width, cam->height);
	image_ready = 1;
}

static void draw_thread(GtkWidget *widget)
{
        for(;;) {
                g_usleep(10000);
                if (quit_flag == 1) break;
                gdk_threads_enter();
                if(image_ready) {	//-数据准备好了就刷新
                        gtk_widget_queue_draw(GTK_WIDGET (widget));
                }
                else {
                        gdk_threads_leave();
                }
                gdk_threads_leave();
        }
}

static void capture_thread(struct camera *cam)	//视频采集线程
{

	for (;;) {
		g_usleep(10000);

		if (quit_flag == 1) break;

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



static void gtk_window_init(int argc, char **argv, struct camera *cam)
{
	if(!g_thread_supported()) {	//如果gthread没有被初始化
		g_thread_init(NULL);	 //进行初始化
	}

	gdk_threads_init();	//初始化GDK多线程，这样可以在多线程中使用成对的gdk_threads_enter()和gdk_thread_leave(),在Gtk程序保证          gdk_threads_init()在main loop执行之前执行，为了保证线程安全应该在gtk_init()之前调用，g_thread_init()必须在函数gdk_threads_init()之前执行。

	gtk_init (&argc, &argv);

	/* window init */
	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(window), "视频采集");
	gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
	gtk_window_set_default_size(GTK_WINDOW(window), cam->width, cam->height);
	g_signal_connect(G_OBJECT(window),"delete_event", GTK_SIGNAL_FUNC(quit_capture), cam);

	/* vbox init */
	vbox = gtk_vbox_new(FALSE,0);
	gtk_container_add(GTK_CONTAINER(window),vbox);

	/* drawingarea init */
	drawingarea = gtk_drawing_area_new();
	gtk_box_pack_start(GTK_BOX(vbox),drawingarea,TRUE,TRUE,0);
   	gtk_drawing_area_size (GTK_DRAWING_AREA (drawingarea), cam->width, cam->height);				
	g_signal_connect (GTK_OBJECT (drawingarea), "expose-event",                         // Put RGB image in darea.
                        GTK_SIGNAL_FUNC (on_darea_expose), cam);


	gtk_widget_show(vbox);							
	gtk_widget_show(drawingarea);
	gtk_widget_show(window);

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

static gboolean on_darea_expose (GtkWidget *widget,
                 GdkEventExpose *event,
                 gpointer user_data)
{
	struct camera *cam;

	cam = (struct camera *)(user_data); 
	gdk_draw_rgb_image (widget->window, widget->style->fg_gc[GTK_STATE_NORMAL],
                	0, 0, cam->width, cam->height,
                     	GDK_RGB_DITHER_MAX, cam->rgbbuf, cam->width * cam->display_depth);
	image_ready = 0;
	return 1;
}

static gboolean quit_capture(GtkWidget *widget,
                 GdkEventExpose *event,
                 gpointer user_data)
{
	struct camera *cam;
	
	cam = (struct camera *)user_data;
	
	quit_flag = 1;

        stop_capturing (cam);
        uninit_camera(cam);

        close_camera(cam);

	free(cam->rgbbuf);
	free(cam);

	gtk_main_quit();

	return 1;

}
