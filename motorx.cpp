#include "motorx.hpp"

int camera_pwm_x(int direction, int camsize)
{
    //speed pwm,times 时间单位ms
    int speed,times;

    switch(camsize)
    {
        case 1:
        case 2:
        {
            if(direction==1)//目标向右移动;
                speed=1900;
            else if(direction==2)//目标向左移动;
                speed=1050;
            break;
        }
        case 3:
        {
            if(direction==1)//目标向右移动;
                speed=1880;
            else if(direction==2)//目标向左移动;
                speed=1050;
            break;
        }
        case 4:
        {
            if(direction==1)//目标向右移动;
                speed=1870;
            else if(direction==2)//目标向左移动;
                speed=1050;
            break;
        }
        case 5:
        {
            if(direction==1)//目标向右移动;
                speed=1860;
            else if(direction==2)//目标向左移动;
                speed=1050;
            break;
        }
        case 6:
        {
            if(direction==1)//目标向右移动;
                speed=1850;
            else if(direction==2)//目标向左移动;
                speed=1100;
            break;
        }
        case 7:
        {
            if(direction==1)//目标向右移动;
                speed=1840;
            else if(direction==2)//目标向左移动;
                speed=1100;
            break;
        }
        case 8:
        {
            if(direction==1)//目标向右移动;
                speed=1830;
            else if(direction==2)//目标向左移动;
                speed=1200;
            break;
        }
        case 9:
        {
            if(direction==1)//目标向右移动;
                speed=1810;
            else if(direction==2)//目标向左移动;
                speed=1200;
            break;
        }
        case 10:
        {
            if(direction==1)//目标向右移动;
                speed=1770;
            else if(direction==2)//目标向左移动;
                speed=1250;
            break;
        }
        case 11:
        {
            if(direction==1)//目标向右移动;
                speed=1720;
            else if(direction==2)//目标向左移动;
                speed=1280;
            break;
        }
        case 12:
        {
            if(direction==1)//目标向右移动;
                speed=1680;
            else if(direction==2)//目标向左移动;
                speed=1300;
            break;
        }
        case 13:
        {
            if(direction==1)//目标向右移动;
                speed=1670;
            else if(direction==2)//目标向左移动;
                speed=1300;
            break;
        }
        case 14:
        {
            if(direction==1)//目标向右移动;
                speed=1660;
            else if(direction==2)//目标向左移动;
                speed=1310;
            break;
        }
        case 15:
        {
            if(direction==1)//目标向右移动;
                speed=1650;
            else if(direction==2)//目标向左移动;
                speed=1320;
            break;
        }
        case 16:
        {
            if(direction==1)//目标向右移动;
                speed=1650;
            else if(direction==2)//目标向左移动;
                speed=1340;
            break;
        }
        case 17:
        {
            if(direction==1)//目标向右移动;
                speed=1640;
            else if(direction==2)//目标向左移动;
                speed=1340;
            break;
        }
        case 18:
        {
            if(direction==1)//目标向右移动;
                speed=1630;
            else if(direction==2)//目标向左移动;
                speed=1350;
            break;
        }
        case 19:
        {
            if(direction==1)//目标向右移动;
                speed=1620;
            else if(direction==2)//目标向左移动;
                speed=1360;
            break;
        }
        case 20:
        {
            if(direction==1)//目标向右移动;
                speed=1620;
            else if(direction==2)//目标向左移动;
                speed=1370;
            break;
        }
        case 21:
        {
            if(direction==1)//目标向右移动;
                speed=1610;
            else if(direction==2)//目标向左移动;
                speed=1380;
            break;
        }
        case 22:
        {
            if(direction==1)//目标向右移动;
                speed=1610;
            else if(direction==2)//目标向左移动;
                speed=1390;
            break;
        }
        case 23:
        {
            if(direction==1)//目标向右移动;
                speed=1600;
            else if(direction==2)//目标向左移动;
                speed=1400;
            break;
        }
        case 24:
        {
            if(direction==1)//目标向右移动;
                speed=1590;
            else if(direction==2)//目标向左移动;
                speed=1410;
            break;
        }
        case 25:
        {
            if(direction==1)//目标向右移动;
                speed=1570;
            else if(direction==2)//目标向左移动;
                speed=1420;
            break;
        }
        case 26:
        {
            if(direction==1)//目标向右移动;
                speed=1560;
            else if(direction==2)//目标向左移动;
                speed=1420;
            break;
        }
        case 27:
        {
            if(direction==1)//目标向右移动;
                speed=1560;
            else if(direction==2)//目标向左移动;
                speed=1440;
            break;
        }
        case 28:
        {
            if(direction==1)//目标向右移动;
                speed=1555;
            else if(direction==2)//目标向左移动;
                speed=1440;
            break;
        }
        case 29:
        {
            if(direction==1)//目标向右移动;
                speed=1550;
            else if(direction==2)//目标向左移动;
                speed=1445;
            break;
        }
        case 30:
        {
            if(direction==1)//目标向右移动;
                speed=1550;
            else if(direction==2)//目标向左移动;
                speed=1445;
            break;
        }
        
        
    }

    //*movcam=speed;
    //*(movcam+1)=times;
    
    return speed;
}


int pix_movex(int width, int direction, int camsize, int *movcam)
{
    //speed pwm,times 时间单位ms
    int speed,times;

    switch(camsize)
    {
        case 1:
        case 2:
        {
            if(direction==1)//目标向右移动;
            {
                speed=1950;
                times=((13900*width)/720);
            }
            else if(direction==2)//目标向左移动;
            {
                speed=1050;
                times=((14000*width)/720);
            }
            break;
        }
        case 3:
        {
            if(direction==1)//目标向右移动;
            {
                speed=1950;
                times=((13600*width)/720);
            }
            else if(direction==2)//目标向左移动;
            {
                speed=1050;
                times=((13600*width)/720);
            }
            break;
        }
        case 4:
        {
            if(direction==1)//目标向右移动;
            {
                speed=1950;
                times=((113000*width)/720);
            }
            else if(direction==2)//目标向左移动;
            {
                speed=1050;
                times=((113000*width)/720);
            }
            break;
        }
        case 5:
        {
            if(direction==1)//目标向右移动;
            {
                speed=1950;
                times=((10300*width)/720);
            }
            else if(direction==2)//目标向左移动;
            {
                speed=1050;
                times=((10100*width)/720);
            }
            break;
        }
        case 6:
        {
            if(direction==1)//目标向右移动;
            {
                speed=1950;
                times=((8300*width)/720);
            }
            else if(direction==2)//目标向左移动;
            {
                speed=1050;
                times=((8100*width)/720);
            }

            break;
        }
        case 7:
        {
            if(direction==1)//目标向右移动;
            {
                speed=1950;
                times=((6900*width)/720);
            }
            else if(direction==2)//目标向左移动;
            {
                speed=1050;
                times=((7000*width)/720);
            }
            break;
        }
        case 8:
        {
            if(direction==1)//目标向右移动;
            {
                speed=1900;
                times=((6400*width)/720);
            }
            else if(direction==2)//目标向左移动;
            {
                speed=1100;
                times=((6500*width)/720);
            }
            break;
        }
        case 9:
        {
            if(direction==1)//目标向右移动;
            {
                speed=1850;
                times=((6400*width)/720);
            }
            else if(direction==2)//目标向左移动;
            {
                speed=1150;
                times=((6400*width)/720);
            }
            break;
        }
        case 10:
        {
            if(direction==1)//目标向右移动;
            {
                speed=1800;
                times=((6300*width)/720);
            }
            else if(direction==2)//目标向左移动;
            {
                speed=1200;
                times=((6400*width)/720);
            }
            break;
        }
        case 11:
        {
            if(direction==1)//目标向右移动;
            {
                speed=1750;
                times=((6700*width)/720);
            }
            else if(direction==2)//目标向左移动;
            {
                speed=1250;
                times=((6800*width)/720);
            }
            break;
        }
        case 12:
        {
            if(direction==1)//目标向右移动;
            {
                speed=1700;
                times=((7600*width)/720);
            }
            else if(direction==2)//目标向左移动;
            {
                speed=1300;
                times=((7800*width)/720);
            }
            break;
        }
        case 13:
        {
            if(direction==1)//目标向右移动;
            {
                speed=1700;
                times=((6800*width)/720);
            }
            else if(direction==2)//目标向左移动;
            {
                speed=1300;
                times=((7000*width)/720);
            }
            break;
        }
        case 14:
        {
            if(direction==1)//目标向右移动;
            {
                speed=1700;
                times=((6100*width)/720);
            }
            else if(direction==2)//目标向左移动;
            {
                speed=1300;
                times=((6400*width)/720);
            }
            break;
        }
        case 15:
        {
            if(direction==1)//目标向右移动;
            {
                speed=1650;
                times=((8400*width)/720);
            }
            else if(direction==2)//目标向左移动;
            {
                speed=1350;
                times=((9000*width)/720);
            }
            break;
        }
        case 16:
        {
            if(direction==1)//目标向右移动;
            {
                speed=1650;
                times=((7300*width)/720);
            }
            else if(direction==2)//目标向左移动;
            {
                speed=1350;
                times=((8000*width)/720);
            }
            break;
        }
        case 17:
        {
            if(direction==1)//目标向右移动;
            {
                speed=1650;
                times=((7500*width)/720);
            }
            else if(direction==2)//目标向左移动;
            {
                speed=1350;
                times=((8000*width)/720);
            }
            break;
        }
        case 18:
        {
            if(direction==1)//目标向右移动;
            {
                speed=1650;
                times=((6500*width)/720);
            }
            else if(direction==2)//目标向左移动;
            {
                speed=1350;
                times=((7000*width)/720);
            }
            break;
        }
        case 19:
        {
            if(direction==1)//目标向右移动;
            {
                speed=1650;
                times=((6000*width)/720);
            }
            else if(direction==2)//目标向左移动;
            {
                speed=1350;
                times=((6500*width)/720);
            }
            break;
        }
        case 20:
        {
            if(direction==1)//目标向右移动;
            {
                speed=1620;
                times=((8000*width)/720);
            }
            else if(direction==2)//目标向左移动;
            {
                speed=1380;
                times=((9500*width)/720);
            }
            break;
        }
        case 21:
        {
            if(direction==1)//目标向右移动;
            {
                speed=1620;
                times=((7000*width)/720);
            }
            else if(direction==2)//目标向左移动;
            {
                speed=1380;
                times=((8300*width)/720);
            }
            break;
        }
        case 22:
        {
            if(direction==1)//目标向右移动;
            {
                speed=1620;
                times=((5800*width)/720);
            }
            else if(direction==2)//目标向左移动;
            {
                speed=1380;
                times=((7200*width)/720);
            }
            break;
        }
        case 23:
        {
            if(direction==1)//目标向右移动;
            {
                speed=1600;
                times=((7200*width)/720);
            }
            else if(direction==2)//目标向左移动;
            {
                speed=1400;
                times=((9500*width)/720);
            }
            break;
        }
        case 24:
        {
            if(direction==1)//目标向右移动;
            {
                speed=1600;
                times=((6400*width)/720);
            }
            else if(direction==2)//目标向左移动;
            {
                speed=1400;
                times=((8500*width)/720);
            }
            break;
        }
        case 25:
        {
            if(direction==1)//目标向右移动;
            {
                speed=1600;
                times=((5500*width)/720);
            }
            else if(direction==2)//目标向左移动;
            {
                speed=1400;
                times=((7500*width)/720);
            }
            break;
        }
        case 26:
        {
            if(direction==1)//目标向右移动;
            {
                speed=1600;
                times=((5300*width)/720);
            }
            else if(direction==2)//目标向左移动;
            {
                speed=1400;
                times=((7200*width)/720);
            }
            break;
        }
        case 27:
        {
            if(direction==1)//目标向右移动;
            {
                speed=1580;
                times=((11000*width)/720);
            }
            else if(direction==2)//目标向左移动;
            {
                speed=1400;
                times=((6000*width)/720);
            }
            break;
        }
        case 28:
        {
            if(direction==1)//目标向右移动;
            {
                speed=1580;
                times=((8000*width)/720);
            }
            else if(direction==2)//目标向左移动;
            {
                speed=1400;
                times=((8000*width)/720);
            }
            break;
        }
        case 29:
        {
            if(direction==1)//目标向右移动;
            {
                speed=1580;
                times=((7400*width)/720);
            }
            else if(direction==2)//目标向左移动;
            {
                speed=1400;
                times=((7400*width)/720);
            }
            break;
        }
        case 30:
        {
            if(direction==1)//目标向右移动;
            {
                speed=1580;
                times=((6400*width)/720);
            }
            else if(direction==2)//目标向左移动;
            {
                speed=1400;
                times=((6400*width)/720);
            }
            break;
        }
        
        
    }

    *movcam=speed;
    *(movcam+1)=times;
    
    return 0;
}



