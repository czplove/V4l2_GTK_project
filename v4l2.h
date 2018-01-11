#ifndef _V4L2_H
#define _V4L2_H
//-#include <linux/videodev.h>
#include <linux/videodev2.h>

//-#define DEBUG_CAM	0	
//-#define RUN_CAM
#define RUN_SHOW

struct buffer {
        void *  start;
        size_t  length;
};

extern unsigned int quit_flag;
struct camera {
	char *device_name;	//-记录摄像头设备的名称，如"/dev/video0"
	int fd;			//-是设备打开后返回的文件描述符
	int width;		//-摄像头采集视频的宽度
	int height;		//-摄像头采集视频的高度
	int display_depth;	//- 显示屏幕的分辨率，以字节为单位，我的显示屏为3,也就是分辨率为24
	int image_size;		//-摄像头采集视频的大小，为width*height*display_depth
   	int frame_number;	//-视频缓冲区标号，在视频采集的时候需要开辟多个缓冲区，这个表示缓冲区的个数
	//-struct video_capability video_cap;	//-是video_capability结构体，主要定义了视频设备的一些信息，通过ioctl命令可以从设备读出这个信息。
	struct v4l2_capability v4l2_cap;	//-是v4l2_capability 结构体，同样定义了一些视频设备的信息与video_capability不同，他是v4l2接口的。但是我发现他缺少video_capability的一些内容，所以还是定义了video_capability  这样两种接口混用了，不过既然v4l2支持设备返回video_capability，这样也没什么不妥。
	struct v4l2_cropcap v4l2_cropcap;	//-是v4l2_cropcap结构体，在操作视频缓冲区的时候使用
	struct v4l2_format v4l2_fmt;		//-是v4l2_format结构体，主要定义了视频显示的一些属性
	//-struct video_window video_win;	//-是video_window结构体，主要定义了视频格式，如高度，宽度等
	//-struct video_picture video_pic;	//-是video_picture结构体，主要定义画面的属性，如亮度，灰度，饱和度等
	struct buffer *buffers;			//-是自定义的struct buffer结构体，包括是频缓冲区的开始地址，以及大小
	unsigned char *rgbbuf;			//-视频缓冲区指针，显示程序就是在这里读取数据的。
};
void process_image(unsigned char *buf, struct camera *cam);
void errno_exit(const char *s);
void open_camera(struct camera *cam);
void close_camera(struct camera *cam);
int read_frame(struct camera *cam);
void start_capturing(struct camera *cam);
void stop_capturing(struct camera *cam);
void uninit_camera(struct camera *cam);
void init_camera(struct camera *cam);
void get_cam_cap(struct camera *cam);
void get_cam_pic(struct camera *cam);
void set_cam_pic(struct camera *cam);
void get_cam_win(struct camera *cam);
void set_cam_win(struct camera *cam);

#endif
