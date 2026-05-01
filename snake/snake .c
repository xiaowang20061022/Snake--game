#include <stdio.h>
#include <Windows.h>
#include <conio.h>
#include <time.h>
#include <mmsystem.h>
#pragma comment(lib,"winmm.lib")

//表示颜色，默认是绿色，有复活甲是金色，中毒是紫色
#define GREEN 0x02
#define GOLDEN 0x06
#define PURPLE 13
#define RED 0x0c
#define DEFAULT 0X07

//中毒持续时间
#define POISON_DURATION 5000

//bgm名字
#define BGMGAME 1
#define BGMSETTING 2
#define BGMTEAM 3
#define BGMRANK 4
#define BGMOVER 5
#define BGMMAIN 6


// -------------------- 数据设计 --------------------

/*
	蛇：
		.x  表示一节身体的x坐标
		.y  表示一节身体的y坐标
*/
typedef struct snake {
	int x;
	int y;
} Snake;


/*
	完整的蛇身体，使用固定最大长度，最大长度为地图大小，有冗余
	数组下标代表的是第几节身体，例如：
		body[0] = {3, 5} 表示的是第0节身体（蛇头）的坐标是（3，5），以此类推
*/
Snake body[20 * 20] = {
	{3, 5},
	{3, 4},
	{3, 3},
	{3, 2},
	{3, 1}
};

/*
	苹果：
		.active  值为0或1，表示该苹果是否可用，0表示不在场上，1表示在场上
		.type  值为1/2/3/, 表示该苹果的种类,1为普通苹果，2是毒苹果，3是复活甲
		.x  表示该苹果的x坐标
		.y  表示该苹果的y坐标
*/
typedef struct apple {
	int active;
	int type;
	int x;
	int y;
} Apple;

/*
管理所有苹果的数组
场上可能同时存在好多苹果
*/
Apple apples[100] = { 0 };

/*
地图：
	map[i][j]表示坐标（i，j）的值
		0表示空地
		1表示墙
		2表示苹果
		3表示蛇头
		4表示蛇身
	如：map[3][6] = 1  表示（3，6）的位置是墙
		map[3][6] = 2  表示（3，6）的位置是苹果
	全部初始化为0，后面用函数生成元素
*/
int map[20][20];

//蛇的长度，默认为3
int targetLength;
//可见长度
int visibleLength;

//移动前的蛇身
int oldBodyX[400] = { 0 };
int oldBodyY[400] = { 0 };
int oldLength = 0;

//蛇前进的方向，默认向右
char dir;

//分数
int score;

/*
	难度等级:
		0：一般模式
		1：困难模式
*/
int difficulty;

/*
	是否有复活甲：
		0：无
		1：有
*/
int hasRevive;

//随机数N（5-10）
int randNum;

//吃的苹果数量
int appleCount;

//吃的毒苹果数量
int poisonCount;

//新吃的苹果数量
int newAppleCount;

//排行榜
typedef struct rank {
	char* name;
	int score;
} Rank;
Rank ranks[10] = { 0 };

//bgm开关,默认开
int bgmSwitch = 1;

//bgm音量，默认400
int bgmVolume = 400;

//音效开关
int soundEffectSwitch = 1;

//中毒指示
int isPoisoned;
//中毒计时
DWORD poison_time;

// -------------------- 数据设计 --------------------


// -------------------- service --------------------
/*
	负责人: 十一
	截止10号晚上9点

	功能: init: 初始化游戏数据
		1  初始化地图
		2  初始化蛇的长度（长度3）
		3  初始化蛇的位置（起始位置）
		4  初始化蛇的朝向（d，表示向右）
		5  初始化复活甲的数量（1）开局赠送的
		6  初始化苹果数组（只有一个普通苹果）
		7  初始化吃的苹果数量、毒苹果数量和新吃的苹果数量
	参数: void
	返回值: void
*/
void init(void);

/*
	负责人: 句号
	截止10号晚上9点
	功能: 生成地图

	参数: int difficulty
	返回值: void
*/
void generateMap(int difficulty);

/*
	负责人: 小王
	截止10号晚上9点
	功能: generateApple: 在空地生成苹果
		遍历苹果数组，找到第一个可用的下标，active==0的苹果
		使用随机数生成坐标，在对应坐标生成普通苹果
		如果新吃苹果newAppleCount等于随机数randNum，遍历苹果数组，找到第一个可用的下标，active==0的苹果，
			使用随机数生成坐标，在对应坐标生成特殊苹果，95%概率生成毒苹果,5%概率生成复活甲
			重新设置randNum，重置newAppleCount的值为0

	参数: void
	返回值: void
*/
void generateApple(void);

/*
	负责人: 啊Sii
	截止10号晚上9点
	功能: changeDirection: 改变蛇的方向
		不能直接反向(如向右时不能立即向左)
	参数:
		newDirection: 新方向 d:右 a:左 w:下 s:上
	返回值:
		0: 方向修改失败(反向)
		1: 方向修改成功
*/
int changeDirection(char newDirection);

/*
	负责人: 星辰
	截止10号晚上9点
	功能: calPosition: 根据当前蛇头的坐标，计算映射后的数组下标，根据对应值执行对应的操作，并决定游戏是否继续
		苹果：被吃的苹果的.active改为0，苹果计数和新吃苹果计数加1，长度加1，积分加2，调用generateApple()函数
		毒苹果：被吃的苹果的.active改为0，毒苹果计数poisonCount加1，长度减poisonCount，积分减2 * poisonCount，蛇身颜色改变
		复活甲：如果有：长度加5，积分加10
				如果没有：复活甲加1
		墙/蛇身：
				如果有复活甲，就重置蛇的位置和visibleLength
								body[0].y = 48;
								body[1].y = 46;
								body[2].y = 44;
								body[0].y = 20;
								body[1].y = 20;
								body[2].y = 20;
								visibleLength = 3;
				没有就返回0
	参数: void
	返回值:
		墙/蛇身：
			如果有复活甲，返回1
			如果没有复活甲，返回0
		空地：返回2
		苹果：返回3
		毒苹果：返回4
		复活甲：返回5
*/
int calPosition(void);

/*
	负责人 :KC
	截止10号晚上9点
	功能: moveSnake: 记录旧蛇的坐标，更新蛇的坐标，更新对应坐标在map中的值

	参数：void
	返回值:
		void
*/
void moveSnake(void);

/*
	负责人:liang
	截止10号晚上9点
	功能: saveScore: 保存分数到排行榜
		如果当前分数高于排行榜第10名,则插入并排序
	参数: inputName
	返回值: void
*/
void saveScore(char* inputName);

// -------------------- service --------------------

// -------------------- view --------------------
/*
	负责人: 星辰
	截止11号晚上9点
	功能: menuView: 展示主菜单
		while(1){
			1. 显示菜单选项(开始游戏/排行榜/游戏设置/团队介绍/退出)
			2. 底部有一条蛇在穿梭,吃到对应选项前的苹果进入对应界面
			疑问
			3. 根据选择执行:
				开始游戏: modeSelectView()
				排行榜: rankView()
				游戏设置: settingView()
				团队介绍: teamView()
				退出: exit(0)
		}
	参数: void
	返回值: void
*/
void menuView();

/*
	负责人:KC
	截止11号晚上9点
	功能: modeSelectView: 模式选择界面
		while(1){
			1. 显示模式选项(一般模式/困难模式/返回主菜单)
			2. 底部有一条蛇在穿梭,吃到对应选项前的苹果进入对应界面
			3. 根据选择执行:
				一般模式: difficulty=0, gameView()
				困难模式: difficulty=1, gameView()
				返回主菜单: break
		}
	参数: void
	返回值: void
*/
void modelSelectView();

/*
	负责人: 十一
	截止11号晚上9点
	功能: gameViewShowMap: 根据Map数组打印游戏地图
		使用不同颜色显示不同元素:
			墙壁: 默认色
			蛇：绿色
			普通苹果: 红色
			特殊苹果: 紫色(毒苹果/复活甲)
		使用gotoXY控制打印位置
	参数: void
	返回值: void
*/
void gameViewShowMap();

/*
	负责人:十一
	截止11号晚上9点
	功能: gameView_ShowInfo: 显示游戏信息
		显示当前分数、难度、复活甲状态、操作说明等
	参数: void
	返回值: void
*/
void gameViewShowInfo();

/*
	*难点2
	负责人: 小王
	截止11号晚上9点
	功能: gameView: 游戏主界面
		1. 调用init()初始化游戏
		2. while(1) {
			打印地图和信息
			处理键盘输入:
				WASD: 改变方向
				空格: 调用pauseView()
			调用moveSnake()函数
			调用calPosition()函数
			根据calPosition()函数的返回值，决定继续还是跳出循环
			程序休眠，根据难度决定休眠时间
		}
		3. 游戏结束,调用overView()
	参数: void
	返回值: void
*/
void gameView();

/*
	负责人:小王
	截止11号晚上9点
	功能: pauseView: 暂停界面
		显示选项: 继续游戏/重新开始/返回主菜单
		按WS移动光标,按回车选择
	参数: void
	返回值:
		0: 继续游戏
		1: 重新开始(返回模式选择)
		2: 返回主菜单
*/
int pauseView();

/*
	负责人:句号
	截止11号晚上9点
	功能: overView: 游戏结束界面
		显示"游戏结束"
		显示本局积分和蛇的长度
		提示按ESC返回主菜单
		如果分数进入排行榜,调用saveScore()
	参数: void
	返回值: void
*/
void overView();

/*
	负责人:liang
	截止11号晚上9点
	功能: rankView: 排行榜界面
		显示前10名的分数和对应昵称
		下方提示按ESC返回主菜单
	参数: void
	返回值: void
*/
void rankView();

/*
	负责人:句号
	截止11号晚上9点
	功能: settingView: 设置界面
		显示可修改选项:
			音效: <开/关>
			音量: <1-5>
		按WS移动光标,按AD修改选项
		按ESC返回主菜单
	参数: void
	返回值: void
*/
void settingView();

/*
	负责人:liang
	截止11号晚上9点
	功能: teamView: 团队介绍界面
		展示开发者名单和对应负责内容
		下方提示按ESC返回主菜单
	参数: void
	返回值: void
*/
void teamView();

/*
	负责人:安乐
	截止11号晚上9点
	功能: playMusic：播放背景音乐
		判断bgmSwitch，如果等于0，直接返回，如果等于1，就执行下面的操作
			1.先停止播放所有音乐
				mciSendString("stop all", NULL, 0, NULL);
			2.设置音量为Volume
			3.用switch case play参数指示的音乐
	参数: int
	返回值: void
*/
void playBackgroundMusic(int scene);

/*
	负责人:KC
	截止11号晚上9点
	功能: playMusic：播放音效
		判断soundEffectSwitch，如果等于0，直接返回，否则就执行下面的操作
			用switch case play参数指示的音效
	参数: int
	返回值: void
*/
void playSoundEffect(int type);

/*
	负责人:安乐
	截止11号晚上9点
	功能: open全部音乐资源
		在这个函数定义的别名是可以在别的地方用的
	参数: void
	返回值: void
*/
void openMusicResource(void);

/*
	负责人:安乐
	截止11号晚上9点
	功能: close全部音乐资源
	参数: void
	返回值: void
*/
void closeMusicResource(void);
// -------------------- view --------------------


// -------------------- 工具函数 --------------------
/*
	功能: gotoXY: 移动光标到指定位置
	参数:
		x: x坐标
		y: y坐标
	返回值: void
*/
void gotoXY(int x, int y);

/*
	功能: setPrintColor: 设置控制台颜色
	参数:
		color: 颜色代码
	返回值: void
*/
void setPrintColor(int color);

void printBox(int x, int y, int w, int h);

void HideCursor();

void clear(int x, int y, int w, int h);

void SetConsoleWindowSize(int widthPx, int heightPx);

void setVolume(char* bgmAliasName);

// -------------------- 工具函数 --------------------

int main()
{
	SetConsoleWindowSize(1024, 540);
	HideCursor();
	openMusicResource();
	menuView();
	return 0;
}

void init() {
	//在此处完成代码;
	//清屏
	system("cls");
	generateMap(difficulty);
	targetLength = 3;
	visibleLength = 3;
	score = 0;
	body[0].x = 54;
	body[1].x = 52;
	body[2].x = 50;
	body[0].y = 20;
	body[1].y = 20;
	body[2].y = 20;
	dir = 'd';
	hasRevive = 1;
	apples[0].active = 1;
	apples[0].type = 1;
	apples[0].x = 60;
	apples[0].y = 21;
	map[(60 - 42) / 2][21 - 4] = 2;
	for (int i = 1; i < 100; i++) {
		apples[i].active = 0;
		apples[i].x = 0;
		apples[i].y = 0;
		apples[i].type = 0;
	}
	appleCount = 0;
	poisonCount = 0;
	newAppleCount = 0;
}

void generateApple() {
	//在此处完成代码
	int index = -1;
	static int randNum = 0;
	for (int i = 0; i < 100; i++) {
		if (apples[i].active == 0) {
			index = i;
			break;
		}
	}

	// No vacant seat
	if (index == -1)return;

	//0-19 is wall
	/*generate rand coordinates*/
	int X;
	int Y;
	do {
		X = rand() % 18 + 1;
		Y = rand() % 18 + 1;
	} while (map[X][Y] != 0);

	map[X][Y] = 2;

	int type = 1;

	if (randNum == 0) {
		randNum = rand() % 6 + 5;
	}

	//special apple's probality
	int pro = rand() % 100;

	if (newAppleCount >= randNum) {
		if (pro < 5) {
			/*generate guardian angle*/
			type = 3;
		}
		else {
			/*generate poison apple*/
			type = 2;
		}

		newAppleCount = 0;
		randNum = rand() % 6 + 5;
	}

	apples[index].active = 1;
	apples[index].type = type;

	apples[index].x = X * 2 + 42;
	apples[index].y = Y + 4;
	if (apples[index].type != 1) {
		index++;
		do {
			X = rand() % 18 + 1;
			Y = rand() % 18 + 1;
		} while (map[X][Y] != 0);

		map[X][Y] = 2;
		apples[index].active = 1;
		apples[index].type = 1;

		apples[index].x = X * 2 + 42;
		apples[index].y = Y + 4;
	}

}

void generateMap(int difficulty) {
	//在此处完成代码
	for (int i = 1; i < 20; i++) {   //空地
		for (int j = 1; j < 20; j++) {
			map[i][j] = 0;
		}
	}
	for (int k = 0; k < 20; k++) {  //墙
		map[0][k] = 1;
		map[k][0] = 1;
		map[k][19] = 1;
		map[19][k] = 1;
	}
	//一般模式: difficulty = 0
	//困难模式: difficulty = 1
	if (difficulty == 1) {
		srand((unsigned)time(NULL));
		int num = rand() % 2;
		//地图一
		if (num == 0) {
			for (int ret1 = 4; ret1 < 6; ret1++) {
				map[4][ret1] = 1;
				map[14][ret1] = 1;
				map[4][ret1 + 10] = 1;
				map[14][ret1 + 10] = 1;
			}
			map[5][5] = 1;
			map[13][5] = 1;
			map[5][14] = 1;
			map[13][14] = 1;
			int sign = 8;
			map[sign - 1][sign] = 1;
			map[sign][sign] = 1;
			map[sign - 1][sign + 1] = 1;
			map[sign + 4][sign + 2] = 1;
			map[sign + 3][sign + 2] = 1;
			map[sign + 4][sign + 1] = 1;
		}
		//地图二
		if (num == 1) {
			for (int i = 6; i <= 12; i++) {
				map[i][9] = 1;
				map[9][i] = 1;
			}
			map[5][3] = 1;
			map[3][5] = 1;
			map[14][3] = 1;
			map[16][5] = 1;


			map[14][16] = 1;
			map[16][14] = 1;
			map[5][16] = 1;
			map[3][14] = 1;
		}
	}
}

void moveSnake(void) {
	//在此处完成代码

	//把旧的身体对应的map全部重置为0
	for (int i = 0; i < 20; i++) {
		for (int j = 0; j < 20; j++) {
			if (map[i][j] == 3 || map[i][j] == 4) {
				map[i][j] = 0;
			}
		}
	}

	//更正visiblelength
	if (visibleLength > targetLength) visibleLength = targetLength;


	//移动逻辑，后一节身体坐标等于前一节的身体坐标
	for (int i = visibleLength; i > 0; i--) {
		body[i].x = body[i - 1].x;
		body[i].y = body[i - 1].y;
	}

	//单独头移动
	switch (dir) {
	case 'w':
		body[0].y--;
		break;
	case 's':
		body[0].y++;
		break;
	case 'a':
		body[0].x -= 2;
		break;
	case 'd':
		body[0].x += 2;
		break;
	}

	//新头的坐标
	int headX = body[0].x;
	int headY = body[0].y;

	//更新map上的值
	map[(headX - 42) / 2][headY - 4] = 3;
	for (int i = 1; i < visibleLength; i++) {
		map[(body[i].x - 42) / 2][body[i].y - 4] = 4;
	}
}

int changeDirection(char newDirection) {
	//在此处完成代码
	if ((dir == 'd' && newDirection == 'a') ||
		(dir == 'a' && newDirection == 'd') ||
		(dir == 'w' && newDirection == 's') ||
		(dir == 's' && newDirection == 'w')) {
		return 0;
	}
	else {
		dir = newDirection;
		return 1;
	}
}


int calPosition(void) {
	//在此处完成代码
	int headNextX = body[0].x;
	int headNextY = body[0].y;

	switch (dir) {
	case 'w':
		headNextY--;
		break;
	case 's':
		headNextY++;
		break;
	case 'a':
		headNextX -= 2;
		break;
	case 'd':
		headNextX += 2;
		break;
	}

	int headx1 = (headNextX - 42) / 2;
	int heady1 = headNextY - 4;

	int point = map[headx1][heady1];

	if (point == 0) {
		return 2;
	}
	else if (point == 1 || point == 4) {
		if (hasRevive == 1) {
			//清理map数组中所有的头，身体
			for (int i = 0; i < 20; i++) {
				for (int j = 0; j < 20; j++) {
					if (map[i][j] == 3 || map[i][j] == 4) {
						map[i][j] = 0;
					}
				}
			}
			for (int i = 3; i < 400; i++) {
				body[i].x = 0;
				body[i].y = 0;
			}

			hasRevive = 0;
			//重置位置
			body[0].x = 54;
			body[1].x = 52;
			body[2].x = 50;
			body[0].y = 20;
			body[1].y = 20;
			body[2].y = 20;
			dir = 'd';
			visibleLength = 3;
			map[(54 - 42) / 2][20 - 4] = 3;
			map[(52 - 42) / 2][20 - 4] = 4;
			map[(50 - 42) / 2][20 - 4] = 4;
			return 1;
		}
		else {
			return 0;
		}
	}
	else if (point == 2) {
		for (int i = 0; i < 100; i++) {
			if (apples[i].active == 1 && apples[i].x == headNextX && apples[i].y == headNextY) {//在该苹果可用时，且该苹果与蛇的坐标要一致
				if (apples[i].type == 1) {//普通苹果
					appleCount++;
					newAppleCount++;
					targetLength++;
					score += 2;
					apples[i].active = 0;
					generateApple();
					return 3;
				}
				else if (apples[i].type == 2) {//毒苹果
					poisonCount++;
					targetLength -= poisonCount;
					targetLength = targetLength < 3 ? 3 : targetLength;
					score -= 2 * poisonCount;
					score = score < 0 ? 0 : score;//避免出现负数

					isPoisoned = 1;
					poison_time = GetTickCount();  // 记录中毒开始
					apples[i].active = 0;
					return 4;
				}
				else if (apples[i].type == 3) {//复活甲
					if (hasRevive == 1) {
						targetLength += 5;
						score += 10;
					}
					else {
						hasRevive++;
					}
					return 5;
				}
			}
		}
	}
	return 2;

}

void saveScore(char* inputName) {
	char names[10][20] = { 0 };
	int scores[10] = { 0 };
	int count = 0;
	char arr[1024] = { 0 };
	const char* filePath = "paihangbang.txt";

	FILE* fp = fopen(filePath, "a+");
	if (fp != NULL) {
		while (fgets(arr, 1024, fp) != NULL && count < 10) {
			if (sscanf_s(arr, "%*d、%19s %d", names[count], (unsigned)_countof(names[count]), &scores[count]) == 2) {
				count++;
			}
		}
		fclose(fp);
	}
	
	int needSort = 0;
	if (count < 10) {
		strcpy_s(names[count], _countof(names[count]), inputName);
		scores[count] = score;
		count++;
		needSort = 1;
	}
	else {

		int minIndex = 0;
		int minScore = scores[0];
		for (int i = 1; i < 10; i++) {
			if (scores[i] < minScore) {
				minScore = scores[i];
				minIndex = i;
			}
		}

		if (score > minScore) {
			strcpy_s(names[minIndex], _countof(names[minIndex]), inputName);
			scores[minIndex] = score;
			needSort = 1;
		}
		else {
			//printf("你的分数(%d分)未超过排行榜最低分(%d分)，未保存！\n", score, minScore);
		}
	}

	if (needSort) {

		for (int i = 0; i < count - 1; i++) {
			for (int j = 0; j < count - 1 - i; j++) {
				if (scores[j] < scores[j + 1]) {

					int tempScore = scores[j];
					scores[j] = scores[j + 1];
					scores[j + 1] = tempScore;

					char tempName[20] = { 0 };
					strcpy_s(tempName, _countof(tempName), names[j]);
					strcpy_s(names[j], _countof(names[j]), names[j + 1]);
					strcpy_s(names[j + 1], _countof(names[j + 1]), tempName);
				}
			}
		}

		FILE* file = NULL;
		errno_t err = fopen_s(&file, filePath, "w");
		if (err != 0 || file == NULL) {
			printf("文件打开失败！\n");
			return;
		}
		for (int i = 0; i < count; i++) {
			fprintf_s(file, "%d、%s %d\n", i + 1, names[i], scores[i]);
		}
		fclose(file);
		
	}


}//在此处完成代码


void menuView()
{
	playBackgroundMusic(BGMMAIN);
	//菜单蛇进行初始化
	body[0] = (Snake){ 40, 26 };
	body[1] = (Snake){ 38, 26 };
	body[2] = (Snake){ 36, 26 };
	char newDirection;
	int oldBodyX[3] = { 0 };
	int oldBodyY[3] = { 0 };
	visibleLength = 3;
	targetLength = 3;
	dir = 'd';

	system("cls");
	setPrintColor(GOLDEN);
	gotoXY(50, 3); printf("贪吃蛇历险记");

	setPrintColor(RED);
	gotoXY(46, 8); printf("果");
	gotoXY(46, 11); printf("果");
	gotoXY(46, 14); printf("果");
	gotoXY(46, 17); printf("果");
	gotoXY(46, 20); printf("果");

	setPrintColor(DEFAULT);
	gotoXY(60, 5); printf("--By  程风破浪");
	gotoXY(50, 8); printf("开始游戏");
	gotoXY(50, 11); printf("排行榜");
	gotoXY(50, 14); printf("游戏设置");
	gotoXY(50, 17); printf("团队介绍");
	gotoXY(50, 20); printf("退出游戏");

	while (1) {

		if (_kbhit()) {
			newDirection = _getch();
			if (newDirection == 'w' || newDirection == 'a' || newDirection == 's' || newDirection == 'd') {
				changeDirection(newDirection);
			}
		}

		for (int i = 0; i < 3; i++) {
			oldBodyX[i] = body[i].x;
			oldBodyY[i] = body[i].y;
		}

		for (int i = visibleLength - 1; i > 0; i--) {
			body[i].x = body[i - 1].x;
			body[i].y = body[i - 1].y;
		}

		//单独头移动
		switch (dir) {
		case 'a':
			body[0].x -= 2;
			break;
		case 'd':
			body[0].x += 2;
			break;
		case 'w':
			body[0].y--;
			break;
		case 's':
			body[0].y++;
			break;
		}

		if (body[0].x < 2)
			body[0].x = 80;//上>下

		if (body[0].x > 80)
			body[0].x = 2;//下>上

		if (body[0].y < 2)//左>右
			body[0].y = 30;

		if (body[0].y > 30)//右>左
			body[0].y = 2;

		if (body[0].x == 46 && body[0].y == 8)
			modelSelectView();

		if (body[0].x == 46 && body[0].y == 11)
			rankView();

		if (body[0].x == 46 && body[0].y == 14)
			settingView();

		if (body[0].x == 46 && body[0].y == 17)
			teamView();

		if (body[0].x == 46 && body[0].y == 20) {
			system("cls");
			gotoXY(50, 13);
			printf("欢迎您下次游玩");
			Sleep(1000);
			closeMusicResource();
			exit(0);
		}

		for (int i = 0; i < 3; i++) {
			setPrintColor(DEFAULT);
			gotoXY(oldBodyX[i], oldBodyY[i]);
			printf("  ");
			setPrintColor(GREEN);
			gotoXY(body[i].x, body[i].y);

			printf(i == 0 ? "头" : "身");
		}
		setPrintColor(DEFAULT);

		Sleep(150);
	}
}

void modelSelectView() {
	//在此完成代码
	body[0] = (Snake){ 40, 26 };
	body[1] = (Snake){ 38, 26 };
	body[2] = (Snake){ 36, 26 };
	char newDirection;
	int choice;
	int oldBodyX[3] = { 0 };
	int oldBodyY[3] = { 0 };
	visibleLength = 3;
	targetLength = 3;
	dir = 'd';

	system("cls");
	setPrintColor(RED);
	gotoXY(50, 3); printf("贪吃蛇历险记");
	gotoXY(46, 8); printf("果");
	gotoXY(46, 14); printf("果");
	gotoXY(46, 20); printf("果");

	setPrintColor(DEFAULT);
	gotoXY(50, 8); printf("一般模式");
	gotoXY(50, 14); printf("困难模式");
	gotoXY(50, 20); printf("返回主菜单");

	while (1) {

		if (_kbhit()) {
			newDirection = _getch();
			if (newDirection == 'w' || newDirection == 'a' || newDirection == 's' || newDirection == 'd') {
				changeDirection(newDirection);
			}
		}

		for (int i = 0; i < 3; i++) {
			oldBodyX[i] = body[i].x;
			oldBodyY[i] = body[i].y;
		}

		for (int i = visibleLength - 1; i > 0; i--) {
			body[i].x = body[i - 1].x;
			body[i].y = body[i - 1].y;
		}

		//单独头移动
		switch (dir) {
		case 'a':
			body[0].x -= 2;
			break;
		case 'd':
			body[0].x += 2;
			break;
		case 'w':
			body[0].y--;
			break;
		case 's':
			body[0].y++;
			break;
		}

		if (body[0].x < 2)
			body[0].x = 80;//上>下

		if (body[0].x > 80)
			body[0].x = 2;//下>上

		if (body[0].y < 2)//左>右
			body[0].y = 30;

		if (body[0].y > 30)//右>左
			body[0].y = 2;

		if (body[0].x == 46 && body[0].y == 8) {
			difficulty = 0;
			choice = 1;
			break;
		}

		if (body[0].x == 46 && body[0].y == 14) {
			difficulty = 1;
			choice = 2;
			break;
		}

		if (body[0].x == 46 && body[0].y == 20) {
			choice = 0;
			break;
		}

		for (int i = 0; i < 3; i++) {
			setPrintColor(DEFAULT);
			gotoXY(oldBodyX[i], oldBodyY[i]);
			printf("  ");
			setPrintColor(GREEN);
			gotoXY(body[i].x, body[i].y);
			printf(i == 0 ? "头" : "身");
		}
		setPrintColor(DEFAULT);

		Sleep(150);
	}
	if (choice == 0) menuView();
	else gameView();
}

void gameViewShowMap() {
	//在此处完成代码
	int left = 41;
	int right = 82;
	int top = 3;
	int bottom = 24;
	//上边界
	gotoXY(left, top);
	for (int i = left; i < right + 1; i++) {
		printf("-");
	}
	//下边界
	gotoXY(left, bottom);
	for (int i = left; i < right + 1; i++) {
		printf("-");
	}

	for (int j = top + 1; j < bottom; j++) {
		gotoXY(left, j);
		printf("|");//左边界
		gotoXY(right, j);
		printf("|");//右边界
	}

	//在这里添加代码
	//你需要遍历map数组，把值为1的的数组下标，映射为群里的坐标系中的坐标，然后gotoXY（）打印成你设计的元素
	//定义一个字符串二维数组
	char* things[] = { "火", "刺", "岩" };

	for (int i = 0; i < 20; i++) {
		for (int j = 0; j < 20; j++) {
			gotoXY(i * 2 + 42, j + 4); //这里很重要，映射坐标
			//打印空地
			if (map[i][j] == 0) {
				printf("  ");
			}
			//打印墙和障碍物
			else if (map[i][j] == 1) { //要增加一个实体的判断
				if (i == 0 || i == 19 || j == 0 || j == 19) {
					printf("墙");//边缘打印墙
				}
				else {
					//i 对数组长度取余，返回值作为things的下标，这样会动态选择字符串数组里的元素，然后打印
					printf("%s", things[i % (sizeof(things) / sizeof(things[0]))]);
				}
			}
		}
	}

	//打印蛇
	//根据状态修改颜色，中毒最优先
	if (hasRevive == 1) {
		setPrintColor(GOLDEN);
	}
	else {
		setPrintColor(GREEN);
	}

	if (isPoisoned == 1) {
		//检测时间是否过了5秒
		//否就要改成紫色打印
		DWORD current_time = GetTickCount();
		DWORD poison_duration = current_time - poison_time;

		if (poison_duration >= POISON_DURATION) {
			isPoisoned = 0;
		}
		else {
			setPrintColor(PURPLE);
		}
	}

	for (int i = 0; i < visibleLength; i++) {
		gotoXY(body[i].x, body[i].y);

		if (i == 0)
			printf("头");
		else
			printf("身");

	}

	clear(1, 1, 20, 30);
	//打印苹果
	for (int i = 0; i < 100; i++) {
		if (apples[i].active != 1) continue;

		setPrintColor(PURPLE);
		if (apples[i].type == 1) setPrintColor(RED);

		gotoXY(apples[i].x, apples[i].y);
		printf("果");
	}
	setPrintColor(DEFAULT);

}

void gameViewShowInfo() {
	// 信息栏起始 X 坐标放在地图右边
	int infoX = 86;
	// 1. 标题
	gotoXY(infoX, 3);
	printf("=== 游戏信息 ===");

	// 2. 游戏数据
	gotoXY(infoX, 5);
	printf("当前分数: ");
	printf("%05d", score);

	gotoXY(infoX, 7);
	printf("当前总长度: ");
	printf("%3d", targetLength);

	gotoXY(infoX, 9);
	printf("游戏难度: %s", difficulty ? "困难模式" : "一般模式");

	gotoXY(infoX, 11);
	printf("复活甲状态: ");
	if (hasRevive == 1) {
		printf("已激活");
	}
	else {
		printf("未获取");
	}

	gotoXY(infoX, 13);
	printf("苹果数量:%d", appleCount);
	gotoXY(infoX, 14);
	printf("毒苹果数量:%d", poisonCount);
	// 3. 绘制操作说明
	gotoXY(infoX, 15);
	printf("=== 操作说明 ===");
	gotoXY(infoX, 16);
	printf("W/A/S/D - 上下左右");
	gotoXY(infoX, 17);
	printf("空格    - 暂停");
}


void gameView() {
	playBackgroundMusic(BGMGAME);
	int choice = 0;
	init();
	while (1) {
		gameViewShowMap();
		gameViewShowInfo();

		if (_kbhit()) {
			char input = _getch();
			if (input == 'w' ||
				input == 'a' ||
				input == 's' ||
				input == 'd') changeDirection(input);
			else if (input == ' ') {
				choice = pauseView();
				if (choice != 0) break;
			}
		}

		int result = calPosition();

		if (result == 0)break;
		playSoundEffect(result);
		moveSnake();

		if (visibleLength < targetLength) visibleLength++;
		if (difficulty == 0) {
			Sleep(150);
		}
		else {
			Sleep(150);
		}
	}

	if (choice == 1) modelSelectView();
	else if (choice == 2) menuView();
	else overView();
}

int pauseView() {

	clear(42, 4, 40, 20);

	gotoXY(58, 6);
	printf("暂停界面");

	int userChoose = 0;

	while (1) {
		// ==================== 打印界面 ====================

		// option0：continue game
		if (userChoose == 0) setPrintColor(0x6f);
		gotoXY(58, 9);
		printf("继续游戏");
		if (userChoose == 0) setPrintColor(0x0f);

		// option1：restart game
		if (userChoose == 1) setPrintColor(0x6f);
		gotoXY(58, 12);
		printf("重新开始");
		if (userChoose == 1) setPrintColor(0x0f);

		// option2：return to main menu
		if (userChoose == 2) setPrintColor(0x6f);
		gotoXY(58, 15);
		printf("返回主菜单");
		if (userChoose == 2) setPrintColor(0x0f);



		// ==================== 接收输入 ====================
		char input = _getch();

		// 处理方向键
		if (input == -32 || input == 227) {
			input = _getch();
			switch (input) {
			case 72: // 上
				userChoose = (userChoose - 1 + 3) % 3;
				break;
			case 80: // 下
				userChoose = (userChoose + 1) % 3;
				break;
			}
		}
		else { // 处理字母键
			switch (input) {
			case 'w':
				userChoose = (userChoose - 1 + 3) % 3;
				break;
			case 's':
				userChoose = (userChoose + 1) % 3;
				break;
			case '\r':
				clear(42, 4, 40, 20);
				return userChoose;
			}
		}
	}
}

void overView() {
	playBackgroundMusic(BGMOVER);
	system("cls"); // 清屏
	gotoXY(50, 5); printf("===== 游戏结束 =====\n");
	gotoXY(50, 7); printf("本局分数：%d\n", score);
	gotoXY(50, 8); printf("蛇最终长度：%d\n", visibleLength);

	// 判断是否进入排行榜
	int isInRank = 0;
	char inputName[20] = { 0 };
	for (int i = 0; i < 10; i++) {
		if (score > ranks[i].score) {
			isInRank = 1;
			break;
		}
	}
	if (isInRank) {
		gotoXY(50, 10); printf("恭喜，你的分数进入排行榜前10\n");
		gotoXY(50, 12); printf("请输入你的昵称（最多12个字符）：");
		
		scanf_s("%13s", inputName, (unsigned)_countof(inputName));
		saveScore(inputName);
	}
	else {
		gotoXY(50, 10); printf("未能进入排行榜\n");
	}

	gotoXY(50, 14); printf("按ESC返回主菜单...\n");
	// 返回主菜单逻辑
	while (1) {
		if (_kbhit()) {
			char key = _getch();
			if (key == 27) { // ESC键ASCII码
				system("cls");
				menuView();
				break;
			}
		}
	}
}//在此处完成代码

void rankView() {
	playBackgroundMusic(BGMRANK);
	HideCursor();
	system("cls");
	FILE* fp = fopen("paihangbang.txt", "at+");
	int i = 0;
	int count = 0;
	int numberCount = 0;
	char arr[1024];
	setPrintColor(14);
	printBox(65, 4, 12, 3);//rank box
	setPrintColor(7);

	//main box
	setPrintColor(11);
	printBox(52, 7, 40, 15);
	setPrintColor(7);

	//rank
	setPrintColor(12);
	gotoXY(68, 5);
	printf("排行榜");
	setPrintColor(7);

	gotoXY(55, 9);
	printf("排名\t   昵称\t          分数");
	// 读取文件内容
	while (fgets(arr, 1024, fp) != NULL && count < 10) {
		gotoXY(55, 10 + count);
		
		printf(" ");
		if (count < 9) {
			while (arr[i] != '\0') {
				if (i == 3) {
					setPrintColor(14);
					printf("\t   %c", arr[i]);
					setPrintColor(7);
					numberCount++;
				}
				else if (i >= 0 && i < 3) {

					setPrintColor(9);
					printf("%c", arr[i]);
					setPrintColor(7);
				}
				else {
					setPrintColor(14);
					printf("%c", arr[i]);
					setPrintColor(7);
					numberCount++;

				}
				i++;
				if (i >= 3 && (arr[i - 1] != '\0') && arr[i] == ' ')break;

			}
			i++;
			if (numberCount < 5) {
				printf("\t\t   ");
			}
			else {

				for (int i = 0; i < 16 - numberCount; i++) {
					printf(" ");
				}
			}

			while (arr[i] != '\0') {
				setPrintColor(12);
				printf("%c", arr[i]);
				setPrintColor(7);
				i++;
			}


		}
		else {
			while (arr[i] != '\0') {
				if (i >= 0 && i <= 3) {
					setPrintColor(9);
					printf("%c", arr[i]);
					setPrintColor(7);
				}
				else if (i == 4) {
					setPrintColor(14);
					printf("\t   %c", arr[i]);
					setPrintColor(7);
				}
				else {
					setPrintColor(14);
					printf("%c", arr[i]);
					setPrintColor(7);
				}
				i++;
				if (i >= 4 && (arr[i - 1] != '\0') && arr[i] == ' ')break;

			}
			i++;
			printf("\t\t   ");
			while (arr[i] != '\0') {
				setPrintColor(12);
				printf("%c", arr[i]);
				setPrintColor(7);
				i++;
			}
		}
		count++;
		i = 0;
		numberCount = 0;
	}

	fclose(fp);

	printf("\n\n\n\n按ESC键退出...\n");
	while (1) {
		if (_kbhit()) {
			int key = _getch();
			if (key == 27) {
				menuView();
			}
		}
	}
	//在此处完成代码

}

void settingView() {
	playBackgroundMusic(BGMSETTING);
	system("cls");
	int cursor = 0; // 光标位置：0=音效 1=音量
	int volumeArr[] = { 1, 2, 3, 4, 5 }; // 音量档位1-5
	int volumeIdx = bgmVolume / 200 - 1; // 默认音量为当前音量

	while (1) {
		// 绘制设置界面
		gotoXY(35, 2); printf("===== 游戏设置 =====\n");

		// 音乐选项
		if (cursor == 0)setPrintColor(0x6f);
		gotoXY(38, 6);
		printf("音乐：< %2s >", bgmSwitch ? "开" : "关");
		if (cursor == 0)setPrintColor(0x0f);

		// 音效选项
		if (cursor == 1)setPrintColor(0x6f);
		gotoXY(38, 8);
		printf("音效：< %2s >", soundEffectSwitch ? "开" : "关");
		if (cursor == 1)setPrintColor(0x0f);

		// 音量选项
		if (cursor == 2)setPrintColor(0x6f);
		gotoXY(38, 10);
		printf("音量：< %2d   >", volumeArr[volumeIdx]);
		if (cursor == 2)setPrintColor(0x0f);

		// 操作提示
		gotoXY(64, 2); 
		printf("===== 操作提示 =====");
		gotoXY(65, 6);
		printf("W/S  移动光标");
		gotoXY(65, 8);
		printf("A/D  修改选项");
		gotoXY(65, 10);
		printf("ESC  保存并返回主菜单");

		// 处理键盘输入
		char key = _getch();
		playSoundEffect(6);
		switch (key) {
		case 'w': // 上移光标
			cursor--;
			if (cursor < 0) cursor = 2;
			break;
		case 's': // 下移光标
			cursor++;
			if (cursor > 2) cursor = 0;
			break;
		case 'a': // 左修改：音效关/音量-
			switch (cursor) {
			case 0:
				bgmSwitch = 0;
				break;
			case 1:
				soundEffectSwitch = 0;
				break;
			case 2:
				volumeIdx--;
				if (volumeIdx < 0) volumeIdx = 4;
				break;
			}
			break;
		case 'd': // 右修改：音效开/音量+
			switch (cursor) {
			case 0:
				bgmSwitch = 1;
				break;
			case 1:
				soundEffectSwitch = 1;
				break;
			case 2:
				volumeIdx++;
				if (volumeIdx > 4) volumeIdx = 0;
				break;
			}
			break;
		case 27: // ESC返回主菜单
			system("cls");
			// 保存最终音量设置
			bgmVolume = volumeArr[volumeIdx] * 200;
			menuView();
		}
	}
	
}//在此处完成代码


void teamView() {
	playBackgroundMusic(BGMTEAM);
	system("cls");
	HideCursor();
	setPrintColor(14);
	printBox(60, 2, 17, 5);//teamview box
	setPrintColor(7);

	setPrintColor(11);
	printBox(75, 7, 40, 18);//funtion box right
	printBox(25, 7, 40, 18);//funtion box left
	setPrintColor(7);

	gotoXY(65, 4);
	setPrintColor(12);
	printf("团队介绍");
	setPrintColor(7);

	//----------- box left---------//

	gotoXY(30, 8);
	setPrintColor(15);
	printf("昵称\t\t  分工");
	gotoXY(82, 8);
	printf("昵称\t\t   分工");
	setPrintColor(7);


	//name
	setPrintColor(13);
	gotoXY(30, 9);
	printf("1.kc");
	gotoXY(30, 13);
	printf("2.十一");
	gotoXY(30, 17);
	printf("3.小王");
	gotoXY(30, 20);
	printf("4.安樂");
	gotoXY(80, 9);
	printf("5.阿sii");
	gotoXY(80, 12);
	printf("6.句号");
	gotoXY(80, 16);
	printf("7.星辰");
	gotoXY(80, 20);
	printf("8.liang");
	setPrintColor(7);

	//funtion
	setPrintColor(14);
	gotoXY(48, 9);
	printf("moveSnake");
	gotoXY(45, 10);
	printf("modelSelectView");
	gotoXY(50, 12);
	printf("init");
	gotoXY(45, 13);
	printf("gameViewShowMap");
	gotoXY(45, 14);
	printf("gameViewShowInfo");
	gotoXY(45, 16);
	printf("generateApple");
	gotoXY(45, 17);
	printf("gameView");
	gotoXY(45, 18);
	printf("pauseView");
	gotoXY(45, 20);
	printf("playBackgroundMusic");
	gotoXY(95, 9);
	printf("changeDirection");
	gotoXY(95, 11);
	printf("generateMap");
	gotoXY(95, 12);
	printf("overView");
	gotoXY(95, 13);
	printf("settingView");
	gotoXY(95, 16);
	printf("calPosition");
	gotoXY(95, 17);
	printf("menuView");
	gotoXY(95, 19);
	printf("teamView");
	gotoXY(95, 20);
	printf("saveScore");
	gotoXY(95, 21);
	printf("rankView");
	setPrintColor(7);


	//line
	setPrintColor(9);
	gotoXY(26, 11);
	printf("———————————————————-------------------");
	gotoXY(26, 15);
	printf("———————————————————-------------------");
	gotoXY(26, 19);
	printf("———————————————————-------------------");
	gotoXY(76, 10);
	printf("———————————————————-------------------");
	gotoXY(76, 15);
	printf("———————————————————-------------------");
	gotoXY(76, 18);
	printf("———————————————————-------------------");
	setPrintColor(7);


	//return 
	printf("\n\n\n\n\n\n\n按ESC键退出...\n");
	while (1) {
		if (_kbhit()) {
			int key = _getch();
			if (key == 27) {
				system("cls");
				menuView();
			}
		}
	}
}

// 传入坐标, 将光标移动到指定坐标
void gotoXY(int x, int y)
{
	COORD c;
	c.X = x - 1;
	c.Y = y - 1;
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), c);
}
// 清理指定矩形区域 矩形左上角坐标(x,y)  矩形宽w高h
void clear(int x, int y, int w, int h)
{
	for (int i = 0; i < h; i++) {
		gotoXY(x, y + i);
		for (int j = 0; j < w; j++) putchar(' ');
	}
}
// 打印边框
void printBox(int x, int y, int w, int h)
{
	// 左竖线
	for (int i = 0; i < h; i++) {
		gotoXY(x, y + i);
		putchar('|');
	}
	// 右竖线
	for (int i = 0; i < h; i++) {
		gotoXY(x + w - 1, y + i);
		putchar('|');
	}
	// 上横线
	gotoXY(x, y);
	for (int i = 0; i < w; i++) {
		putchar('-');
	}
	// 下横线
	gotoXY(x, y + h - 1);
	for (int i = 0; i < w; i++) {
		putchar('-');
	}
	// 光标挪到其它位置  避免后面的输出覆盖这里的打印
	gotoXY(x + w, y + h + 1);
}

// 隐藏光标
void HideCursor() {
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_CURSOR_INFO cursorInfo;
	GetConsoleCursorInfo(hConsole, &cursorInfo);
	cursorInfo.bVisible = FALSE;
	SetConsoleCursorInfo(hConsole, &cursorInfo);
}

void setPrintColor(int color) {
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}

void SetConsoleWindowSize(int widthPx, int heightPx)
{
	HWND hWnd = GetConsoleWindow();
	if (hWnd == NULL)
	{
		//std::cerr << "无法获取控制台窗口句柄" << std::endl;
		return;
	}

	// 获取当前窗口位置
	RECT rect;
	GetWindowRect(hWnd, &rect);
	int x = rect.left;
	int y = rect.top;

	// 修改窗口大小（不改变位置）
	SetWindowPos(
		hWnd,
		NULL,
		x, y,          // 左上角坐标
		widthPx,       // 新宽度
		heightPx,      // 新高度
		SWP_NOZORDER   // 不改变 Z 序
	);
}

void setVolume(char* bgmAliasName) {
	wchar_t cmd[MAX_PATH];
	wsprintf(cmd, L"setaudio %hs volume to %d", bgmAliasName, bgmVolume);
	mciSendString(cmd, NULL, 0, NULL);
	return;
}

void playBackgroundMusic(int bgmName) {
	//在此完成代码
	//如果bgmSwitch==0，表示用户关闭了bgm
	//先停止所有的音乐
	mciSendString(L"stop all", 0, NULL, NULL);

	if (bgmSwitch == 0) return;

	//判断是那个界面
	switch (bgmName) {
	case BGMGAME: // 游戏界面
		mciSendString(L"play game from 0 repeat ", 0, NULL, NULL);
		setVolume("game");
		break;
	case BGMSETTING: // 设置界面
		mciSendString(L"play setting from 0 repeat", 0, NULL, NULL);
		setVolume("setting");
		break;
	case BGMTEAM: // 团队
		mciSendString(L"play team from 0 repeat", 0, NULL, NULL);
		setVolume("team");
		break;
	case BGMRANK: // 排行榜
		mciSendString(L"play rank from 0 repeat", 0, NULL, NULL);
		setVolume("rank");
		break;
	case BGMOVER: // 结算
		mciSendString(L"play over from 0 repeat", 0, NULL, NULL);
		setVolume("over");
		break;
	case BGMMAIN:
		mciSendString(L"play main from 0 repeat", 0, NULL, NULL);
		setVolume("main");
		break;
	}
}

void playSoundEffect(int type) {
	if (soundEffectSwitch == 0) return;
	switch (type) {
	case 1:
		mciSendString(L"close hitting", 0, NULL, NULL);
		mciSendString(L"open ../soundEffect/hitting.wav alias hitting type mpegvideo", 0, NULL, NULL);
		mciSendString(L"play hitting", 0, NULL, NULL);
		mciSendString(L"setaudio hitting volume to 800", 0, NULL, NULL);
		break;
	case 3:
		mciSendString(L"close apple", 0, NULL, NULL);
		mciSendString(L"open ../soundEffect/apple.wav alias apple type mpegvideo", 0, NULL, NULL);
		mciSendString(L"play apple", 0, NULL, NULL);
		mciSendString(L"setaudio apple volume to 800", 0, NULL, NULL);
		break;
	case 4:
		mciSendString(L"close poison", 0, NULL, NULL);
		mciSendString(L"open ../soundEffect/poison.wav alias poison type mpegvideo", 0, NULL, NULL);
		mciSendString(L"play poison", 0, NULL, NULL);
		mciSendString(L"setaudio poison volume to 800", 0, NULL, NULL);
		break;
	case 5:
		mciSendString(L"close revive", 0, NULL, NULL);
		mciSendString(L"open ../soundEffect/revive.wav alias revive type mpegvideo", 0, NULL, NULL);
		mciSendString(L"play revive", 0, NULL, NULL);
		mciSendString(L"setaudio revive volume to 800", 0, NULL, NULL);
		break;
	case 6:
		mciSendString(L"close click", 0, NULL, NULL);
		mciSendString(L"open ../soundEffect/click.wav alias click type mpegvideo", 0, NULL, NULL);
		mciSendString(L"play click", 0, NULL, NULL);
		mciSendString(L"setaudio click volume to 400", 0, NULL, NULL);
		break;
	}
}
void openMusicResource(void) {
	//在此完成代码
	gotoXY(50, 15);
	printf("Loading Resources...");
	mciSendString(L"open ../bgm/bgm_game.wav alias game type mpegvideo", 0, NULL, NULL);
	mciSendString(L"open ../bgm/bgm_over.wav alias over type mpegvideo", 0, NULL, NULL);
	mciSendString(L"open ../bgm/bgm_rank.wav alias rank type mpegvideo", 0, NULL, NULL);
	mciSendString(L"open ../bgm/bgm_setting.wav alias setting type mpegvideo", 0, NULL, NULL);
	mciSendString(L"open ../bgm/bgm_team.wav alias team type mpegvideo", 0, NULL, NULL);
	mciSendString(L"open ../bgm/bgm_main.wav alias main type mpegvideo", 0, NULL, NULL);
	Sleep(3000);
}

void closeMusicResource(void) {
	//在此完成代码
	system("cls");
	gotoXY(50, 15);
	printf("Releasing Resources...");
	mciSendString(L"close game", 0, NULL, NULL);
	mciSendString(L"close over", 0, NULL, NULL);
	mciSendString(L"close rank", 0, NULL, NULL);
	mciSendString(L"close setting", 0, NULL, NULL);
	mciSendString(L"close team", 0, NULL, NULL);
	mciSendString(L"close main", 0, NULL, NULL);
	Sleep(2000);
}
