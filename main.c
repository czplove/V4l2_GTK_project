/*
视频显示，主要是利用GTK图形库构建图形界面，以及将视频数据显示在窗口上.
初始化图形显示界面了，主要是建立窗口，设置属性，定义信号链接函数。
一切就绪后，进入GTK窗体主循环。
利用了一个全局变image_ready 进行两个线程之间的同步。
*/
#include <cairo.h>    // 绘图所需要的头文件
#include <gtk/gtk.h>

int startx = 0;  
int w = 400;  
int h = 300;  

/*
 这个线程很简单，判断image_ready，如果被置1那么就调用gtk_widget_queue_draw函数，触发widget的‘expose-event’事件，
 从而执行相关处理函数。这个widget参数是gtk控件drawingarea,视频就是显示在这个控件上。在窗口初始化的时候定义了这个
 控件，并初始化了控件的'expose-event'事件的处理函数为on_darea_expose()，这个函数调用了GTK提供的RGB绘图函数
 gdk_draw_rgb_image（）将缓冲区的内容绘制到屏幕上。
        在设备初始化之后，两个线程通过image_ready进行同步，视频采集进程默默的采集数据，每采回一帧数据，都会调用
        process_image，对数据进行处理，而process_image处理完数据后置位image_ready，然后视频显示线程将视频显示在
        屏幕上，同时清零image_ready，准备下一次转换。
*/
#if 0
static void draw_thread(GtkWidget *widget)
{
        for(;;) {  
                g_usleep(10000);  
        if (quit_flag == 1) break;  
                gdk_threads_enter();  
                if(image_ready) {  
                        gtk_widget_queue_draw(GTK_WIDGET (widget));  
                }  
                else {  
                        gdk_threads_leave();  
                }  
                gdk_threads_leave();  
        }  
}
#endif


// 绘图事件
gboolean on_expose_event (GtkWidget * widget, GdkEventExpose *event, gpointer data)
{
    cairo_t *cr = gdk_cairo_create(widget->window);  // 创建cairo环境，注意参数  
  
    // 画背景图  
    // 获取图片  
    GdkPixbuf *src_pixbuf = gdk_pixbuf_new_from_file("./back.jpg", NULL);   
    // 指定图片大小  
    GdkPixbuf* dst_pixbuf = gdk_pixbuf_scale_simple(src_pixbuf, w, h, GDK_INTERP_BILINEAR);  
      
    // dst_pixbuf作为cr环境的画图原材料，(0, 0)：画图的起点坐标  
    gdk_cairo_set_source_pixbuf(cr, dst_pixbuf, 0, 0);  
    cairo_paint(cr);    // 绘图  
      
    // 释放资源  
    g_object_unref(dst_pixbuf);  
    g_object_unref(src_pixbuf);  
  
    // 画笑脸  
    src_pixbuf = gdk_pixbuf_new_from_file("./face.png", NULL);  
    dst_pixbuf = gdk_pixbuf_scale_simple(src_pixbuf, 80, 80, GDK_INTERP_BILINEAR);  
    gdk_cairo_set_source_pixbuf(cr, dst_pixbuf, startx, (h/10)*3);  
    cairo_paint(cr);  
    g_object_unref(dst_pixbuf);  
    g_object_unref(src_pixbuf);  
  
    /* 
    // 绘图与写字共存的测试 
    // 如果绘完图片后想继续写字或画线， 
    // 必须手动设置画笔颜色cairo_set_source_rgb() 
    // 否则，字体或线条会被图片覆盖。 
    cairo_set_source_rgb(cr, 0.627, 0, 0);  // 设置字体颜色 
    cairo_set_font_size(cr, 40.0);          // 设置字体大小 
    cairo_move_to(cr, 50.0, 130.0);         // 写字的起点坐标 
    cairo_show_text(cr, "This is a test");  // 写字 
    */  
  
    cairo_destroy(cr);  // 回收所有Cairo环境所占用的内存资源  
  
    return FALSE;   // 必须返回FALSE  
}

// 按钮按下回调函数
void deal_button_clicked(GtkWidget *widget, gpointer data)
{
    startx += 20;
    if(startx >= w){
        startx = 0;  
    }
  
    gtk_widget_queue_draw( GTK_WIDGET(data) );  // 更新刷图区域，刷新整个窗口  
}

int main(int argc, char **argv)
{
#if 0
    /* 
     * init struct camera  
     */  
    struct camera *cam;  
      
    cam = malloc(sizeof(struct camera));  
        //分配内存  
    if (!cam) {   
        printf("malloc camera failure!\n");  
        exit(1);  
    }  
    cam->device_name = "/dev/video0";  
        //在ubuntu下，我的摄像头对应的就是这个设备  
    cam->buffers = NULL;  
    cam->width = 320;  
    cam->height = 240;  
        //我的摄像头质量比较差，最大分辨率只有320*240  
    cam->display_depth = 3;  /* RGB24 */  
    cam->rgbbuf = malloc(cam->width * cam->height * cam->display_depth);  
  
  
    if (!cam->rgbbuf) {   
        printf("malloc rgbbuf failure!\n");  
        exit(1);  
    }  
    open_camera(cam); //打开设备  
    get_cam_cap(cam); //得到设备信息，如果定义了DEBUG_CAM，则会打印视频信息  
    get_cam_pic(cam); //得到图形信息，同样如果定义了DEBUG_CAM，则会打印信息  
    get_cam_win(cam); //得到视频显示信息  
    cam->video_win.width = cam->width;  
    cam->video_win.height = cam->height;  
    set_cam_win(cam);    
        //设置图像大小，视频显示信息中包括摄像头支持的最大分辨率以及最小分辨率，这个可以设置，我设置的是320×240,当然也可以设置成其他，不过只能设置成特定的一些值  
    get_cam_win(cam);  
    //显示设置之后的视频显示信息，确定设置成功    
        init_camera(cam);  
        //初始化设备，这个函数包括很多有关v4l2的操作  
        start_capturing (cam);  
    //打开视频采集  
#endif

#if 0
    gtk_window_init(argc,argv,cam);  //初始化图形显示  
        
  
    g_thread_create((GThreadFunc)draw_thread, drawingarea, FALSE, NULL);  
    g_thread_create((GThreadFunc)capture_thread, cam, FALSE, NULL);  
        //建立线程  
  
    gdk_threads_enter();  
    gtk_main();  
    gdk_threads_leave();  
        //进入主循环之后，两个线程开始工作  
#endif

	GtkWidget    *window;
   GtkWidget    *label;
   gtk_init(&argc,&argv);
   window = gtk_window_new(GTK_WINDOW_TOPLEVEL);	// 顶层窗口  
   gtk_window_set_title(GTK_WINDOW(window),"Hello World");
   g_signal_connect(window,"destroy",G_CALLBACK(gtk_main_quit),NULL);
   
   gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);    // 中央位置显示
   gtk_widget_set_size_request(window, 400, 300);          // 窗口最小大小
   //  gtk_window_set_default_size(window,600,700);   
   //-     gdk_window_maximize(window); 	//-目前使用这个出错
//-   gtk_window_set_resizable(GTK_WINDOW(window), FALSE);    // 固定窗口的大小
	GtkWidget *table = gtk_table_new(5, 5, TRUE);   // 表格布局容器
	gtk_container_add(GTK_CONTAINER(window), table); // 容器加入窗口
	
	
	
	// button
	GtkWidget *button = gtk_button_new_with_label("click me");      // 按钮
	g_signal_connect(button, "clicked", G_CALLBACK(deal_button_clicked), window);
	gtk_table_attach_defaults(GTK_TABLE(table), button, 3, 4, 4, 5);// 把按钮加入布局
	
	// 绘图事件信号与回调函数的连接
	g_signal_connect(window, "expose-event", G_CALLBACK(on_expose_event), NULL);
	gtk_widget_set_app_paintable(window, TRUE); // 允许窗口可以绘图
   
   label = gtk_label_new("Hello, World");
   gtk_container_add(GTK_CONTAINER(window),label);
   gtk_widget_show_all(window);	// 显示所有控件
   gtk_main();
    return 0;  

  
 
#if 0
 
    
#endif  

}

   
