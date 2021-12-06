/*只能被main.cpp包含*/
#ifndef DP_LEARNING__H__
#define DP_LEARNING__H__

#include <pthread.h> //pthread_testcancel();//设置取消点；

#define  SAVE_FRAME

//#define  SAVE_FRAME_IN //保存输入视频帧;
//#define  SAVE_FRAME_OUT //保存输出视频帧;
#define  SAVE_FRAME_NUM 1 //抽帧的频率

#define IMGWID_LIMIT_TOWE 0.6  //设定杆塔目标框的边界；0.7,0.6,0.5
#define IMGHEI_LIMIT_TOWE 0.6

#define IMGWID_LIMIT_INSU 0.5  //设定绝缘子目标框的边界；0.7,0.6,0.5
#define IMGHEI_LIMIT_INSU 0.5  //  

#define IMGWID_LIMIT_HANG 0.4 //设定下挂点目标框的边界；0.7,0.6,0.5
#define IMGHEI_LIMIT_HANG 0.4

#define IMGWID_LIMIT_GROUND 0.4 ////groundpoint设定地线挂点目标框的边界；0.7,0.6,0.5
#define IMGHEI_LIMIT_GROUND 0.4

#define IMGWID_LIMIT_UPPOINT 0.2 //设定绝缘子上挂点目标框的边界；0.7,0.6,0.5
#define IMGHEI_LIMIT_UPPOINT 0.2

#define IMGWID_LIMIT_SPEAK 0.3 //设定塔尖目标框的边界；0.7,0.6,0.5
#define IMGHEI_LIMIT_SPEAK 0.3

#define IMGWID 960  //用于计算的目标长宽；
#define IMGHEI 720

//#define TRACKBOX_AREA  146400  //目标框面积的边界；
#define TRACKBOX_AREA  196400  //目标框面积的边界；
#define CENTER_RANGE  25 //目标框中心点范围；
#define THRNUM 3         //线程数量;
#define UART2 "/dev/ttyTHS1" //串口;
#define UART0 "/dev/ttyTHS3" //串口文件;
#define UART1 "/dev/ttyTHS2" //串口文件;

#define MY_SCORE_TOWE 0.99     //tower 杆塔
#define MY_SCORE_HANG 0.4     //hangingpoint 绝缘子下挂点
//#define MY_SCORE_INSU 0.24
#define MY_SCORE_INSU 0.25     //insulator 绝缘子
#define MY_SCORE_GROUND 0.2   //groundpoint  地线挂点
#define MY_SCORE_UPPOINT 0.15  //uppoint 绝缘子上挂点
#define MY_SCORE_SPEAK 0.5   //towerpeak 塔尖
#define MANUCAM 4 //手动变焦倍数;

struct target
{
    char *name;    //目标框名字;
    int box_label_name;
    int centre=960;
    //在深度学习中输出结果中感兴趣区域的目标框;
    cv::Rect my_learnbox;  //全局变量;
    //my_gray,是深度学习检测输出的视频帧，在learning和track进程中共享视频帧;
    cv::Mat my_gray; // 全局变量;
    //目标框得分连续累计;
	float my_box_score;//每一帧距离中心点最近目标框得分连续累计;
    float score_limit; 
    float image_lim[2]; //画框的比例；
    int img_limit[2];   
    //互斥量；
    pthread_mutex_t mut_my_gray;   
};
void saveframe(cv::Mat img, int frame_index,char *savename, char *path);

int test(void) ;

#endif
