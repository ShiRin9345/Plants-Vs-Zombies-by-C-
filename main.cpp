/* 2023.11.16
* 1.创建主场景并导入素材
* 2.物品栏
* 3.植物卡牌
*/

/* 2023.11.17
* 1.实现植物卡牌的拖动
* 2.实现植物种植
*/

/* 2023.11.19
* 1.创建初始菜单
* 2.创建、显示、收集阳光
*/

/*2023.11.20
* 1.阳光的收集动画
* 2.创建僵尸
*/

#include<stdio.h>
#include<graphics.h>           //安装esayx图形库
#include<time.h>
#include"vector2.h"
#include"tools.h"        
#include<mmsystem.h>
#pragma comment(lib,"winmm.lib")        //???

#define WIN_WIDTH 900
#define WIN_HEIGHT 600        //定义宏方便修改数值
#define ZOMBINE_MAX 10

enum{BEAN,XIANG_RI_KUI,DA_ZUI_HUA,CHERRY,PLANT_COUNT};
IMAGE imageBg;              //背景图片
IMAGE imageCard;
IMAGE imageShovel;
IMAGE imageSlot;
IMAGE imageCards[PLANT_COUNT];
IMAGE * chosedPlant[PLANT_COUNT][20];

bool useShovel=false;

struct Plant
{
	int type;                 //植物类型 0为无，1为第一种
	int frameIndex;           //序列帧的序号 
	bool eaten;
	int shootTime;
	int deadTime;            //死亡计时器
	int sunshin;
	int timer;
	int x, y;
};

struct Plant Map[3][9];      //结构体数组
enum{SUNSHINE_DOWN,SUNSHINE_GROUND,SUNSHINE_COLLECT,SUNSHINE_PRODUCT};

struct sunshineBall
{
	int x, y;                //飘落的初始坐标   
	int frameIndex;          //图片索引 
	int destY;               //目标Y坐标
	bool ifused;              //是否使用
	int timer;
	bool click;               //是否点击 
	vector2 p1, p2, p3, p4;
	vector2 pCur;
	float t;
	float speed;
	int status;
};
struct sunshineBall Balls[10];
IMAGE imgSunshineBalls[29];
int sunshine;

int nowX, nowY;
int curPlant;                //全局与静态变量默认是0，指针和自动变量为随机值

enum{GOING,WIN,FAIL};
int killCount;
int zombineCount;
int gameStatus;

struct zombine
{
	int x, y;
	int row;
	int frameIndex;
	bool ifused;
	int speed;
	int HP;
	bool dead;
	bool eating;
	bool ash;
};
struct zombine zombine[10];
IMAGE imgZombine[22];
IMAGE imgZombineDead[10];
IMAGE imgZombineEating[21];
IMAGE imgZombineStand[11];
IMAGE imgZombineAsh[20];

struct bullet
{
	int x, y;
	int row;
	bool used;
	int speed;
	bool blast;               //是否发生爆炸       
	int frameIndex;

};
struct bullet bullets[50];
IMAGE imgBulletNormal;
IMAGE imgBulletBlast[4];

bool fileExist(const char* name)
{
	FILE * fp = fopen(name, "r");
	if (fp)
	{
		fclose(fp);
	}
	return fp != NULL;
}

void initialize(void)         
{
	loadimage(&imageBg, "res/bg.jpg");   //加载背景图片 
	loadimage(&imageCard, "res/bar5.png");
	loadimage(&imageShovel, "res/Screen/shovel.png");
	loadimage(&imageSlot, "res/Screen/shovelSlot.png");

	memset(chosedPlant, 0, sizeof(chosedPlant));
	memset(Map, 0, sizeof(Map));

	killCount = 0;
	zombineCount = 0;
	gameStatus = GOING;

	char name[60];                       //加载植物卡牌
	for (int i = 0; i < PLANT_COUNT; i++)
	{
		sprintf_s(name, sizeof(name), "res/Cards/card_%d.png", i+1);
		loadimage(&imageCards[i], name);

		for (int j = 0; j < 20; j++)
		{
			sprintf_s(name, sizeof(name), "res/zhiwu/%d/%d.png", i, j+1);
			if (fileExist(name))
			{
				chosedPlant[i][j] = new IMAGE;               //分配内存
				loadimage(chosedPlant[i][j], name);
			}
			else
			{
				break;
			}
		}

		curPlant = 0;
	}
	sunshine = 50;
	
	memset(Balls, 0, sizeof(Balls));
	for (int i=0; i < 29; i++)                                //加载阳光球
	{
		sprintf_s(name, sizeof(name), "res/sunshine/%d.png", i + 1);
		loadimage(&imgSunshineBalls[i], name);                     //就是这里出了错！！！
	}

	srand(time(NULL));

	initgraph(WIN_WIDTH, WIN_HEIGHT);                  //创建游戏窗口

	LOGFONT f;                                         //设置字体
	gettextstyle(&f);
	f.lfHeight = 30;
	f.lfWeight = 15;
	strcpy(f.lfFaceName, "Segoe UI Black");
	f.lfQuality = ANTIALIASED_QUALITY;               //抗锯齿
	settextstyle(&f);
	setbkmode(TRANSPARENT);
	setcolor(BLACK);

	memset(zombine, 0, sizeof(zombine));              //初始化僵尸
	for (int i = 0; i < 22; i++)
	{
		sprintf_s(name, sizeof(name), "res/zm/%d.png", i);                  //未分配内存
		loadimage(&imgZombine[i], name);
	}
	//加载僵尸尸体
	for (int i = 0; i < 10; i++)
	{
		sprintf_s(name, sizeof(name), "res/Zombies/ZombineDead/result-%d.png", i);
		loadimage(&imgZombineDead[i],name);
	}

	for (int i = 0; i < 20; i++)
	{
		sprintf_s(name, sizeof(name), "res//zm_dead/%d.png", i+1);           //路径错误
		loadimage(&imgZombineAsh[i], name);
	}

	loadimage(&imgBulletNormal, "res/bullets/bullet_normal.png");
	memset(bullets, 0, sizeof(bullets));

	//加载僵尸吃植物
	for (int i = 0; i < 21; i++)
	{
		sprintf_s(name, sizeof(name), "res/zm_eat/%d.png", i+1);
		loadimage(&imgZombineEating[i], name);
	}

	//豌豆子弹
	loadimage(&imgBulletBlast[3], "res/bullets/bullet_blast.png");
	for (int i = 0; i < 3; i++)
	{
		float k = (i + 1) * 0.2;
		loadimage(&imgBulletBlast[i], "res/bullets/bullet_blast.png", imgBulletBlast[3].getwidth() * k, imgBulletBlast[3].getheight() * k, true);
	}

	//僵尸站立
	for (int i = 0; i < 11; i++)
	{
		sprintf_s(name, sizeof(name), "res/zm_stand/%d.png", i + 1);
		loadimage(&imgZombineStand[i],name);
	}
}


void drawZombine()
{
	int ZombineCount = sizeof(zombine) / sizeof(zombine[0]);
	for (int i = 0; i < ZombineCount; i++)
	{
		if (zombine[i].ifused)
		{
			/*IMAGE* img = (zombine[i].dead) ? imgZombineDead : imgZombine;                //妙
			img += zombine[i].frameIndex;*/
			IMAGE* img = NULL;                             //定义空指针
			if (zombine[i].dead)
			{
				img = imgZombineDead;

			}
			else if (zombine[i].ash)
			{
				img = imgZombineAsh;
			}
			else if (zombine[i].eating)
			{
				img = imgZombineEating;
			}
			else
			{
				img = imgZombine;
			}
			img += zombine[i].frameIndex;
			putimagePNG(zombine[i].x, zombine[i].y-img->getheight(), img);
		}
	}

	
}

void drawSunshine()
{
	int BallMax = sizeof(Balls) / sizeof(Balls[0]);           //sizeof()便于改变大小
	for (int i = 0; i < BallMax; i++)
	{
		//if (Balls[i].ifused || Balls[i].click)
		if(Balls[i].ifused)
		{
			IMAGE* img = &imgSunshineBalls[Balls[i].frameIndex];        //精彩指针
			//putimagePNG(Balls[i].x, Balls[i].y, img);
			putimagePNG(Balls[i].pCur.x, Balls[i].pCur.y, img);
		}
	}
}

void drawCards()
{
	putimage(-112, 0, &imageBg);                    //渲染              
	putimagePNG(250, 0, &imageCard);             //生成无背景图片（去黑边）

	for (int i = 0; i < PLANT_COUNT; i++)
	{
		int x = 338 + i * 65;
		int y = 6;

		putimagePNG(x, y, &imageCards[i]);
	}
}

void drawPlant()
{
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 9; j++)
		{
			if (Map[i][j].type > 0)
			{
				//int x = 256 + j * 81;
				//int y = 179 + i * 106;
				int PlantType = Map[i][j].type - 1;
				int index = Map[i][j].frameIndex;
				//putimagePNG(x, y, chosedPlant[PlantType][index]);
				putimagePNG(Map[i][j].x, Map[i][j].y, chosedPlant[PlantType][index]);
			}
		}
	}
	//调换语块位置可以使拖拽的植物图片显示在放置植物图片上      
	if (curPlant)              //静态变量的作用域为函数块                       
	{
		putimagePNG(nowX - 30, nowY - 40, chosedPlant[curPlant - 1][0]);
	}
}

void drawBullets()
{
	int bulletMax = sizeof(bullets) / sizeof(bullets[0]);
	for (int i = 0; i < bulletMax; i++)
	{

		if (bullets[i].used)
		{
			if (bullets[i].blast)
			{
				IMAGE* img = &imgBulletBlast[bullets[i].frameIndex];
				putimagePNG(bullets[i].x, bullets[i].y, img);
			}
			else
			{
				putimagePNG(bullets[i].x, bullets[i].y, &imgBulletNormal);
			}

		}
	}
}

void drawShovel()
{
	putimagePNG(250-68, 0, &imageSlot);
	putimagePNG(250 - 68, 0, &imageShovel);
}

void drawMoveShovel()
{
	if (useShovel)
	{
		putimagePNG(nowX - 30, nowY - 40, &imageShovel);
	}
}

void SetTextStyle()
{
	//字体只可以输出字符串
	char scoreText[8];
	sprintf_s(scoreText, sizeof(scoreText), "%d", sunshine);
	outtextxy(276, 67, scoreText);                        //输出阳光值
}

void updateWindow(void)
{
	BeginBatchDraw();                            //双缓冲开始（）防止屏幕闪烁

	drawCards();
	drawShovel();
	SetTextStyle();
	drawPlant();
    drawZombine();
	drawSunshine();
	drawBullets();
	drawMoveShovel();
	
	EndBatchDraw();                              //双缓冲关闭
}


void collectSunshine(ExMessage* msg)            //这是一个结构体指针，指向变量要用箭头
{
	int count = sizeof(Balls) / sizeof(Balls[0]);
	int W = imgSunshineBalls[0].getwidth();
	int H = imgSunshineBalls[0].getheight();
	for (int i = 0; i < count; i++)
	{
		if (Balls[i].ifused)
		{
			int x = Balls[i].pCur.x;
			int y = Balls[i].pCur.y;
			if (msg->x > x && msg->x<x + W && msg->y>y && msg->y < y + H)
			{
				//Balls[i].ifused = false;
				Balls[i].status = SUNSHINE_COLLECT;

				mciSendString("stop res/audio/points.mp3", 0, 0, 0);
				mciSendString("play res/audio/points.mp3", 0, 0, 0);
				//PlaySound("play res/sunshine.wav", NULL, SND_FILENAME | SND_ASYNC);
				//mciSendString("play res/sunshine.mp3", 0, 0, 0);
				Balls[i].p1 = Balls[i].pCur;
				Balls[i].p4 = vector2(262, 0);
				Balls[i].t = 0;
				float distance = dis(Balls[i].p1 - Balls[i].p4);
				float off = 8;
				Balls[i].speed = 1.0 / (distance / off);
				break;
			}
		}
	}
}

void clickShovel(ExMessage msg)
{
	if (msg.message == WM_LBUTTONDOWN)
	{
		if (msg.x > 192 && msg.x < 240  && msg.y < 96)
		{
			mciSendString("stop res/audio/shovel.mp3", 0, 0, 0);
			mciSendString("play res/audio/shovel.mp3", 0, 0, 0);
			useShovel = true;                      
		}
	
	}
	else if (msg.message == WM_MOUSEMOVE && useShovel)
	{
		nowX = msg.x;
		nowY = msg.y;
	}
	else if (msg.message == WM_LBUTTONUP && useShovel)
	{
		useShovel = false;
		if (msg.x > 256 - 112 && msg.y > 179 && msg.y < 489)
		{
			mciSendString("stop res/audio/plant1.mp3", 0, 0, 0);
			mciSendString("play res/audio/plant1.mp3", 0, 0, 0);
			int row = (msg.y - 179) / 102;
			int col = (msg.x - 256 + 112) / 81;
			if (Map[row][col].type)
			{
				Map[row][col].type = 0;
				mciSendString("play res/audio/plant2.mp3", 0, 0, 0);


			}
		}

	}
}

void clickPlant(ExMessage msg)
{
	static int status = 0;
	if (msg.message == WM_LBUTTONDOWN)
	{
		if (msg.x > 338 && msg.x < 338 + 65 * PLANT_COUNT && msg.y < 96)
		{
			mciSendString("stop res/audio/seedlift.mp3", 0, 0, 0);
			mciSendString("play res/audio/seedlift.mp3", 0, 0, 0);
			int index = (msg.x - 338) / 65;
			status = 1;                       //选中
			curPlant = index + 1;               //所选植物的编号
		}
		else
		{
			collectSunshine(&msg);           //传递指针更快捷且占用内存小 
		}
	}
	else if (msg.message == WM_MOUSEMOVE && status == 1)
	{
		nowX = msg.x;
		nowY = msg.y;
	}
	else if (msg.message == WM_LBUTTONUP && status == 1)
	{
		if (msg.x > 256 - 112 && msg.y > 179 && msg.y < 489)
		{
			mciSendString("stop res/audio/plant1.mp3", 0, 0, 0);
			mciSendString("play res/audio/plant1.mp3", 0, 0, 0);
			int row = (msg.y - 179) / 102;
			int col = (msg.x - 256 + 112) / 81;
			status = 0;
			if (!Map[row][col].type)
			{
				Map[row][col].type = curPlant;
				Map[row][col].frameIndex = 0;
				Map[row][col].shootTime = 0;
				Map[row][col].x = 256 - 112 + col * 81;
				Map[row][col].y = 179 + row * 102 + 14;


			}
		}

		curPlant = 0;
	}
}

void UserClick()
{
	ExMessage msg;
	if (peekmessage(&msg))
	{
		clickPlant(msg);
		clickShovel(msg);
	}
}

void createSunshine()
{
	static int count = 0;               //控制生成的间隔帧
	static int fre = 300;               //收到主函数的帧数限制的影响
	count++;
	if (count >= fre)                   //md加了个;全错了
	{
		fre = 200 + rand() % 100;       //更改两次生成阳光的时间间隔 
		count = 0;

		int BallMax = sizeof(Balls) / sizeof(Balls[0]);

		int i;
		for (i = 0; i < BallMax && Balls[i].ifused; i++);
		if (i >= BallMax)
		{
			return;
		}
		Balls[i].ifused = true;
		Balls[i].status = SUNSHINE_DOWN;
		Balls[i].frameIndex = 0;
		Balls[i].timer = 0;
		Balls[i].t = 0;
		Balls[i].p1 = vector2(260-112 + rand() % (800 - 260+112),60); 
		Balls[i].p4 = vector2(Balls[i].p1.x,200+(rand()%4)*90);
		int off=2;
		float distance = Balls[i].p4.y - Balls[i].p1.y;
		Balls[i].speed = 1.0 / (distance / off);
	}

	//向日葵生产阳光
	int BallMax = sizeof(Balls) / sizeof(Balls[0]);
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 9; j++)
		{
			if (Map[i][j].type == XIANG_RI_KUI + 1)
			{
				Map[i][j].timer++;
				if (Map[i][j].timer > 200)
				{
					Map[i][j].timer = 0;

					int k=0;
					for (; k < BallMax && Balls[k].ifused; k++);
					if (k >= BallMax)
					{
						return;
					}
					Balls[k].ifused = true;
					Balls[k].p1 = vector2(Map[i][j].x, Map[i][j].y);
					int width=(100+rand()%50)*(rand()%2?1:-1);
					Balls[k].p4 = vector2(Map[i][j].x+width, Map[i][j].y+chosedPlant[XIANG_RI_KUI][0]->getheight()-imgSunshineBalls[0].getheight());
					Balls[k].p2 = vector2(Balls[k].p1.x + width * 0.3,Balls[k].p1.y-100);
					Balls[k].p3 = vector2(Balls[k].p1.x + width * 0.7, Balls[k].p1.y - 100);
					Balls[k].status = SUNSHINE_PRODUCT;
					Balls[k].speed = 0.05;
					Balls[k].t = 0;


				}
			}
		}
	}

}

void updateSunshine()
{
	int Ballmax = sizeof(Balls) / sizeof(Balls[0]);
	for (int i = 0; i < Ballmax; i++)
	{
		if (Balls[i].ifused)
		{
			Balls[i].frameIndex = (Balls[i].frameIndex + 1) % 29;             //%防止越界
			if (Balls[i].status == SUNSHINE_DOWN)
			{
				struct sunshineBall* sun = &Balls[i];
				sun->t += sun->speed;
				sun->pCur = sun->p1 + sun->t * (sun->p4 - sun->p1);
				if (sun->t >= 1)
				{
					sun->status = SUNSHINE_GROUND;
					sun->timer = 0;
				}

			}
			else if (Balls[i].status == SUNSHINE_GROUND)
			{
				Balls[i].timer++;
				if (Balls[i].timer > 100)
				{
					Balls[i].ifused = false;
					Balls[i].timer = 0;
				}
			}
			else if (Balls[i].status==SUNSHINE_COLLECT)
			{
				struct sunshineBall* sun = &Balls[i];
				sun->t += 4*sun->speed;
				sun->pCur = sun->p1 + sun->t * (sun->p4 - sun->p1);
				if (sun->t >= 1)
				{
					sun->ifused = false;
					sunshine += 25;
				}
			}
			else if (Balls[i].status == SUNSHINE_PRODUCT)
			{
				struct sunshineBall* sun = &Balls[i];
				sun->t += sun->speed;
				sun->pCur = calcBezierPoint(sun->t, sun->p1, sun->p2, sun->p3, sun->p4);
				if (sun->t > 1)
				{
					sun->status = SUNSHINE_GROUND;
					sun->timer = 0;
				}
			}
		}
		
	}
}

void createZombine()
{
	if (zombineCount >= ZOMBINE_MAX)
	{
		return;
	}

	static int count = 0;
	static int zombineFre = 500;
	static  bool flag=true;
	count++;
	if (count > zombineFre)
	{
		count = 0;
		zombineFre = 200 + rand() % 200;

		int i;
		int ZombineMax = sizeof(zombine) / sizeof(zombine[0]);
		for (i = 0; i < ZombineMax && zombine[i].ifused; i++);
		if (i < ZombineMax)
		{
			zombine[i].ifused = true;
			zombine[i].row = rand() % 3;
			zombine[i].x = WIN_WIDTH;
			zombine[i].y = 172 + 100 * (zombine[i].row+1);
			zombine[i].speed = 1;
			zombine[i].HP = 100;
			zombine[i].dead = false;               //未进行初始化
			zombine[i].eating = false;
			zombine[i].ash = false;
			zombineCount++;
		}
		if (flag)
		{
			mciSendString("play res/audio/siren.mp3", 0, 0, 0);
			flag = false;
		}
	}
	
}

void updateZombine()
{
	int zombineMax = sizeof(zombine) / sizeof(zombine[0]);

	static int count = 0;                        //用于控制僵尸移速
	count++;
	if (count > 1)
	{
		count = 0;

		for (int i = 0; i < zombineMax; i++)
		{
			if (zombine[i].ifused)
			{
				zombine[i].x -= zombine[i].speed;
				if (zombine[i].x < 56)
				{
					gameStatus = FAIL;
				}
			}
		}
		for (int i = 0; i < zombineMax; i++)
		{
			if (zombine[i].ifused)
			{
				if (zombine[i].dead)
				{
					zombine[i].frameIndex++;
					if (zombine[i].frameIndex >= 9)
					{
						zombine[i].ifused = false;
						killCount++;
						if (killCount == ZOMBINE_MAX)
						{
							gameStatus = WIN;
						}
					}
				}
				else if (zombine[i].ash)
				{
					zombine[i].frameIndex++;
					if (zombine[i].frameIndex >= 20)
					{
						zombine[i].ifused = false;
						killCount++;
						if (killCount == ZOMBINE_MAX)
						{
							gameStatus = WIN;
						}
					}
				}
				else if (zombine[i].eating)
				{
					zombine[i].frameIndex = (zombine[i].frameIndex + 1) % 21;
				}
				else
				{
					zombine[i].frameIndex = (zombine[i].frameIndex + 1) % 22;
				}
			}
		}
		
	}
	
}

void shoot()
{
	int lines [3] = { 0 };
	int zombineCount = sizeof(zombine) / sizeof(zombine[0]);
	int bulletMax = sizeof(bullets) / sizeof(bullets[0]);
	int dangerX=WIN_WIDTH-imgZombine[0].getwidth()/2;
	for (int i = 0; i < zombineCount; i++)
	{
		if (zombine[i].ifused && zombine[i].x < dangerX)
		{
			lines[zombine[i].row] = 1;
		}
	}

	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 9; j++)
		{
			if (Map[i][j].type == BEAN + 1&&lines[i])                    //解除bug
			{
				Map[i][j].shootTime++;
				if (Map[i][j].shootTime > 20)
				{
					Map[i][j].shootTime = 0;

					int k;
					for (k = 0; k < bulletMax && bullets[k].used; k++);
					if (k < bulletMax)
					{
						bullets[k].used = true;
						bullets[k].row = i;
						bullets[k].speed = 19;
						bullets[k].blast = false;
						bullets[k].frameIndex = 0;

						int zwX = 256-112 + j * 81;
						int zwY = 179 + i * 106;
						bullets[k].x = zwX + chosedPlant[Map[i][j].type - 1][0]->getwidth()-10;         //指针操作要用->
						bullets[k].y = zwY + 5;
					}
				}
			}
		}
	}
}

void updateBullets()
{
	int countMax = sizeof(bullets) / sizeof(bullets[0]);
	for (int i = 0; i < countMax; i++)
	{
		if (bullets[i].used)
		{
			bullets[i].x += bullets[i].speed;
			if (bullets[i].x > WIN_WIDTH)
			{
				bullets[i].used = false;
			}

			if (bullets[i].blast)                     //实现子弹碰撞检测
			{
				bullets[i].frameIndex++;
				if (bullets[i].frameIndex >= 4)
				{
					bullets[i].used = false;
				}
			}
		}
	}
}

void collisionBulletZombine()
{
	int bulletCount = sizeof(bullets) / sizeof(bullets[0]);
	int zombineCount = sizeof(zombine) / sizeof(zombine[0]);
	for (int i = 0; i < bulletCount; i++)
	{
		if (!bullets[i].used || bullets[i].blast)
		{
			continue;
		}
		for (int k = 0; k < zombineCount; k++)
		{
			if (!zombine[k].ifused)                        //可能有bug
			{
				continue;
			}

			int x1 = zombine[k].x + 80;
			int x2 = zombine[k].x + imgZombine[0].getwidth();
			if (!zombine[k].dead && bullets[i].row == zombine[k].row && bullets[i].x > x1 && bullets[i].x < x2)
			{
				mciSendString("stop res/audio/splat1.mp3", 0, 0, 0);
				mciSendString("play res/audio/splat1.mp3", 0, 0, 0);
				zombine[k].HP -= 10;
				bullets[i].blast = true;
				bullets[i].speed = 0;

				if (zombine[k].HP <= 0)
				{
					zombine[k].dead = true;
					zombine[k].speed = 0;
					zombine[k].frameIndex = 0;
				}
				break;
			}

		}
	}
}

void BombRadius()
{
	int ZombineCount = sizeof(zombine) / sizeof(zombine[0]);
	for (int i = 0; i < ZombineCount; i++)
	{
		if (zombine[i].ifused)
		{
			for (int k = 0; k < 9; k++)
			{
				for (int j = 0; j < 3; j++)
				{
					if (Map[j][k].type == 0)
					{
						continue;
					}
					int PlantX = 256 - 112 + k * 81;
					int PlantY = 179 + j * 102 + 14;
					int x1 = PlantX - 80;
					int x2 = PlantX + 160;
					int y1 = PlantY - 102;
					int y2 = PlantY + 204;
					int x3 = zombine[i].x + 80;
					int y3 = zombine[i].y;
					if (x3 > x1 && x3 < x2 && y3 > y1 && y3 < y2 && Map[j][k].type == CHERRY + 1&&!zombine[i].ash)
					{
						if (Map[j][k].frameIndex >= 7)
						{
							zombine[i].ash = true;
							zombine[i].speed = 0;
							zombine[i].frameIndex = 0;
						}
					}
				}
			}
		}
		
	}
}

void collisionPlant()
{
	int ZombineCount = sizeof(zombine) / sizeof(zombine[0]);
	for (int i = 0; i < ZombineCount; i++)
	{
		int row = zombine[i].row;
		for (int k = 0; k < 9; k++)
		{
			if (Map[row][k].type == 0)
			{
				continue;
			}
			int Plant= 256-112 + k * 81;
			int x1 = Plant - 10;
			int x2 = Plant + 60;
			int x3 = zombine[i].x + 80;
			{
				if (x3 > x1 && x3 < x2)
				{
					if (Map[row][k].eaten)
					{
						zombine[i].eating = true;
						zombine[i].speed = 0;
						if (!zombine[i].dead)
						{
							Map[row][k].deadTime++;
						}
						{
							if (Map[row][k].deadTime > 70)               //有点小问题

							{
								Map[row][k].deadTime = 0;
								Map[row][k].type = 0;
								for (int z = 0; z < ZombineCount; z++)
								{
									if (zombine[z].eating&& zombine[z].row==row&&(zombine[z].x+80)>x1&& (zombine[z].x + 80) <x2)
									{
										zombine[z].eating = false;
										zombine[z].frameIndex = 0;
										zombine[z].speed = 1;
									}
								}
								


							}
						}
					}
					else
					{
						Map[row][k].eaten = true;
						Map[row][k].deadTime = 0;
						zombine[i].eating = true;
						zombine[i].speed = 0;
						zombine[i].frameIndex = 0;
					}
				}
			}
		}
	}
}

void ifCollision()
{
	collisionBulletZombine();
	collisionPlant();
}

bool checkBomb(Plant * Cherry)       //要改变原数据要使用指针
{
	if (Cherry->frameIndex == 7)
	{
		mciSendString("stop res/audio/cherrybomb.mp3", 0, 0, 0);
		mciSendString("play res/audio/cherrybomb.mp3", 0, 0, 0);
	}
	if (Cherry->type == CHERRY+1 && Cherry->frameIndex >18)
	{
		Cherry->type = 0;
	}
	return Cherry->type==0;
}

void updateGame()
{
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 9; j++)
		{
			if (Map[i][j].type > 0)
			{
				Map[i][j].frameIndex++;
				if (checkBomb(&Map[i][j]) )      //注意帧数
				{
					continue;
				}
				int PlantType = Map[i][j].type-1;
				int index = Map[i][j].frameIndex;
				if (chosedPlant[PlantType][index]==NULL)
				{
					Map[i][j].frameIndex = 0;
				}
			}
		}
	}

	createSunshine();             //创建阳光
	updateSunshine();             //更新阳光状态 
	BombRadius();
	createZombine();
	updateZombine();
	shoot();                      //发送豌豆子弹
	updateBullets();
	ifCollision();

}

void startUI()
{
	IMAGE imgBg, imgMenu1, imgMenu2;
	loadimage(&imgBg, "res/menu.png");
	loadimage(&imgMenu1, "res/menu1.png");
	loadimage(&imgMenu2, "res/menu2/png");
	bool flag = false;                             //判断是否点击

	mciSendString("play res/bg.mp3", 0, 0, 0);

	while (1)
	{
		BeginBatchDraw();

		putimage(0, 0, &imgBg);
		putimagePNG(475, 75, flag ? &imgMenu2 : &imgMenu1);       //通过flag进行分支选择

		ExMessage msg;
		if (peekmessage(&msg))
		{
			if (msg.message == WM_LBUTTONDOWN)
			{
				if (msg.x > 474 && msg.x < 774 && msg.y>75 && msg.y < 215)
				{
					flag = true;

				}

			}
			else if (msg.message == WM_LBUTTONUP&&flag)
			{
				mciSendString("stop res/bg.mp3", 0, 0, 0);
				EndBatchDraw();
				break;                                   //执行return或者break后跳出循环，结束函数
			}
		}

		EndBatchDraw();
		
	}
}

void viewScene()
{   
	//预览
	int RLimit = WIN_WIDTH - imageBg.getwidth();
	vector2 points[9] = { {550,80},{530,160}, {630,170}, {530,220}, {515,270}, {565,370}, {605,340}, {705,280}, {690,340} };
	
	int index[9];
	for (int i = 0; i < 9; i++)
	{
		index[i] = rand() % 11;
	}

	int count = 0;
	for (int x = 0; x >= RLimit; x -= 2)
	{
		BeginBatchDraw();

		putimage(x, 0, &imageBg);

		count++;
		for (int k = 0; k < 9; k++)
		{
			putimagePNG(points[k].x - RLimit + x, points[k].y, &imgZombineStand[index[k]]);
			if (count >= 10)
			{
				index[k] = (index[k] + 1) % 11;
			}
		}
		if (count >= 10)
		{
			count = 0;
		}

		EndBatchDraw();
		Sleep(5);                          //起到缓冲的作用
	}

	//停留
	for (int i = 0; i < 100; i++)
	{
		BeginBatchDraw();

		putimage(RLimit, 0, &imageBg);
		for (int k = 0; k < 9; k++)
		{
			putimagePNG(points[k].x, points[k].y, &imgZombineStand[index[k]]);
			index[k] = (index[k] + 1) % 11;
		}

		EndBatchDraw();
		Sleep(80);
	}

	//回放
	for (int x = RLimit; x<= -112; x+=2)
	{
		BeginBatchDraw();

		putimage(x, 0, &imageBg);
		count++;
		for (int k = 0; k < 9; k++)
		{
			putimagePNG(points[k].x-RLimit+x, points[k].y, &imgZombineStand[index[k]]);
			if (count >= 10)
			{
				index[k] = (index[k] + 1) % 11;
			}
			
		}
		if (count >= 10)
		{
			count = 0;
		}
		EndBatchDraw();
		Sleep(5);
	}

}

void cardsDown()
{
	int height = imageCard.getheight();
	for (int h = -height; h <= 0; h++)
	{
		BeginBatchDraw();
		putimagePNG(250, h, &imageCard);
		for (int i = 0; i < PLANT_COUNT; i++)
		{
			int x = 338 + i * 65;
			int y = 6 + h;
			putimage(x, y, &imageCards[i]);
		}

		EndBatchDraw();
		Sleep(10);
	}

}

bool GameStatus()
{
	bool ret = false;
	if (gameStatus == WIN)
	{
		Sleep(2000);
		loadimage(0, "res/gameWin.png");
		mciSendString("play res/win.mp3", 0, 0, 0);
		mciSendString("stop res/audio/UraniwaNi.mp3", 0, 0, 0);
		ret = true;
	}
	else if (gameStatus == FAIL)
	{
		Sleep(2000);
		loadimage(0, "res/gameFail.png");
		mciSendString("play res/lose.mp3", 0, 0, 0);
		mciSendString("stop res/audio/UraniwaNi.mp3", 0, 0, 0);
		mciSendString("play res/audio/scream.mp3", 0, 0, 0);
		ret = true;
	}
	return ret;
}

int main(void)
{
	initialize();
	startUI();
	mciSendString("play res/audio/UraniwaNi.mp3 repeat", 0, 0, 0);
	viewScene();
	cardsDown();

	int timer = 0;
	bool flag = true;                  //flag为标记
	while (1)
	{
		UserClick();
		timer += getDelay();
		if (timer > 50)
		{
			flag = true;
			timer = 0;
		}
		if (flag)
		{
			flag = false;
			updateWindow();
		    updateGame();
			if (GameStatus())
			{
				break;
			}

		}
		
		                                    //sleep(ms)函数可实现帧等待                                                           
	}                    
	system("pause");
	return 0;
}
