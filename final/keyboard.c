/* �L�[�{�[�h�֌W */

#include "bootpack.h"
#include<stdio.h>

struct FIFO32 *keyfifo;
int keydata0;
extern int count,grade;
extern struct TIMER *timer4;
extern struct FIFO32 fifo;
void inthandler21(int *esp)
{
	int data;
	io_out8(PIC0_OCW2, 0x61);	/* IRQ-01��t������PIC�ɒʒm */
	data = io_in8(PORT_KEYDAT);
	fifo32_put(keyfifo, data + keydata0);
	return;
}

#define PORT_KEYSTA				0x0064
#define KEYSTA_SEND_NOTREADY	0x02
#define KEYCMD_WRITE_MODE		0x60
#define KBC_MODE				0x47

void wait_KBC_sendready(void)
{
	/* �L�[�{�[�h�R���g���[�����f�[�^���M�\�ɂȂ�̂�҂� */
	for (;;) {
		if ((io_in8(PORT_KEYSTA) & KEYSTA_SEND_NOTREADY) == 0) {
			break;
		}
	}
	return;
}

void init_keyboard(struct FIFO32 *fifo2, int data0)
{
	/* �������ݐ��FIFO�o�b�t�@���L�� */
	keyfifo = fifo2;
	keydata0 = data0;
	/* �L�[�{�[�h�R���g���[���̏����� */
	wait_KBC_sendready();
	io_out8(PORT_KEYCMD, KEYCMD_WRITE_MODE);
	wait_KBC_sendready();
	io_out8(PORT_KEYDAT, KBC_MODE);
	return;
}

//game.c
//界面
struct SNAKE snake;
struct BODY body[game_row*game_col];
int face[game_row][game_col];//记录每个小格的状态（食物，墙，蛇）

void initgameface(struct SHEET* sht)
{
    boxfill8(sht->buf,sht->bxsize,0,0,0,sht->bxsize,sht->bysize);
    //boxfill8(sht->buf,sht->bxsize,0,boxsize,boxsize,sht->bxsize-boxsize,sht->bysize-boxsize);
    int i,j;
    for(i=0;i<game_row;i++){
        for(j=0;j<game_col;j++){
            if(i==0||i==game_row-1||j==0||j==game_col-1){
                face[i][j]=wall;
				boxfill8(sht->buf,sht->bxsize,11,j*boxsize,i*boxsize,j*boxsize+boxsize-boxgap,i*boxsize+boxsize-boxgap);
            }
            else face[i][j]=empty;
        }
    }
	//  障碍物：
	for(i=5;i<8;i++){
		for(j=5;j<8;j++){
			face[i][j]=wall;
			boxfill8(sht->buf,sht->bxsize,11,j*boxsize,i*boxsize,j*boxsize+boxsize-boxgap,i*boxsize+boxsize-boxgap);
		}
	}

	for(i=15;i<18;i++){
		for(j=15;j<18;j++){
			face[i][j]=wall;
			boxfill8(sht->buf,sht->bxsize,11,j*boxsize,i*boxsize,j*boxsize+boxsize-boxgap,i*boxsize+boxsize-boxgap);
		}
	}

	for(i=5;i<8;i++){
		for(j=25;j<28;j++){
			face[i][j]=wall;
			boxfill8(sht->buf,sht->bxsize,11,j*boxsize,i*boxsize,j*boxsize+boxsize-boxgap,i*boxsize+boxsize-boxgap);
		}
	}
	
	char s[20];
	sprintf(s,"SCORE:%d ",grade);
	putfonts8_asc_sht(sht,sht->bxsize/2-40,0,10,7,s,9);	
	//init snake
	snake.len = 2; //蛇的身体长度
	snake.x = game_col / 2; //蛇头位置的横坐标
	snake.y = game_row / 2; //蛇头位置的纵坐标
	//蛇身坐标的初始化
	body[0].x = game_col / 2 - 1;
	body[0].y = game_row / 2;
	body[1].x = game_col/ 2 - 2;
	body[1].y = game_row / 2;
	//将蛇头和蛇身位置进行标记
	face[snake.y][snake.x] = Body;
	face[body[0].y][body[0].x] = Body;
	face[body[1].y][body[1].x] = Body;
	draw_snake(sht,1);
}
void RandFood(struct SHEET* sht)
{
	int i=10,j=10;
	char s[10];
		//sprintf(s,"%d",count);
		//putfonts8_asc_sht(sht,sht->bxsize/2-20,80,1,7,s,10);
		//sheet_refresh(sht,sht->bxsize/2-20,80,sht->bxsize/2-20+40,80+20);
	while(1){
		i=count%game_row;
		j=count%game_col;
		if(face[i][j]==empty)break;
		count++;
	}
	face[i][j]=food;
	draw_cycle(sht->buf,sht->bxsize,sht->bysize,j*boxsize+boxsize/2-boxgap/2,i*boxsize+boxsize/2-boxgap/2,boxsize/2-boxgap/2-2,1);

}

void draw_snake(struct SHEET* sht,int flag)
{
	int i;
	if(flag){
		boxfill8(sht->buf,sht->bxsize,headcol,snake.x*boxsize,snake.y*boxsize,snake.x*boxsize+boxsize-boxgap,snake.y*boxsize+boxsize-boxgap);
		for(i=0;i<snake.len;i++){
			int x=body[i].x*boxsize,y=body[i].y*boxsize;
			boxfill8(sht->buf,sht->bxsize,bodycol,x,y,x+boxsize-boxgap,y+boxsize-boxgap);
		}
	}
	else {
		if(body[snake.len-1].x!=0){
			int x=body[snake.len-1].x*boxsize,y=body[snake.len-1].y*boxsize;
			boxfill8(sht->buf,sht->bxsize,backcol,x,y,x+boxsize-boxgap,y+boxsize-boxgap);
		}
	}

}
void  movesnake(struct SHEET* sht,int x, int y)
{
	if(judgefunc(sht,x,y)==wall)return wall;
	draw_snake(sht,0); //先覆盖当前所显示的蛇
	face[body[snake.len - 1].y][body[snake.len - 1].x] = empty; //蛇移动后蛇尾重新标记为空
	face[snake.y][snake.x] = Body; //蛇移动后蛇头的位置变为蛇身
	//蛇移动后各个蛇身位置坐标需要更新
	int i;
	for (i = snake.len - 1; i > 0; i--)
	{
		body[i].x = body[i - 1].x;
		body[i].y = body[i - 1].y;
	}
	//蛇移动后蛇头位置信息变为第0个蛇身的位置信息
	body[0].x = snake.x;
	body[0].y = snake.y;
	//蛇头的位置更改
	snake.x = snake.x + x;
	snake.y = snake.y + y;
	draw_snake(sht,1); //打印移动后的蛇
	return food;
}

int judgefunc(struct SHEET* sht,int x,int y)
{
	//若蛇头即将到达的位置是食物，则得分
	if (face[snake.y+y ][snake.x +x] == food)
	{
		snake.len++; //蛇身加长
		grade += 10; //更新当前得分
		int len=snake.len;
		int relx=body[len-2].x-body[len-3].x,rely=body[len-2].y-body[len-3].y;
		body[len-1].x=relx+body[len-2].x;body[len-1].y=rely+body[len-2].y;
		char s[20];
		sprintf(s,"SCORE:%d ",grade);
		putfonts8_asc_sht(sht,sht->bxsize/2-40,0,10,7,s,9);	
		face[snake.y+y ][snake.x +x] == Body;
		boxfill8(sht->buf,sht->bxsize,backcol,(snake.y+y)*boxsize,(snake.x+x)*boxsize,(snake.y+y)*boxsize+boxsize,(snake.x+x)*boxsize+boxsize);
		RandFood(sht); //重新随机生成食物
		return food;
	}
	//若蛇头即将到达的位置是墙或者蛇身，则游戏结束
	else if (face[snake.y+y ][snake.x+x ] == wall || face[snake.y +y][snake.x +x] == Body)
	{
		draw_fail(sht);
		return wall;
	}
	return Body;
}

void draw_fail(struct SHEET *sht)//绘制失败界面
{
	boxfill8(sht->buf,sht->bxsize,0,boxsize,boxsize,sht->bxsize-boxsize,sht->bysize-boxsize);
	char s[20]={"FAIL!!"};
	putfonts8_asc_sht(sht,sht->bxsize/2-100,sht->bysize/2-60,1,Grey,s,7);	
	timer_init(timer4,&fifo,26);
	sprintf(s,"Your Score:%d",grade);
	putfonts8_asc_sht(sht,sht->bxsize/2-100,sht->bysize/2-60+25,1,Grey,s,strlen(s)+1);	
	char s2[30]={"Press Enter To Continue"};
	putfonts8_asc_sht(sht,sht->bxsize/2-100,sht->bysize/2-60+50,1,Grey,s2,strlen(s2)+1);	
	grade=0;
	sprintf(s,"<<Back To Menu");
	//putfonts8_asc_sht(sht,100,100,Yellow,DarkGrey,s,strlen(s));
	draw_button(sht,100,100,s,Yellow,DarkGrey);
	sprintf(s,"New Game>>");
	//putfonts8_asc_sht(sht,sht->bxsize-200,sht->bysize-100,Green,DarkGrey,s,strlen(s));
	draw_button(sht,sht->bxsize-200,sht->bysize-100,s,Green,DarkGrey);
	//draw_arrow(sht,sht->bxsize-200,sht->bysize-100,sht->bxsize-200+80,sht->bysize-100+80,Green,DarkGrey);
}
