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
	char *device_name;	//-��¼����ͷ�豸�����ƣ���"/dev/video0"
	int fd;			//-���豸�򿪺󷵻ص��ļ�������
	int width;		//-����ͷ�ɼ���Ƶ�Ŀ��
	int height;		//-����ͷ�ɼ���Ƶ�ĸ߶�
	int display_depth;	//- ��ʾ��Ļ�ķֱ��ʣ����ֽ�Ϊ��λ���ҵ���ʾ��Ϊ3,Ҳ���Ƿֱ���Ϊ24
	int image_size;		//-����ͷ�ɼ���Ƶ�Ĵ�С��Ϊwidth*height*display_depth
   	int frame_number;	//-��Ƶ��������ţ�����Ƶ�ɼ���ʱ����Ҫ���ٶ���������������ʾ�������ĸ���
	//-struct video_capability video_cap;	//-��video_capability�ṹ�壬��Ҫ��������Ƶ�豸��һЩ��Ϣ��ͨ��ioctl������Դ��豸���������Ϣ��
	struct v4l2_capability v4l2_cap;	//-��v4l2_capability �ṹ�壬ͬ��������һЩ��Ƶ�豸����Ϣ��video_capability��ͬ������v4l2�ӿڵġ������ҷ�����ȱ��video_capability��һЩ���ݣ����Ի��Ƕ�����video_capability  �������ֽӿڻ����ˣ�������Ȼv4l2֧���豸����video_capability������Ҳûʲô���ס�
	struct v4l2_cropcap v4l2_cropcap;	//-��v4l2_cropcap�ṹ�壬�ڲ�����Ƶ��������ʱ��ʹ��
	struct v4l2_format v4l2_fmt;		//-��v4l2_format�ṹ�壬��Ҫ��������Ƶ��ʾ��һЩ����
	//-struct video_window video_win;	//-��video_window�ṹ�壬��Ҫ��������Ƶ��ʽ����߶ȣ���ȵ�
	//-struct video_picture video_pic;	//-��video_picture�ṹ�壬��Ҫ���廭������ԣ������ȣ��Ҷȣ����Ͷȵ�
	struct buffer *buffers;			//-���Զ����struct buffer�ṹ�壬������Ƶ�������Ŀ�ʼ��ַ���Լ���С
	unsigned char *rgbbuf;			//-��Ƶ������ָ�룬��ʾ��������������ȡ���ݵġ�
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
