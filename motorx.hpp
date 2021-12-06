#ifndef MOTORX_H__
#define MOTORX_H__

//direction 相机运动方向，camsize 当前焦距;
//返回 pwm 
int camera_pwm_x(int direction, int camsize);

//height 目标宽度，direction 相机运动方向，1 代表向下，2代表向上。
//camsize 当前焦距;
int pix_movex(int width, int direction, int camsize, int *movcam);

#endif
