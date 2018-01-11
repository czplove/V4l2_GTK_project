/*
 * Copyright (C) 2007 daniel g. siegel <dgsiegel@gmail.com>
 * All rights reserved. This software is released under the GPL2 licence.
 */
#include <gtk/gtk.h>

#include "window.h"
#include "v4l2.h"


extern unsigned char image_ready;

GtkWidget *window;
GtkWidget *vbox;
GtkWidget *drawingarea;

static gboolean on_darea_expose (GtkWidget *widget,
                          GdkEventExpose *event,
                          gpointer user_data);
static gboolean quit_capture (GtkWidget *widget,
                          GdkEventExpose *event,
                          gpointer user_data);
                          
                          
void draw_thread(GtkWidget *widget)
{
        for(;;) {
                g_usleep(10000);
                if (quit_flag == 1) break;
                gdk_threads_enter();
                if(image_ready) {	//-����׼�����˾�ˢ��
                        gtk_widget_queue_draw(GTK_WIDGET (widget));
                }
                else {
                        gdk_threads_leave();
                }
                gdk_threads_leave();
        }
}

void gtk_window_init(int argc, char **argv, struct camera *cam)
{
	if(!g_thread_supported()) {	//���gthreadû�б���ʼ��
		g_thread_init(NULL);	 //���г�ʼ��
	}

	gdk_threads_init();	//��ʼ��GDK���̣߳����������ڶ��߳���ʹ�óɶԵ�gdk_threads_enter()��gdk_thread_leave(),��Gtk����֤          gdk_threads_init()��main loopִ��֮ǰִ�У�Ϊ�˱�֤�̰߳�ȫӦ����gtk_init()֮ǰ���ã�g_thread_init()�����ں���gdk_threads_init()֮ǰִ�С�

	gtk_init (&argc, &argv);

	/* window init */
	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(window), "��Ƶ�ɼ�");
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

                          