

int read_frame(struct camera *cam)
{
        struct v4l2_buffer buf;  
  
    CLEAR (buf);  
        //这是自定义的一个宏，调用memset对内存清零  
  
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;  
        buf.memory = V4L2_MEMORY_MMAP;  
    if (quit_flag == 0) {  
            if (-1 == xioctl (cam->fd, VIDIOC_DQBUF, &buf)) {  
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
    }  
  
        assert (buf.index < n_buffers);  
  
    process_image (cam->buffers[buf.index].start, cam);  
  
    if (quit_flag == 0) {  
        if (-1 == xioctl (cam->fd, VIDIOC_QBUF, &buf))  
            errno_exit ("VIDIOC_QBUF");  
    }  
    return 1;  
}

static void capture_thread(struct camera *cam)
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
  
        r = select (cam->fd + 1, &fds, NULL, NULL, &tv);  
  
        if (-1 == r) {  
                if (EINTR == errno)  
                    continue;  
                errno_exit ("select");  
        }  
  
        if (0 == r) {  
                fprintf (stderr, "select timeout\n");  
                exit (EXIT_FAILURE);  
        }  
  
        if (read_frame (cam))  
                    gdk_threads_leave();  
        /* EAGAIN - continue select loop. */  
    }     
}
