
struct camera {
    char *device_name;  
    int fd;  
    int width;  
    int height;  
    int display_depth;  
    int image_size;  
    int frame_number;  
    struct video_capability video_cap;  
    struct v4l2_capability v4l2_cap;  
    struct v4l2_cropcap v4l2_cropcap;  
    struct v4l2_format v4l2_fmt;  
    struct video_window video_win;  
    struct video_picture video_pic;  
    struct buffer *buffers;  
    unsigned char *rgbbuf;  
};
