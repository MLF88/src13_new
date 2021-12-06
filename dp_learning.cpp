#include <ssd_detect_so.hpp>
#include <math.h>
#include <sys/stat.h>
#include "dp_learning.hpp"
#include "dp_argu.hpp"

using namespace caffe;  // NOLINT(build/namespaces)

extern cv::Mat myframe;
extern pthread_mutex_t mut_myframe, mut_my_targ;

extern int my_img_flags;
extern int sendapp_model_flags;
extern int pthr_flags;
extern struct target gx[7];

extern char save_pic[64];

struct per_img 
{
	char *name; //目标框名字;
    int min;   //一帧图像中离中心点最小距离值;
    //返回的分数是最近目标分数，未必是本框的分数;
    float scores;  //每一帧最小距离的框分数;
    cv::Point top, bottom; //一帧图片距中心点最小距离框坐标;
};

static cv::Scalar g_colors[10] = {
	CV_RGB(255, 255, 255)
	, CV_RGB(255, 128, 255)
	, CV_RGB(0, 0, 0)
	, CV_RGB(255, 0, 0)
	, CV_RGB(128, 128,128)//4 灰
	, CV_RGB(0, 0, 255)
	, CV_RGB(0, 255, 0)//6绿
	, CV_RGB(64, 0, 128)
	, CV_RGB(87, 44, 0)
	, CV_RGB(255, 255, 0) };

//获取矩形中心点；
int getCenterPoint2(cv::Point p1,cv::Point p2)
{
	int ret;
	cv::Point cpt;
	cpt.x = (480-(p1.x+cvRound((p2.x-p1.x)/2.0)));
	cpt.y = (360-(p1.y+cvRound((p2.y-p1.y)/2.0)));
	ret=sqrt((cpt.x*cpt.x)+(cpt.y*cpt.y));
	//printf("p1.x=%d,p1.y=%d,p2.x=%d,p2.y=%d,ret=%d\n",p1.x,p1.y,p2.x,p2.y,ret);
	return ret;
}

//图片的边界筛选目标框;
static int target_box_lim(cv::Point& top,cv::Point& bottom)
{	
	if((top.x>=0) && (top.y>=0) && (bottom.x >= 5) && (bottom.y >= 5))
	{
		if(bottom.x > 960)
			bottom.x=960;
		if(bottom.y > 720)
			bottom.y=720;
		if( ((bottom.x-top.x)>5) && ((bottom.y-top.y)>5) )
			return 1;
		else 
			return 2;
	}
	else
		return 2;
}

//保存图片；
void saveframe(cv::Mat img, int frame_index,char *savename, char *path)
{
	char save_frame[512];
	sprintf(save_frame,"%s/%s%d.jpg",path,savename,frame_index);
	//printf("%s\n",save_frame);
	if((frame_index%SAVE_FRAME_NUM)==0)
		cv::imwrite(save_frame,img);
}
//筛选图片中的目标框，并赋值给gx_mid;
void box_filt(const vector<float> d, cv::Mat& img, char *label_name, struct per_img *gx_mid, int i)
{
	char buffer[10];

    //float scores;//返回的分数是最近目标分数，未必是本框的分数;
    cv::Point top_left_pt,bottom_right_pt;
	int box_tocenter;
	
	//目标框坐标点数据;
	top_left_pt.x = (int)(d[3] * img.cols);
	top_left_pt.y = (int)(d[4] * img.rows);
	bottom_right_pt.x = (int)(d[5] * img.cols);
	bottom_right_pt.y = (int)(d[6] * img.rows);
	
    //在画幅中绘制目标框，分数，目标框名称;
    cv::rectangle(img, top_left_pt, bottom_right_pt, g_colors[i % 10], 2);
	snprintf(buffer, sizeof(buffer), "%3.3f",d[2]);
	cv::putText(img, buffer, bottom_right_pt - cv::Point(0, 0), 
					cv::FONT_HERSHEY_SIMPLEX, 1, g_colors[i % 10], 1, 2);
			  
	cv::putText(img, label_name, bottom_right_pt - cv::Point(0, 30), 
                    cv::FONT_HERSHEY_SIMPLEX, 1, g_colors[i % 10], 1, 2);	

	//根据坐标点计算出目标中心点距画幅中心点距离;
    box_tocenter=getCenterPoint2(top_left_pt,bottom_right_pt);
    //根据中心点筛选目标框;
	if(box_tocenter < gx_mid->min)
	{
	    //着多个目标框同时出现在画幅中时，寻找最小距离点;更新到top,bottom,scores中;
		//top,bottom,scores 才是有效的目标框数据，每一帧画幅一个有效目标信息可用;
		gx_mid->min=box_tocenter;
		gx_mid->top=top_left_pt;
		gx_mid->bottom=bottom_right_pt;
		gx_mid->scores = d[2];
		//printf("min=%d,top.x=%d,top.y=%d,bottom.x=%d,bottom.y=%d,scores=%f\n",min,top.x,top.y,bottom.x,bottom.y,scores);
    }
       
    //d[2]才是本框的分数;                            
}
//gx_mid 结构体数组初始化;
void box_init(struct per_img *gx_mid)
{
	for(int i=1; i<7; i++)
	{
		gx_mid[i].min = 960;
		gx_mid[i].scores = 0.0;
		gx_mid[i].top.x = 0;
		gx_mid[i].top.y = 0;
		gx_mid[i].bottom.x = 0;
		gx_mid[i].bottom.y = 0;
	}
	gx_mid[1].name = "tower";
	gx_mid[2].name = "hangingpoint";
	gx_mid[3].name = "insulator";
	gx_mid[4].name = "groundpoint";
	gx_mid[5].name = "uppoint";
	gx_mid[6].name = "towerpeak";

}
//gx_mid 结构体数组内容写进gx结构体数组;
void box_write(struct per_img *gx_mid, cv::Mat img, struct target *gx, int frame_index)
{ 
	for(int i=1;i<7;i++)
	{
		gx[i].my_box_score += gx_mid[i].scores;
		gx[i].centre = gx_mid[i].min;//目标框到中心点的距离;
        //printf("gx[%d].my_box_score=%f,gx_mid[%d].scores=%f\n",i,gx[i].my_box_score,i,gx_mid[i].scores);
		//筛选目标框;并写入全局变量中;
		if( target_box_lim(gx_mid[i].top, gx_mid[i].bottom)==1 )
		{
			//锁；
			pthread_mutex_lock(&(gx[i].mut_my_gray));
			//sendapp_model_flags++; //发送给app端模型数量的标记
			//根据设置条件判断是否写入信息;     
			if(gx[i].my_box_score >= gx[i].score_limit) //目标框得分值累计到设定的分数，向共享全局变量复制目标截图;
			{
				//std::cout<<"my_box_score="<<my_box_score<<std::endl;
				gx[i].my_learnbox.x=gx_mid[i].top.x;
				gx[i].my_learnbox.y=gx_mid[i].top.y;
				gx[i].my_learnbox.width=gx_mid[i].bottom.x-gx_mid[i].top.x;//width;
				gx[i].my_learnbox.height=gx_mid[i].bottom.y-gx_mid[i].top.y;//heigth;
				//gx[i].centre = gx_mid[i].min;//目标框到中心点的距离;
				//printf("gx_mid[%d]=%d",gx_mid[i].min);			
				img(gx[i].my_learnbox).copyTo(gx[i].my_gray);

			}
			pthread_mutex_unlock(&(gx[i].mut_my_gray));
			//解锁；
		}
		else
		{
			//锁；
			pthread_mutex_lock(&(gx[i].mut_my_gray));
			(gx[i].my_gray).release();
			pthread_mutex_unlock(&(gx[i].mut_my_gray));
			//解锁；
		}
	}
}

//gx_mid 结构体数组复位;
void box_clean(struct per_img *gx_mid)
{
	for(int i=1; i<7; i++)
	{
		gx_mid[i].min = 960;
		gx_mid[i].scores = 0.0;
		gx_mid[i].top.x = 0;
		gx_mid[i].top.y = 0;
		gx_mid[i].bottom.x = 0;
		gx_mid[i].bottom.y = 0;
	}
}

int test()
{
	sleep(10);
	cv::Mat img;
	int frame_index=0;
	struct per_img gx_mid[7];

	box_init(gx_mid);

	mkdir(save_pic, 0777);
	// Initialize the network.
	Detector detector(DP_MODEL, DP_WEIGHTS, DP_MEAN_FILE, DP_MEAN_VALUE);

	while(1)
  	{
		//每一帧的数据;	
		frame_index++;
    	pthread_mutex_lock(&mut_myframe);//枷锁；
		if(myframe.empty())
			printf("myframe isfaild \n");
    	myframe.copyTo(img);//v4l2 read image;
    	pthread_mutex_unlock(&mut_myframe);

#ifdef SAVE_FRAME_IN
		saveframe(img, frame_index, "sv_pic_in", save_pic);
#endif

		//设置取消点；
    	pthread_testcancel();
		if(pthr_flags==1)
			return 0;
		std::vector<vector<float> > detections = detector.Detect(img);
		//设置取消点；	
    	pthread_testcancel();
		if(pthr_flags==1)
			return 0;
	
		//目标数量标记更新数据;
		pthread_mutex_lock(&mut_my_targ);//加锁；
		sendapp_model_flags++;//发送给app端模型数量的标记
		pthread_mutex_unlock(&mut_my_targ);//解锁；
	
    	/* Print the detection results. */
    	for (int i = 0; i < detections.size(); ++i)//目标框数量；
    	{
      		const vector<float> dc = detections[i];
      		// Detection format: [image_id, label, score, xmin, ymin, xmax, ymax].      
	  		const float score = dc[2];
	  		int label = (int)(dc[1]);

	  		//person,insulator;tower,hangingpoint,chair;
	  		switch(label)  
	  		{
#if 0
				case 1:    //tower 杆塔
				{
					if (score >= MY_SCORE_TOWE) //分数阈值判断
					{   
						gx[label].name = "tower";
						box_filt(dc, img, "tower", &gx_mid[label], i);
					}
					break;
				}
#endif
				case 2:    //hangingpoint 绝缘子下挂点
				{
					if (score >= MY_SCORE_HANG) //分数阈值判断
					{   
						gx[label].name = "hangingpoint";
						box_filt(dc, img, "hangingpoint", &gx_mid[label], i);
					}	  
					break;
				}
				case 3:    //insulator 绝缘子
				{
					if (score >= MY_SCORE_INSU) //分数阈值判断
					{   
						gx[label].name = "insulator";
						box_filt(dc, img, "insulator", &gx_mid[label], i);
					}
					break;
				}
				case 4:    //groundpoint  地线挂点
				{
					if (score >= MY_SCORE_GROUND) //分数阈值判断
					{   
						gx[label].name = "groundpoint";
						box_filt(dc, img, "groundpoint", &gx_mid[label], i);
					}
					break;
				}
				case 5:    //uppoint 绝缘子上挂点
				{
					if (score >= MY_SCORE_UPPOINT) //分数阈值判断
					{   
						gx[label].name = "uppoint";
						box_filt(dc, img, "uppoint", &gx_mid[label], i);
					}
					break;
				}
				case 6:    //towerpeak 塔尖
				{
					if (score >= MY_SCORE_SPEAK) //分数阈值判断
					{   
						gx[label].name = "towerpeak";
						box_filt(dc, img, "towerpeak", &gx_mid[label], i);
					}
					break;
				}
	  		}
      
		}
		//筛选目标框;并写入全局变量中;六个目标框依次写入;
		//printf("********\n");
		box_write(gx_mid, img, gx, frame_index);
		box_clean(gx_mid);

//save pic;
#ifdef SAVE_FRAME_OUT
		saveframe(img, frame_index, "sv_pic_out",save_pic);
#endif

  	}

  return 0;
}
