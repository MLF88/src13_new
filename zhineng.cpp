#include "zhineng.hpp"
#include "motorx.hpp"

extern cv::Mat myframe;
extern pthread_mutex_t mut_myframe;

extern int track_flags;//开启追踪算法并控制相机的标记；1开启，0关闭；
extern FILE *fpsv_pos;
extern int cam_box_count;
extern int insu_v_right;
//struct target gx[7];

//等待循环；
void mydelay(void)//10ms忙等待；
{
    int i;
    for(i=0;i<2000000;i++)
    {

    }
}
//打印时间；
void printf_time(clock_t timestart)
{
    clock_t end,real;
    long int clktck = 0;
    if(clktck == 0)
        if((clktck = sysconf(_SC_CLK_TCK)) < 0)
            fprintf(stderr,"sysconf error %d, %s\n",errno,strerror(errno));
    
    printf("clktck:%ld\n",clktck);
    end = times(NULL);
    real = end-timestart;
    printf("real:%7.2f\n",real/(double)clktck);
}
//获取矩形中心点；
cv::Point getCenterPoint(cv::Rect rect)
{
	cv::Point cpt;
	cpt.x = rect.x+cvRound(rect.width/2.0);
	cpt.y = rect.y+cvRound(rect.height/2.0);
	return cpt;
}

//初始化串口文件；115200,8位数据，1位停止，没有校验；
static int set_attr(int me_uart2)
{
    struct termios newtio;
	int inputspeed;
	//int outputspeed;
    tcgetattr(me_uart2,&newtio);
    memset(&newtio,0,sizeof(newtio));
	inputspeed = cfsetispeed(&newtio,B115200);//设置读波特率115200；
	if(inputspeed < 0)
	{
		perror("cfsetispeed()");
		printf("set speed is error\n");
	}
	else
	{
		printf("inputspeed = %d\n",inputspeed);
	}
	if(cfsetospeed(&newtio,B115200) < 0) //设置写波特率为115200；
	{
		perror("cfsetospeed()");
		printf("set speed is error\n");
    }
    newtio.c_cflag |= (CLOCAL | CREAD);
	newtio.c_cflag &= ~CSIZE;
    newtio.c_cflag |= CS8;//设置数据位 为8位，并且使能接收；
    newtio.c_cflag &= ~PARENB;//不进行校验；
	newtio.c_iflag &= ~INPCK;//不进行校验；
    newtio.c_cflag &= ~CSTOPB;//1位停止位；
	newtio.c_cflag &= ~CRTSCTS;//do not use stream control
	newtio.c_oflag &= ~OPOST;
	newtio.c_lflag &= ~(ICANON|ECHO|ECHOE|ISIG);
	newtio.c_oflag &= ~(ONLCR|OCRNL);
	newtio.c_iflag &= ~(INLCR|ICRNL|IGNCR);
    newtio.c_cc[VMIN] = 1;//当接收到一个字节数据就读取；
    newtio.c_cc[VTIME] = 0;//不使用计数器；
    tcflush(me_uart2,TCIOFLUSH); //刷清输入输出缓冲区；
    tcsetattr(me_uart2,TCSANOW,&newtio);//使用设置的终端属性立即生效

	return 0;
}

//串口文件打开并初始化设置；return fd;
int my_uart_init(const char *uart_name)
{
    int fd;
    fd=open(uart_name,O_RDWR|O_NOCTTY|O_NDELAY);
    if(fd<0)
    {
        perror("open()");
        return 0;
    }
    set_attr(fd);
    printf("child_track-do-work(1)\n");
    
    return fd;
}

//发送数据;
//控制相机俯仰，航向；0x31：俯仰，0x32：航向，0x03:变焦 加入ID，0x04:拍照；0x05:关机；
//fd: 串口文件；cmd：控制命令（俯仰，航向）；data: pid参数值；camdata: 一组串口数据；int size: 变焦倍数；
//将俯仰和航向数据一起发送；
//发送给app识别目标的标志；
int snd_uart(int me_uart2, unsigned char cmd, int data, int size, unsigned char id, unsigned char cmd2, int data2)
{
    int len;
    unsigned char camdata[15];
    memset(camdata,0,sizeof(camdata));
    camdata[0]=0xa5;
    camdata[2]=cmd;
    if(cmd2 == 0)   //发送单指令;变焦，拍照，拍照完成回馈指令，俯仰航向；
    {
        unsigned char c1 = data&0xff;//l;
        unsigned char c2 = (data >> 8)&0xff;//h;
        unsigned char c3 = size&0xff;//l,camsize;
    
        switch (cmd)
        {
            //航向，俯仰；
            case 49:
            case 50:
                camdata[1]=(sizeof(c1)+sizeof(c2)+sizeof(cmd));
                camdata[3]=c2;
                camdata[4]=c1;
                camdata[5]=((cmd+c1+c2)/256);
                camdata[6]=((cmd+c1+c2)%256);
                printf("**************snd_uart*********************************\n"); 
                break;
            //变焦；
            case 3:
                camdata[1]=(sizeof(c1)+sizeof(c2)+sizeof(c3)+sizeof(cmd)+sizeof(id));
                camdata[3]=c2;
                camdata[4]=c1;
                camdata[5]=c3;
                camdata[6]=id;
                camdata[7]=((cmd+c1+c2+c3+id)/256);
                camdata[8]=((cmd+c1+c2+c3+id)%256);
                break;
            //拍照,关机；拍照完成回馈指令；
            case 4:
                camdata[1]=(sizeof(cmd)+sizeof(cmd));
                camdata[3]=data;//0x00;
                camdata[4]=((cmd+data)/256);
                camdata[5]=((cmd+data)%256);
                break;
            //发送给app的目标识别的数量；
            case 33:
                camdata[1]=(sizeof(cmd)+sizeof(cmd));
                camdata[3]=data;
                camdata[4]=((cmd+data)/256);
                camdata[5]=((cmd+data)%256);
                break;

            default:
                fprintf(stderr,"Unsupported parity\n");
                break;
        }
        len = write(me_uart2, camdata, sizeof(camdata));
        if(len < 5)
        {
            printf("send data to control camera or to app is failed\n");
            return 0;
        }
        return 0;
    }
    else            //发送俯仰和航向双指令；
    {
        unsigned char c1 = data&0xff;//l;
        unsigned char c2 = (data >> 8)&0xff;//h;
        unsigned char c3 = data2&0xff;//l;
        unsigned char c4 = (data2 >> 8)&0xff;//h;
    
        camdata[1]=(sizeof(c1)+sizeof(c2)+sizeof(cmd));
        camdata[3]=c2;
        camdata[4]=c1;
        camdata[5]=((cmd+c1+c2)/256);
        camdata[6]=((cmd+c1+c2)%256);
    
        camdata[7]=0xa5;
        camdata[8]=camdata[1];
        camdata[9]=cmd2;
        camdata[10]=c4;
        camdata[11]=c3;
        camdata[12]=((cmd2+c3+c4)/256);
        camdata[13]=((cmd2+c3+c4)%256);
        
        len = write(me_uart2, camdata, sizeof(camdata));
        if(len < 13)
        {
            printf("send data to control camera\n");
            return 0;
        }
        return 0;
    }
    
}

//分析数据;
//Pact_Analysis 返回值为5时代表要结束吊舱程序;
//return 5,结束吊舱；return 6,关闭TX1电源；
//return 1,结束目标追踪并标记设置0,return 2,开启目标追踪并标记设置1;

int Pact_Analysis(unsigned char *pact, int *camsize_area)
{
	int check = 0;
    int i = 0;
    int pwm_zoom;
    int cmdata = camsize_area[0];
    
    //int *cmsize_are = camsize_area;

	if(pact[0] == 0xa5)
	{
		for(i = 0; i < pact[1]; i++)
		{
			check += pact[i + 2];
		}
		if(((check / 256) == pact[pact[1] + 2]) && ((check % 256) == pact[pact[1] + 3]))
		{
			if(pact[2] == 0x20) //接收开始追踪信号，
			{
                if(pact[3] == 0x01) //
                {
                    return 1;
                }
                else if(pact[3] == 0x00) //
                {
                    return 2;
                }
                else if(pact[3] == 0x05)//结束吊舱程序;
                {
                    printf("结束吊舱********pact[3]=%d\n",pact[3]);
                    return pact[3];
                }
                else if(pact[3] == 0x06)//关闭TX1电源；
                {
                    printf("关闭tx1电源********pact[3]=%d\n",pact[3]);
                    return pact[3];
                }				
			} 
			else if(pact[2] == 0x03)//接收变焦倍数,重置pid参数；
			{
                pwm_zoom = pact[3] * 256 + pact[4];

                if(pwm_zoom == 1200)  //缩小焦距;
                {
                    if((cmdata - pact[5]) > 0)
                        cmdata -= pact[5];
					else if((cmdata - pact[5]) <= 0)
                        cmdata = 1;
                    camsize_area[0] = cmdata;    
					fprintf(fpsv_pos,"***cmdata=%d****\n",cmdata);
					fflush(fpsv_pos);
                    return 0;
                }
                else if(pwm_zoom == 1800) //放大焦距;
                {   
					if((cmdata + pact[5]) < 30)
                        cmdata += pact[5];	
					else if((cmdata + pact[5]) >= 30)
                        cmdata = 30;

                    camsize_area[0] = cmdata;    	 
					fprintf(fpsv_pos,"***cmdata=%d****\n",cmdata);
					fflush(fpsv_pos);
                    return 0;
                }
            }
        }
	}
	return 0;
}

//接受数据;
//return 5,结束吊舱；return 6,关闭TX1电源；
//return 1,结束目标追踪并标记设置0,return 2,开启目标追踪并标记设置1;
int rcv_uart(int me_uart2,int *camsize_area)
{
    int return_value;
	int len;
	unsigned char camdata[11];
	memset(camdata,0,sizeof(camdata));
    len=read(me_uart2,camdata,10*sizeof(char));
	if(len<5)
		return 0;

    return_value=Pact_Analysis(camdata, camsize_area);

	return return_value;
}

static int cam_size_per_call(int max_cam,float per_tar)
{
	printf("cam_size_per_call max_cam=%d,per_tar=%f\n",max_cam,per_tar);
    int ret=0;

	int tar=(int)(per_tar*10);

    switch(tar)
    {
		case 4:
			ret = max_cam-3;
			break;
        case 5:
            ret = max_cam-2;
            break;
        case 6:
            ret = max_cam-1;
            break;
        case 7:
            ret = max_cam;
            break;
        default:
            fprintf(stderr,"Unsupported parity\n");
            break;
    }
    
    printf("cam_size_per_call ret=%d\n",ret);

    if(ret >= 0)
        return ret;
    else
        return 0;
}
//返回需要变焦的倍数;
static int cam_size_per (int cam_size, float percent_img, float percent_tar)
{
    int ret;

    if(cam_size < 3)
        ret = 3;
    else if((cam_size >= 3) && (cam_size < 30))
    {
        if ( percent_img < 0.10 )
        {
            ret = 4;
        }
        else if ( (percent_img >= 0.10) && (percent_img < 0.20) )
        {
            if((int)(percent_tar*10) == 3)
                return 3;
            else 
                return 4;
        }
        else if ( (percent_img >= 0.20) && (percent_img < 0.30) )
        {
            if((int)(percent_tar*10) == 5)
                return 5;
            else if(percent_tar > 0.5)
                return 6;
            else if((int)(percent_tar*10) == 4)
                return 3;
            else if((int)(percent_tar*10) == 3)
                return 2;
        }
        else if ( (percent_img >= 0.30) && (percent_img < 0.40) )
        {
            ret = cam_size_per_call(5, percent_tar);
		}
        else if ( (percent_img >= 0.40) && (percent_img < 0.50) )
        {
            ret = cam_size_per_call(4, percent_tar);   
        }
        else if ( (percent_img >= 0.50) && (percent_img < 0.60) )
        {
            ret = cam_size_per_call(3, percent_tar);
        }
        else if ( (percent_img >= 0.60) && (percent_img < 0.70) )
        {
            ret = cam_size_per_call(2, percent_tar);
        }
        else if ( ( percent_img >= 0.70 ) && (percent_img < 0.80) )
        {
            ret = 0;
        }
        else if ( (percent_img >= 0.80 ) && (percent_img < 0.90) )
        {
            ret = -2;
        }
        else if ( percent_img >= 0.90 ) 
        {
            ret = -3;
        }
        else 
            ret = 0;
    }
    else
        ret = 0;
	
    return ret;    

} 

//输入当前相机倍数，感兴趣区域坐标；返回在此基础上需要变焦的倍数，以达到已定画幅比例;
//percent_tar 指定的目标框与画幅的比例;
//int cam_size_comput(int cam_size,cv::Rect trackbox,float *percent_tar)
int cam_size_comput(int cam_size, cv::Rect trackbox, struct target *camera_obj)
{
	printf("cam_size=%d,trackbox.height=%d,trackbox.width=%d\n",cam_size,trackbox.height,trackbox.width);
    int size;
    float percent_height, img_height=720.00, percent_width, img_width=720.00;

    percent_height = trackbox.height/img_height;
    percent_width = trackbox.width/img_width;
    printf("percent_height=%f,percent_width=%f\n",percent_height,percent_width);
    if(trackbox.width > trackbox.height)          //width > height;width:720
    {
        size = cam_size_per(cam_size, percent_width, camera_obj->image_lim[0]);
        return size;
    }
    else if(trackbox.width <= trackbox.height)    //width < height; height:720
    { 
        size = cam_size_per(cam_size, percent_height, camera_obj->image_lim[1]);
		printf("cam_size_comput_size=%d\n",size);
        return size;
    }

    return 0;
}

//变焦后返回延时的时间;
static int insulator_cam_size(int size)
{
    int ret;

    switch (size)
    {
        case 1:
        case 2:
        case 3:
        case 4:
        {
            ret = size;
            break;
        }
        case 5:
        {
            ret = size - 2;
            break;
        }
        case 6:
        {
            ret = size - 2;
            break;
        }
        case 7:
        {
            ret = size - 3;
            break;
        }
        case 8:
        {
            ret = size - 4;
            break;
        }
        case 9:
        {
            ret = size - 4;
            break;
        }
        case 10:
        {
            ret = size - 5;
            break;
        }
        case 11:
        case 12:
        case 13:
        case 14:
        case 15:
        case 16:
        {
            ret = 6;
            break;
        }
        default:
                fprintf(stderr,"Unsupported parity\n");
                break;
    }
    
    return ret;

}
//me_fd 串口文件; arrar 数组包含pwm值和时间; size 挂点需要放大的焦距;
//完成 移动绝缘子, 停止移动， 放大焦距， 连拍照3张, 缩小相同焦距;;
static int insulator_cam_call(int me_fd, int *arrar, int size)
{
    snd_uart(me_fd, 0x31, arrar[0], 0, 0, 0x32, 1500);//移动;
    usleep((arrar[1]*1000));//延时;
    snd_uart(me_fd, 0x31, 1500, 0, 0, 0x32, 1500);//停止;
    //usleep(200000);
    sleep(6);
    snd_uart(me_fd, 0x03, 1800, size, 0x03, 0, 0);//放大焦距;
    sleep(insulator_cam_size(size));//延时;
	sleep(7);
    //snd_uart(me_fd, 0x04, 0, 0, 0, 0, 0);//拍照;
    for(int k=0; k<3; k++)
	{
        snd_uart(me_fd, 0x04, 0, 0, 0, 0, 0);//拍照;
		sleep(7);
	}
    //sleep(10);//延时;
    snd_uart(me_fd, 0x03, 1200, size, 0x03, 0, 0);//缩小焦距;
    sleep(insulator_cam_size(size));//延时;

	return 0;
}

//相机 移动， 停止， 放大; arrar[0] pwm值，arrar[1] 移动花费时间;
// size 需要变化的焦距;
static int insulator_cam_act_y(int me_fd, int *arrar, int size)
{
    snd_uart(me_fd, 0x31, arrar[0], 0, 0, 0x32, 1500);//移动;
    usleep((arrar[1]*1000));//延时;
    snd_uart(me_fd, 0x31, 1500, 0, 0, 0x32, 1500);//停止;
    //sleep(2);
    sleep(1);
    if(size != 0)
    {
        snd_uart(me_fd, 0x03, 1800, size, 0x03, 0, 0);//放大焦距;
        sleep(insulator_cam_size(size));//延时;
    }
    //sleep(2); 
	return 0;
}

//相机 移动， 停止， 放大; arrar[0] pwm值，arrar[1] 移动花费时间;
// size 需要变化的焦距;
static int insulator_cam_act_x(int me_fd, int *arrar, int size)
{
    snd_uart(me_fd, 0x31, 1500, 0, 0, 0x32, arrar[0]);//移动;
    usleep((arrar[1]*1000));//延时;
    snd_uart(me_fd, 0x31, 1500, 0, 0, 0x32, 1500);//停止;
    //sleep(2);
    sleep(1);
    if(size != 0)
    {
        snd_uart(me_fd, 0x03, 1800, size, 0x03, 0, 0);//放大焦距;
        sleep(insulator_cam_size(size));//延时;
    }
    //sleep(2); 
	return 0;
}

//me_fd 串口文件， trackbox 目标框，size 需要焦距;
//me_camsize[0] 相机焦距倍数,实际变焦倍数。
int insulator_cam_height(int me_fd, int data, int *me_camsize, int size, int direction)
{
    int movcam[2];
    //printf("me_trackbox.height=%d,me_camsize=%d\n",me_trackbox.height,me_camsize);

    //调用pix_mov函数，将pwm,time 存入数组movcam中；
    //计算移动数据;
    //pix_movey(((me_trackbox.height*3)/10), 1, me_camsize[0], movcam);

    pix_movey(data, direction, me_camsize[0], movcam);

    //pix_move((me_trackbox.height/2), 1, me_camsize, movcam);//向下移动;
    //printf("movcam[0]=%d,mocam[1]=%d\n",movcam[0],movcam[1]);

    //me_camsize[0] += size; //同步焦距;
	if( (me_camsize[0]+size) >=26)
	{
		size = 26 - me_camsize[0];
		me_camsize[0] = 26;
	}
	else
	{
		me_camsize[0] += size; //同步焦距;
	}
    //根据计算的数据对相机调整，
    insulator_cam_act_y(me_fd, movcam, size);

    return size;
}
int insulator_cam_width(int me_fd, int data, int *me_camsize, int size, int direction)
{
    int movcam[2];
    //printf("me_trackbox.height=%d,me_camsize=%d\n",me_trackbox.height,me_camsize);

    //调用pix_mov函数，将pwm,time 存入数组movcam中；
    //计算移动数据;
    //pix_movey(((me_trackbox.height*3)/10), 1, me_camsize[0], movcam);

    pix_movex(data, direction, me_camsize[0], movcam);

    //pix_move((me_trackbox.height/2), 1, me_camsize, movcam);//向下移动;
    //printf("movcam[0]=%d,mocam[1]=%d\n",movcam[0],movcam[1]);

    //me_camsize[0] += size; //同步焦距;
	if( (me_camsize[0]+size) >=26)
	{
		size = 26 - me_camsize[0];
		me_camsize[0] = 26;
	}
	else
	{
		me_camsize[0] += size; //同步焦距;
	}
    //根据计算的数据对相机调整，
    insulator_cam_act_x(me_fd, movcam, size);

    return size;
}
double tracking(cv::Mat &templateImage_bak, cv::Rect &trackBox) //myframe是全局变量；
{  
    cv::Mat gray,templateImage; 

    //灰度处理;
    cvtColor(templateImage_bak, templateImage, CV_RGB2GRAY);
    //枷锁myframe； 
    pthread_mutex_lock(&mut_myframe);
    while(myframe.empty())
    {
		printf("@@@@@@@@@@@@@@@@@@@@@@@\n");
        pthread_mutex_unlock(&mut_myframe);
        sched_yield();
        pthread_mutex_lock(&mut_myframe);     
    }
    cvtColor(myframe, gray, CV_RGB2GRAY); 
    pthread_mutex_unlock(&mut_myframe); 
    //解锁myframe；

    //printf("trackBox.x=%d,trackBox.y=%d\n," trackBox.x,trackBox.y);
    //printf("trackBox.width=%d,trackBox.height=%d\n," trackBox.width,trackBox.height);
    int regional_coefficient = 20; //目标框长宽增量；为搜索区域；
    int trackbox_top_left_offset=10;//目标框左上角点坐标偏移量；为搜索区域；
    cv::Point pic2 = getCenterPoint(trackBox);
    cv::Point pic1 = cv::Point((IMGWID/2),(IMGHEI/2));
    //定义搜索区域；
    cv::Rect searchWindow;
    searchWindow.width = trackBox.width + regional_coefficient + 10;  
    searchWindow.height = trackBox.height + regional_coefficient + 10;
    if((pic2.x <= pic1.x) && (pic2.y <= pic1.y))//第一区间，中心点不变；
    {
        //printf("第一区间，pic2.x=%d,pic2.y=%d\n",pic2.x,pic2.y);
        searchWindow.x = trackBox.x - trackbox_top_left_offset;
        if(searchWindow.x < 0)
            searchWindow.x = 0;
        searchWindow.y = trackBox.y - trackbox_top_left_offset;
        if(searchWindow.y < 0)
            searchWindow.y = 0;
    }
    else if((pic2.x > pic1.x) && (pic2.y <= pic1.y))
    {
        //printf("第二区间，pic2.x=%d,pic2.y=%d\n",pic2.x,pic2.y);
        searchWindow.x = trackBox.x - regional_coefficient;
        searchWindow.y = trackBox.y - trackbox_top_left_offset;
        if(searchWindow.y < 0)
            searchWindow.y = 0;
    }
    else if((pic2.x > pic1.x) && (pic2.y > pic1.y))
    {
        //printf("第三区间，pic2.x=%d,pic2.y=%d\n",pic2.x,pic2.y);
        searchWindow.x = trackBox.x - regional_coefficient;
        searchWindow.y = trackBox.y - regional_coefficient;              
    }
    else if((pic2.x <= pic1.x) && (pic2.y > pic1.y))
    {
        //printf("第四区间，pic2.x=%d,pic2.y=%d\n",pic2.x,pic2.y);
        searchWindow.x = trackBox.x - trackbox_top_left_offset;
        if(searchWindow.x < 0)
            searchWindow.x = 0;
        searchWindow.y = trackBox.y - regional_coefficient;
    }
    //图像边界约束；搜索区域要小于图像边界；
    if((searchWindow.x + searchWindow.width) >= gray.cols) //x轴，宽度边界约束；
        searchWindow.width = gray.cols - searchWindow.x;
    if((searchWindow.y + searchWindow.height) >= gray.rows) //y轴，高度边界约束；
        searchWindow.height = gray.rows - searchWindow.y;

    //printf("searchWindow.x=%d,searchWindow.y=%d\n",searchWindow.x,searchWindow.y);
    //printf("searchWindow.width=%d,searchWindow.height=%d\n",searchWindow.width,searchWindow.height);
    int result_cols = searchWindow.width - templateImage.cols + 1;  //???
    int result_rows = searchWindow.height - templateImage.rows + 1; //???
    cv::Mat result = cv::Mat( result_cols, result_rows, CV_32FC1);

    /*******************************/
    matchTemplate(gray(searchWindow), templateImage, result, CV_TM_CCOEFF_NORMED);   
      
    double minVal, maxVal;
    cv::Point minLoc, maxLoc, matchLoc;
  
    minMaxLoc( result, &minVal, &maxVal, &minLoc, &maxLoc, cv::Mat());
    matchLoc = maxLoc;


    //通过引用传参，更新目标框的坐标。算是本函数主要作用，并且返回匹配概率；
    trackBox.x = matchLoc.x + searchWindow.x;  
    trackBox.y = matchLoc.y + searchWindow.y;
    trackBox.width = templateImage.cols;
    trackBox.height = templateImage.rows;

    if(gray.empty())
	{
		printf("gray.empty() \n");
    }
    
    //更新匹配的区域。在这里可视察情况来取舍这句代码。这里暂时隐掉。
    //templateImage_bak = gray(trackBox);
    fprintf(fpsv_pos,"************maxVal=%f*************\n",maxVal);
	fflush(fpsv_pos);

    return maxVal;//返回匹配概率；

}

//Point pic1画幅中心点，pic2 目标框的中心点，lsarea 中心点区域范围;
//return 1符合要求，return 2不符合要求;
static int box_lim(cv::Point pic1, cv::Point pic2, int area)
{
    int ret;
    int yl,yh,xl,xr;
    yl = pic1.y-area;
    yh = pic1.y+area;
    xl = pic1.x-area;
    xr = pic1.x+area;

    if((pic2.y >= yl) && (pic2.y <= yh) && (pic2.x >= xl) && (pic2.x <= xr))
        ret = 1;// 目标框符合要求
    else
        ret = 2; //不符合要求;

    return ret;
}
//input： pv 像素实际坐标  sp 期望坐标，camsize 相机焦距，
int pix_control_x(int pv,int sp, int *camsize,int me_fd)
{
    //usleep(50000);
    //调用pix_mov函数，将pwm,time 存入数组movcam中；
    int movcam[2],speed_pwm;
    int err = pv-sp;
	printf("xerr=%d\n",err);
    if (err>camsize[1])
    {
        //目标向左移动，dir=2,
        speed_pwm = camera_pwm_x(2, camsize[0]);
        return speed_pwm;
    }
    else if(err<(-camsize[1]))
    {
        //目标向右移动，dir=1,fabs（a）
		//printf("abs(err)=%d\n",abs(err));
        speed_pwm = camera_pwm_x(1, camsize[0]);
        return speed_pwm;
    }
    return 1500;
}

//input： pv 像素实际坐标  sp 期望坐标,camsize 相机焦距，
int pix_control_y(int pv,int sp, int *camsize, int me_fd)
{
    //usleep(50000);
    //调用pix_mov函数，将pwm,time 存入数组movcam中；
    int movcam[2],speed_pwm;
    int err=pv-sp;
	printf("yerr=%d\n",err);
    if (err>camsize[1])
    {
        //目标向上移动，dir=1,
        speed_pwm = camera_pwm_y(1, camsize[0]);
        return speed_pwm;
    }
    else if(err<(-camsize[1]))
    {
        //目标向下移动，dir=2,fabs（a）
		printf("abs(err)=%d\n",abs(err));
        speed_pwm = camera_pwm_y(2, camsize[0]);
        return speed_pwm;
    }
    return 1500;
}

//同过串口向相机控制板发送pid参数；
//return 2;跳转重新读取神经网络提供的模板和坐标；
//int me_fd: 串口文件，Rect me_trackbox:目标框值,camsize_area[0] 相机焦距倍数;
//camsize_area[1] 中心点区域; int *img_lim 数组包含目标框长宽的边界;
int send_pid(int me_fd, cv::Rect me_trackbox, int *camsize_area, struct target *camera_obj, int *label_box)
{
    int pwmx,pwmy;//pid参数;
    
        //目标变焦过程记录;拍照时清零;
    static cv::Rect insu_cam;   //进入变焦模式的标记；弥补相机变焦没有反馈指示;
    
    //pic1原始图像中心点，pic2目标框中心点;
    cv::Point pic2;
    static cv::Point pic1 = cv::Point((IMGWID/2),(IMGHEI/2));
    pic2 = getCenterPoint(me_trackbox);
    //x 轴方向，pic2.x 目标框中心点x轴坐标值，pic1.x 目标框中心点x轴中心点坐标值，返回pwm值，
    pwmx = pix_control_x(pic2.x, pic1.x, camsize_area, me_fd);
    //y 轴方向，pic1.y 目标框中心点x轴坐标值，pic1.y 目标框中心点y轴中心点坐标值，返回pwm值;
    pwmy = pix_control_y(pic2.y, pic1.y, camsize_area, me_fd);
    fprintf(fpsv_pos,"pwmx=%d,pwmy=%d,camsize=%d,pic2.y=%d,pic2.x=%d,*label_box=%d\n",pwmx,pwmy,camsize_area[0],pic2.y,pic2.x,*label_box);
    fflush(fpsv_pos);

    //目标中心点是否在画幅中心点范围内;
    printf("*************sen_pid paizhao or bianjiao*****************\n");
    if( (me_trackbox.width>=camera_obj->img_limit[0])||(me_trackbox.height>=camera_obj->img_limit[1]) || (camsize_area[0] >= 26))
    {
        fprintf(fpsv_pos,"*********paizhao********\n");
        fflush(fpsv_pos);
        snd_uart(me_fd, 0x32, 1500, 0, 0, 0x31, 1500);//发送俯仰和航向停止;
        usleep(20000);
        //snd_uart(me_fd, 0x32, 1500, 0, 0, 0x31, 1500);//发送俯仰和航向停止;
        /**************拍照***********************************/
       if(*label_box==3)//拍一张绝缘子照片;
        {
            fprintf(fpsv_pos,"***track_flags=%d,take pictures of insulator**\n",track_flags);
            fflush(fpsv_pos);
            usleep(500000);
            insu_cam.height = me_trackbox.height;
            insu_cam.width = me_trackbox.width;
            //移动至下挂点，并放大8倍焦距;
            cam_box_count = 0;
            int value_focal = insulator_cam_height(me_fd, ((me_trackbox.height*5)/10), camsize_area, MANUCAM,1);
            cam_box_count += value_focal;
            //model_flags=0;
            *label_box = 2; //准备拍摄绝缘子;
            fprintf(fpsv_pos,"***frome insulator to *label_box=%d**\n",*label_box);
            fflush(fpsv_pos);
            return 2;//跳转重新读取神经网络提供的模板和坐标;
        }
        else if(*label_box==2)//拍两张下挂点照片;
        {
            //下挂点;
            for(int j=0; j<2; j++)
	        {
                snd_uart(me_fd, 0x04, 0, 0, 0, 0, 0);//拍照;
		        sleep(4);
            }
            fprintf(fpsv_pos,"***track_flags=%d,take pictures of hangingpoint**\n",track_flags);
            fflush(fpsv_pos);
            //拍照完成以后要返回到之前绝缘子拍照状态的焦距（包括自动变焦和固定变焦），tx1控制相机变焦有记录；               
            snd_uart(me_fd, 0x03, 1200, cam_box_count, 0x03, 0, 0);//缩小焦距;
            sleep(5);
            camsize_area[0] -= cam_box_count;
            cam_box_count = 0;
            fprintf(fpsv_pos,"insu_cam.height=%d,insu_cam.width=%d\n",insu_cam.height,insu_cam.width);
            fflush(fpsv_pos);
            if( (insu_cam.height/insu_cam.width) < 2 ) // V串绝缘子;
            {
                //将相机右移动1/2宽度，画幅左移;
                insulator_cam_width(me_fd, ((insu_cam.width*5)/10), camsize_area, 0, 2);
                insu_v_right = 1;
                sleep(1);
            }
            //向下移动整个绝缘子，将绝缘子上挂点至于画幅中心，并放大7倍;
            int value_focal = insulator_cam_height(me_fd, ((insu_cam.height*10)/10), camsize_area, MANUCAM, 2);
            cam_box_count += value_focal;
            *label_box=5; //将对象切换为绝缘子上挂点;
            fprintf(fpsv_pos,"***frome uppoint to *label_box=%d**\n",*label_box);
            fflush(fpsv_pos);
			
            return 2;//跳转重新读取神经网络提供的模板和坐标;     
        }
        else if(*label_box==5)//拍摄绝缘子上挂点照片;
        {
            //绝缘子上挂点;
            for(int j=0; j<2; j++)
	        {
                snd_uart(me_fd, 0x04, 0, 0, 0, 0, 0);//拍照;
		        sleep(4);
            }
            if( (insu_cam.height/insu_cam.width) < 2 ) // V串绝缘子;
            {
                //缩小焦距至于绝缘子拍照状态;
                snd_uart(me_fd, 0x03, 1200, cam_box_count, 0x03, 0, 0);
                sleep(5);
                camsize_area[0] -= cam_box_count;
                cam_box_count = 0;
                if(insu_v_right == 1)//拍摄v串绝缘子左上挂点;
                {
                    //将相机左移动1个宽度，画幅右移;
                    insulator_cam_width(me_fd, ((insu_cam.width*10)/10), camsize_area, MANUCAM, 1);
                    sleep(5);
                    insu_v_right = 2;
                    return 2;//跳转重新读取神经网络提供的模板和坐标;
                }
                else if(insu_v_right == 2)//拍摄v串绝缘子右上挂点;
                {
                    snd_uart(me_fd, 0x03, 1200, (camsize_area[0]+1), 0x03, 0, 0);
                    insu_v_right = 0;
                }
            }
            else        //直串绝缘子;
                snd_uart(me_fd, 0x03, 1200, (camsize_area[0]+1), 0x03, 0, 0);
            //拍照完成以后要返回到之前的焦距，tx1控制相机变焦有记录;               
            track_flags=0;
            camsize_area[0] = 1;  //当前焦距,相机初始化为1倍焦距;
            cam_box_count = 0;
            sleep(10);
            //变量复位为0；全局变量；读取目标的标记变量；read_from_learning;
			snd_uart(me_fd, 0x04, 1, 0, 0, 0, 0);//拍照完成标记。
			usleep(100000);
            fprintf(fpsv_pos,"***track_flags=%d,take pictures of uppoint**\n",track_flags);
            fflush(fpsv_pos);

			return 0;//任务结束;
        }
        else if(*label_box==4)//groundpoint 地线挂点;
        {
            //地线挂点拍照;
            for(int j=0; j<2; j++)
            {
                 snd_uart(me_fd, 0x04, 0, 0, 0, 0, 0);//拍照;
                 sleep(4);
            }
            fprintf(fpsv_pos,"**track_flags=%d, take pictures of groundpoint**\n",track_flags);
            fflush(fpsv_pos);
            snd_uart(me_fd, 0x03, 1200, (camsize_area[0]+1), 0x03, 0, 0);
            track_flags=0;
            camsize_area[0] = 1;  //当前焦距,相机初始化为1倍焦距;
            cam_box_count = 0;
            sleep(9);
            snd_uart(me_fd, 0x04, 1, 0, 0, 0, 0);//拍照完成标记。
            usleep(100000);

			return 0;//任务结束;
        }
        else if(*label_box==6)//towerpeak 塔尖;
        {
            fprintf(fpsv_pos,"**track_flags=%d, take pictures of groundpoint**\n",track_flags);
            fflush(fpsv_pos);
            //移动至塔尖目标上方移动至中心点,并放大0倍焦距;
            cam_box_count = 0;
            int value_focal = insulator_cam_height(me_fd, ((me_trackbox.height*3)/10), camsize_area, 2, 2);

            cam_box_count += 2;
            camsize_area[0] += 2;
            //model_flags=0;
            fprintf(fpsv_pos,"***frome towerspeak to *label_box=%d**\n",*label_box);
            fflush(fpsv_pos);
            *label_box=4;
            return 2;//跳转重新读取神经网络提供的模板和坐标;
        }             
    }  
    else
    {
        fprintf(fpsv_pos,"*********bianjiao or send pwm********\n");
        fflush(fpsv_pos);
        if(box_lim(pic1, pic2, camsize_area[1]) == 1)
        {
            usleep(50000);
            snd_uart(me_fd, 0x32, 1500, 0, 0, 0x31, 1500);//发送俯仰和航向停止;
            usleep(50000);
            /****************自动变焦******************************/
            //需要变的焦距倍数;
            int resize_comput=cam_size_comput(camsize_area[0], me_trackbox, camera_obj); 

            fprintf(fpsv_pos,"h=%d,w=%d,cam[0]=%d,resize=%d\n",me_trackbox.height,me_trackbox.width,camsize_area[0],resize_comput);
            fflush(fpsv_pos);
            usleep(1);
            snd_uart(me_fd, 0x03, 1800, resize_comput, 0x03, 0, 0);//变焦
            sleep(resize_comput);//等待变焦完成;
            camsize_area[0] += resize_comput;
            cam_box_count += resize_comput;
            
            if(camsize_area[0] >= 26)
            {
                if(camsize_area[0] >= 30)
                    camsize_area[0] = 30;
            }
            else
                return 2;//跳转重新读取神经网络提供的模板和坐标;
        }  
        else
	    {   //目标框中心点不在图像中心范围之内，发送pwm值控制相机；
            //printf("*********box is not center********\n");
            //fprintf(fpsv_pos,"*********box is not center********\n");
            //fflush(fpsv_pos);
            snd_uart(me_fd, 0x32, pwmx, 0, 0, 0x31, pwmy);//发送控制相机俯仰和航向的指令;
	    }                  
    }  
          
    return 0;
}


