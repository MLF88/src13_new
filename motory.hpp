#ifndef MOTORY_H__
#define MOTORY_H__


//height 目标高度，direction 相机运动方向，camsize 当前焦距。
//返回 pwm;
int camera_pwm_y(int direction, int camsize);

//height 目标高度，direction 相机运动方向，1 代表向下，2代表向上。
//camsize 当前焦距;
int pix_movey(int height, int direction, int camsize, int *movcam);

#endif
