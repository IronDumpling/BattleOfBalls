//
//  BattleOfBalls_CSDN.c
//  
//
//  Created by 张楚岳 on 2021-04-06.
//

#include "BattleOfBalls_CSDN.h"
#include<graphics.h> //包含easyx图形库文件
#include<time.h>     //C语言时间头文件
#include<mmsystem.h>//win32多媒体设备接口文件
#pragma comment(lib,"winmm.lib")//win32多媒体设备接口库
#define WIN_WIDTH  1024  //屏幕的宽
#define WIN_HEIGHT 640//576   //屏幕高
#define MAP_WIDTH  (WIN_WIDTH*3) //地图宽度
#define MAP_HEIGHT (WIN_HEIGHT*3)//地图高度
#define FOOD_NUM 500  //食物数量
#define AI_NUM 200    //ai数量
IMAGE map(MAP_WIDTH, MAP_HEIGHT);
POINT  g_CameraPos;      // 摄像机(屏幕左上角)在地图上的位置

struct Ball
{
  bool    flag;    // 是否被吃 活 1,死 0
  COLORREF  color;    // 颜色
  float    r;      //
  float    x;      // 坐标
  float    y;
  int      type;       //食物独有属性，决定是什么类型的食物（圆？矩形？多边形？）
};

struct Ball mover; //玩家
struct Ball food[FOOD_NUM]; //食物数组
struct Ball ai[AI_NUM]; //AI数量
void ChaseAlgorithom(Ball *chase, Ball *run);
float DisTance(Ball b1, Ball b2);


// 初始化游戏
void Gameinit()
{
  //设置随机数种子
  srand((unsigned)time(NULL));
  
  //播放背景音乐
  mciSendString("open BallGame.mp3 alias bk", 0, 0, 0);
  mciSendString("play bk repeat", 0, 0, 0);
  
  //初始化食物
  for (int i = 0; i < FOOD_NUM; i++){
    food[i].x = (float)(rand() % MAP_WIDTH);
    food[i].y = (float)(rand() % MAP_HEIGHT);
    food[i].r = (float)(rand() % 5 + 1);
    food[i].color = RGB(rand() % 256, rand() % 256, rand() % 256);  // 随机颜色
    food[i].flag = 1;
    food[i].type = rand() % 3;
  }
  
  //初始化AI
  for (int i = 0; i < AI_NUM; i++){
    ai[i].color = RGB(rand() % 256, rand() % 256, rand() % 256);   //rand()%256  随机取值 0-255
    ai[i].flag = 1;
    ai[i].x = rand() % (MAP_WIDTH - int(ai[i].r + 0.5)) + int(ai[i].r + 0.5);   //AI产生的位置不会出现一个越界
    ai[i].y = rand() % (MAP_HEIGHT - int(ai[i].r + 0.5)) + int(ai[i].r + 0.5);
    ai[i].r = float(rand() % 10 + 10);
  }
  
  {//初始化玩家数据
    mover.color = BLUE;
    mover.r = 10;
    
    //玩家在窗口的位置
    mover.x = rand() % WIN_WIDTH;
    mover.y = rand() % WIN_HEIGHT;
    mover.flag = 1;
  }
}


//游戏地图开始绘制位置判断
void computeCameraPos()
{
  // 以人物位置为中心计算摄像机的理论位置
  g_CameraPos.x = mover.x - WIN_WIDTH / 2;
  g_CameraPos.y = mover.y - WIN_HEIGHT / 2;

  // 防止摄像机越界
  if (g_CameraPos.x < 0)    g_CameraPos.x = 0;
  if (g_CameraPos.y < 0)    g_CameraPos.y = 0;
  if (g_CameraPos.x > MAP_WIDTH - WIN_WIDTH)  g_CameraPos.x = MAP_WIDTH - WIN_WIDTH;
  if (g_CameraPos.y > MAP_HEIGHT - WIN_HEIGHT)  g_CameraPos.y = MAP_HEIGHT - WIN_HEIGHT;
}


//游戏绘制
void Gamedraw()
{
  SetWorkingImage(&map);
  setbkcolor(WHITE);          // 白色背景
  cleardevice();
  
    //画食物
  for (int i = 0; i < FOOD_NUM; i++){
    if (food[i].flag == 1)
    {
      setfillcolor(food[i].color);
      if (food[i].type == 0)
      {
        solidellipse((int)food[i].x, (int)food[i].y, (int)(food[i].x + food[i].r), (int)(food[i].y + food[i].r));
      }
      else if (food[i].type == 1)
      {
        solidpie((int)food[i].x, (int)food[i].y, (int)(food[i].x + food[i].r), (int)(food[i].y + food[i].r), 0.0, 1.5);
      }
      else
      {
        solidcircle(int(food[i].x), int(food[i].y), int(food[i].r));
      }
    }
    else
    {
      food[i].x = (float)(rand() % MAP_WIDTH);
      food[i].y = (float)(rand() % MAP_HEIGHT);
      food[i].r = (float)(rand() % 5 + 1);
      food[i].color = RGB(rand() % 256, rand() % 256, rand() % 256);  // 随机颜色
      food[i].flag = 1;
      food[i].type = rand() % 3;
    }
  }
    
  //画AI
  for (int i = 0; i < AI_NUM; i++){
    if (ai[i].flag == 1)
    {
      setfillcolor(ai[i].color);
      solidcircle(ai[i].x, ai[i].y, ai[i].r);
    }
    else
    {
      ai[i].color = RGB(rand() % 256, rand() % 256, rand() % 256);   //rand()%256  随机取值 0-255
      ai[i].flag = 1;
      ai[i].x = rand() % (MAP_WIDTH - int(ai[i].r + 0.5)) + int(ai[i].r + 0.5);   //AI产生的位置不会出现一个越界
      ai[i].y = rand() % (MAP_HEIGHT - int(ai[i].r + 0.5)) + int(ai[i].r + 0.5);
      ai[i].r = float(rand() % 10 + 10);
    }
  }
    
  //绘制玩家
  setfillcolor(RED);
  fillcircle(mover.x, mover.y, mover.r);
  //绘制玩家名称
  settextcolor(BLACK);
  setbkmode(TRANSPARENT);
  settextstyle(20, 0, "楷体");
  char pname[] = "微信公众号：C语言Plus";
  int twidth = textwidth(pname)/2;//计算文字宽度为了居中显示在玩家中央
  outtextxy(mover.x-twidth, mover.y, pname);
  //恢复默认工作区，即窗口
  SetWorkingImage();
  //更新摄像机位置
  computeCameraPos();
  //把地图显示到窗口上
  putimage(0, 0, WIN_WIDTH, WIN_HEIGHT, &map, g_CameraPos.x, g_CameraPos.y);
}


//玩家移动，参数为玩家移动的速度（单位像素）
void Gamemove(int speed)
{
  //获取键盘按键
  if (GetAsyncKeyState(VK_UP) & 0x8000)
  {
    if (mover.y - mover.r > 0)
    {
      mover.y -= speed;
    }
  }
  if (GetAsyncKeyState(VK_DOWN) & 0x8000)
  {
    if (mover.y + mover.r < MAP_HEIGHT)
    {
      mover.y += speed;
    }
  }
  if (GetAsyncKeyState(VK_LEFT) & 0x8000)
  {
    if (mover.x - mover.r > 0)
    {
      mover.x -= speed;
    }
  }
  if (GetAsyncKeyState(VK_RIGHT) & 0x8000)
  {
    if (mover.x + mover.r < MAP_WIDTH)
    {
      mover.x += speed;
    }
  }
  //作弊测试，按空格不断增大,A缩小
  if (GetAsyncKeyState(VK_SPACE) & 0x8000)
  {
    mover.r += 3;
  }
  if (GetAsyncKeyState('A') & 0x8000)
  {
    if (mover.r > 1)
      mover.r -= 1;
  }
}


//玩家吃食物
void EatFood()
{
  for (int i = 0; i < FOOD_NUM; i++)
  {
    if (food[i].flag == 1 && DisTance(food[i], mover) < mover.r)
    {
      food[i].flag = 0;
      mover.r += food[i].r / 4;
    }
  }
}


//玩家吃Ai，Ai吃玩家
void EatAi()
{
  for (int i = 0; i < AI_NUM; i++)
  {
    if (ai[i].flag == 0)
      continue;
    //玩家吃Ai
    if (DisTance(ai[i], mover) < mover.r - ai[i].r / 3)
    {
      ai[i].flag = 0;
      mover.r += ai[i].r / 4;
      break;//吃到一个就退出
    }
    //ai吃玩家
    else if (DisTance(ai[i], mover) < ai[i].r - mover.r / 3)
    {
      mover.x = rand() % MAP_WIDTH;
      mover.y = rand() % MAP_HEIGHT;
      ai[i].r += mover.r / 4;
      mover.r = 10;
      break;
    }
  }
}


//AI吃食物,AI吃Ai
void AiEatFood()
{
  for (int i = 0; i < AI_NUM; i++)
  {
    if (ai[i].flag == 0)
      continue;
    //ai吃食物
    for (int j = 0; j < FOOD_NUM; j++)
    {
      if (food[j].flag == 1 && DisTance(ai[i], food[j]) < ai[i].r)
      {
        food[j].flag = 0;
        ai[i].r += food[i].r / 4;
      }
    }
    //Ai吃Ai
    for (int k = i + 1; k < AI_NUM; k++)
    {
      if (ai[k].flag == 1)
      {
        if (DisTance(ai[i], ai[k]) < ai[k].r - ai[i].r / 3)
        {
          ai[i].flag = 0;
          ai[k].r += ai[i].r / 4;
        }
        else if (DisTance(ai[i], ai[k]) < ai[i].r - ai[k].r / 3)
        {
          ai[k].flag = 0;
          ai[i].r += ai[k].r / 4;
        }
      }

    }
  }
}


//Ai移动算法，追逐比自己半径小的球
void AiMove()
{
  for (int i = 0; i < AI_NUM; i++)
  {
    double min_DISTANCE = MAP_WIDTH;//最大搜索距离
    int min = -1;//用来保存找到目标小球的下标
    if (ai[i].flag == 1)
    {
      //AI靠近AI
      for (int k = i + 1; k < AI_NUM; k++)
      {
        if (ai[i].r > ai[k].r&&ai[k].flag == 1)
        {//如果能吃，且距离达到要求则更新距离，并保存下标
          if (DisTance(ai[i], ai[k]) < min_DISTANCE)
          {
            min_DISTANCE = DisTance(ai[i], ai[k]);
            min = k;
          }
        }
      }
    }
    //如果找到目标，则去追逐
    if ((min != -1))
    {
      ChaseAlgorithom(&ai[i], &ai[min]);
    }

  }
}


void  JudeEat()
{
  EatFood();
  EatAi();
  AiEatFood();
  AiMove();
}


//追逐算法
void ChaseAlgorithom(Ball *chase, Ball *run)
{
  if (rand() % 2 == 0)
  {
    if (chase->x < run->x)
    {
      chase->x += 2;
    }
    else
    {
      chase->x -= 2;
    }
  }
  else
  {
    if (chase->y < run->y)
    {
      chase->y += 2;
    }
    else
    {
      chase->y -= 2;
    }
  }
}


//求两点之间的距离
float DisTance(Ball b1, Ball b2)
{
  return sqrt((b1.x - b2.x)*(b1.x - b2.x) + (b1.y - b2.y)*(b1.y - b2.y));
 }


int main()
{
  initgraph(WIN_WIDTH, WIN_HEIGHT, 1);
  Gameinit();
  DWORD t1, t2;
  t1 = t2 = GetTickCount();
  while (1)
  {
    Gamedraw();
    Gamemove(1);
    if (t2 - t1 > 50)
    {
      JudeEat();
      t1 = t2;
    }
    t2 = GetTickCount();
  }

  getchar();
  closegraph();
  return 0;
}
