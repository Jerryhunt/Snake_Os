/* bootpack�̃��C�� */

#include "bootpack.h"
#include <stdio.h>
#include <string.h>

void make_game8(struct SHEET *sht, int xsize, int ysize, char *title);
//void putfonts8_asc_sht(struct SHEET *sht, int x, int y, int c, int b, char *s, int l);
void make_textbox8(struct SHEET *sht, int x0, int y0, int sx, int sy, int c);
void init_kaijitimer(struct TIMER* timer[],int n,int gap);
void ani_err(struct SHEET *sht);
void make_pause(struct SHEET *sht);
//void draw_arrow(struct SHEET *sht,int x0,int y0,int x1,int y1,int c,int b);
struct FIFO32 fifo;
struct TIMER *timer4;

enum Phase{
	kaiji=1,acount,desktop,game, ingame,gameover,pause
};
int mx,my,count=0,grade=0;
int difficulty=mid;
int dif[3]={20,10,5};

void HariMain(void)
{
	struct BOOTINFO *binfo = (struct BOOTINFO *) ADR_BOOTINFO;
	enum Phase phase=kaiji;
	char s[40],tmp[10];
	int fifobuf[128];
	int used=0;
	struct TIMER  *timer2, *timer3,*timer_kaiji[20],*timer5,*timer6,*timer7;//timer4 for snake moving;timer5 for clock;timer6 for double click
	int  i, cursor_x=8, cursor_c,lastime=0;
	//int click=0;//for double click
	int hour=0,min=0,sec=0,abt=3600*23+60*59+50;//for clock
	int drt=right;
	unsigned int memtotal;
	struct MOUSE_DEC mdec;
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
	struct SHTCTL *shtctl;
	struct SHEET *sht_back, *sht_mouse, *sht_win, *sht_kaiji, *sht_acount,*sht_game,*sht_win2,*sht_pause;
	unsigned char *buf_back, buf_mouse[256], *buf_win,*buf_kaiji,*buf_acount,*buf_game,*buf_win2,*buf_pause;
	static char keytable[0x54] = {
		0,   0,   '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '^', 0,   0,
		'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '@', '[', 0,   0,   'A', 'S',
		'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', ':', 0,   0,   ']', 'Z', 'X', 'C', 'V',
		'B', 'N', 'M', ',', '.', '/', 0,   '*', 0,   ' ', 0,   0,   0,   0,   0,   0,
		0,   0,   0,   0,   0,   0,   0,   '7', '8', '9', '-', '4', '5', '6', '+', '1',
		'2', '3', '0', '.'
	};
	char password[30]={"JERRY"};
	char pas_buffer[30]={""};

	init_gdtidt();
	init_pic();
	io_sti(); /* IDT/PIC�̏��������I������̂�CPU�̊��荞�݋֎~������ */
	fifo32_init(&fifo, 128, fifobuf);
	init_pit();
	init_keyboard(&fifo, 256);
	enable_mouse(&fifo, 512, &mdec);
	io_out8(PIC0_IMR, 0xf8); /* PIT��PIC1�ƃL�[�{�[�h������(11111000) */
	io_out8(PIC1_IMR, 0xef); /* �}�E�X������(11101111) */

	timer2 = timer_alloc();
	timer_init(timer2, &fifo, 5);
	timer_settime(timer2, 500);//time for kaiji
	timer3 = timer_alloc();
	timer_init(timer3, &fifo, 1);
	timer_settime(timer3, 50);
	init_kaijitimer(timer_kaiji,20,25);
	timer4 = timer_alloc();
	timer_init(timer4, &fifo, 25);
	timer5 = timer_alloc();
	timer_init(timer5, &fifo, 30);
	timer6 = timer_alloc();
	timer_init(timer6, &fifo, 31);
	timer7 = timer_alloc();
	timer_init(timer7, &fifo, 32);
	//timer_settime(timer4, 70);
	

	memtotal = memtest(0x00400000, 0xbfffffff);
	memman_init(memman);
	memman_free(memman, 0x00001000, 0x0009e000); /* 0x00001000 - 0x0009efff */
	memman_free(memman, 0x00400000, memtotal - 0x00400000);

	init_palette();
	shtctl = shtctl_init(memman, binfo->vram, binfo->scrnx, binfo->scrny);
	sht_back  = sheet_alloc(shtctl);
	sht_mouse = sheet_alloc(shtctl);
	sht_win   = sheet_alloc(shtctl);
	sht_kaiji =sheet_alloc(shtctl);
	sht_acount =sheet_alloc(shtctl);
	sht_game=sheet_alloc(shtctl);
	sht_win2=sheet_alloc(shtctl);
	sht_pause=sheet_alloc(shtctl);
	buf_back  = (unsigned char *) memman_alloc_4k(memman, binfo->scrnx * binfo->scrny);
	buf_win   = (unsigned char *) memman_alloc_4k(memman, 160 * 52);
	buf_kaiji   = (unsigned char *) memman_alloc_4k(memman, 400*400);
	buf_acount  = (unsigned char *) memman_alloc_4k(memman, 400*400);
	buf_game  = (unsigned char *) memman_alloc_4k(memman, game_col*game_row*20*20);
	buf_win2=(unsigned char *) memman_alloc_4k(memman, 21*22);
	buf_pause=(unsigned char *) memman_alloc_4k(memman, 100*100);
	sheet_setbuf(sht_back, buf_back, binfo->scrnx, binfo->scrny, -1); /* �����F�Ȃ� */
	sheet_setbuf(sht_mouse, buf_mouse, 16, 16, 99);
	sheet_setbuf(sht_win, buf_win, 160, 52, -1); /* �����F�Ȃ� */
	sheet_setbuf(sht_kaiji, buf_kaiji, 400, 400, -1);
	sheet_setbuf(sht_acount, buf_acount,400,400, -1);
	sheet_setbuf(sht_game, buf_game,game_col*20,game_row*20, -1);
	sheet_setbuf(sht_win2, buf_win2, 21, 22, -1);
	sheet_setbuf(sht_pause, buf_pause, 100, 100, -1);
	//init_screen8(buf_back, binfo->scrnx, binfo->scrny);
	init_mouse_cursor8(buf_mouse, 99);
	make_game8(sht_game, 900,700, "Snake");
	make_textbox8(sht_win, 8, 28, 144, 16, COL8_FFFFFF);
	make_kaiji(sht_kaiji,400,20,0);//to-do
	//make_kaiji2(buf_back,sht_back->bxsize,400,400);
	make_acount(sht_acount,400,400);//to-do
	make_window2(buf_win2,21,24);
	make_pause(sht_pause);
	cursor_x = 8;
	cursor_c = COL8_FFFFFF;
	sheet_slide(sht_back, 0, 0);
	mx = (binfo->scrnx - 16) / 2; /* ��ʒ����ɂȂ�悤�ɍ��W�v�Z */
	my = (binfo->scrny - 28 - 16) / 2;
	sheet_slide(sht_mouse, mx, my);
	sheet_slide(sht_win, 80, 72);
	sheet_slide(sht_kaiji,312+20,184-20);
	sheet_slide(sht_acount,312,184);
	sheet_slide(sht_game,50,50);
	sheet_slide(sht_win2, 70,binfo->scrny-24);
	sheet_slide(sht_pause,420,250);
	sheet_updown(sht_back,  -1);
	sheet_updown(sht_win,   -1);
	sheet_updown(sht_mouse, -1);
	sheet_updown(sht_kaiji,1);
	sheet_updown(sht_acount,-1);
	sheet_updown(sht_game,-1);
	sheet_updown(sht_win2,   -1);
	sheet_updown(sht_pause,-1);
	//boxfill8(buf_back,binfo->scrnx,0,0,0,binfo->scrnx,binfo->scrny);
	//sprintf(s, "(%3d, %3d)", mx, my);
	//putfonts8_asc_sht(sht_back, 0, 0, COL8_FFFFFF, COL8_008484, s, 10);
	//sprintf(s, "memory %dMB   free : %dKB",
	//		memtotal / (1024 * 1024), memman_total(memman) / 1024);
	//putfonts8_asc_sht(sht_back, 0, 32, COL8_FFFFFF, COL8_008484, s, 40);
	int lastmove=0,lastx=0,lasty=0;

	for (;;) {
		count++;
		io_cli();
		if (fifo32_status(&fifo) == 0) {
			io_stihlt();
		} else {
			i = fifo32_get(&fifo);
			io_sti();
			if (256 <= i && i <= 511) { //keybord
				//sprintf(s, "%02X", i - 256);
				//putfonts8_asc_sht(sht_back, 0, 16, COL8_FFFFFF, COL8_008484, s, 2);
				switch (phase)
				{
				case kaiji:
					/* code */
					break;
				case acount:
				    //sprintf(s, "%02X", i - 256);
					//putfonts8_asc_sht(sht_back, 0, 16, COL8_FFFFFF, COL8_008484, s, 2);
					if (i < 0x54 + 256) {
						if (keytable[i - 256] != 0 && cursor_x < 8*26) { /* �ʏ핶�� */
							/* �ꕶ���\�����Ă���A�J�[�\����1�i�߂� */
							s[0] = keytable[i - 256];
							s[1] = 0;
							putfonts8_asc_sht(sht_acount, cursor_x+30, 310, COL8_000000, COL8_FFFFFF, s, 1);
							pas_buffer[cursor_x/8-1]=s[0];
							//timer_init(timer7,&fifo,32);
							if(!used){
								timer_settime(timer7,20);
								used=1;
							}
							cursor_x += 8;
							
						}
					}
					if (i == 256 + 0x0e && cursor_x > 8) { //退格键
						/* �J�[�\�����X�y�[�X�ŏ����Ă���A�J�[�\����1�߂� */
						putfonts8_asc_sht(sht_acount, cursor_x+30, 310, COL8_000000, COL8_FFFFFF, "", 1);
						pas_buffer[cursor_x/8-2]=0;
						cursor_x -= 8;
					}
					if(i==256+0x1C ){
						int pass=strcmp(pas_buffer,password);
						if(pass==0){//进入桌面
							//ani_pas(sht_acount);
							phase=desktop;
							init_screen8(buf_back, binfo->scrnx, binfo->scrny);
							make_windows(sht_back->buf,sht_back->bxsize,430,300);
							putfonts8_asc_sht(sht_back,440,300+50,Yellow,DarkBlue,"Jerry_OS",9);
							make_snake(sht_back,10,10);
							//make_windows(sht_back->buf,sht_back->bxsize,10,10);
							sheet_updown(sht_acount,-1);
							sheet_updown(sht_back,1);
							sheet_updown(sht_mouse,2);
							timer_settime(timer5,100);
							used=0;
							//sprintf(s,"(%d)",desktop);
							//putfonts8_asc_sht(sht_back,50,600,0,7,s,10);
						}
						else {
							ani_err(sht_acount);
							//char tmp[5]={""};
							int leth=strlen(pas_buffer);
							int t=0;
							for(;t<leth;t++){
								pas_buffer[t]='\0';
							}
							cursor_x=8;
							used=0;
							sheet_refresh(sht_acount,0,200,400,400);
						}
						//sheet_refresh(sht_acount,0,0,400,400);
					}
					/* �J�[�\���̍ĕ\�� */
					//boxfill8(sht_acount->buf, sht_acount->bxsize, cursor_c, cursor_x, 310, cursor_x + 7, 310+15);
					//putfonts8_asc_sht(sht_acount,30,380,2,7,pas_buffer,40);
					sheet_refresh(sht_acount, cursor_x+30, 310, cursor_x + 38, 310+20);
					sheet_refresh(sht_acount,0,0,400,400);
					break;
				case desktop:

					break;
				case game:
					if(i-256==0x1c){//开始游戏
						phase=ingame;
						drt=right;
						initgameface(sht_game);	
						timer_init(timer4,&fifo,25);
						timer_settime(timer4,70);
						RandFood(sht_game);
						sheet_refresh(sht_game,0,0,sht_game->bxsize,sht_game->bysize);
					}
					break;
				case ingame:
					switch (i-256)
					{
					case Up:
						if(drt==right||drt==left){
							drt=up;
						}
						break;
					case Down:
						if(drt==right||drt==left){
							drt=down;
						}
						break;
					case Right:
						if(drt==up||drt==down){
							drt=right;
						}
						break;
					case Left:
						if(drt==up||drt==down){
							drt=left;
						}
						break;
					case 0x01:
						phase=game;
						make_game8(sht_game,sht_game->bxsize,sht_game->bysize,"Snake");
						sheet_refresh(sht_game,0,0,sht_game->bxsize,sht_game->bysize);
						break;
					case 0x39:  //pause
						phase=pause;
						timer_init(timer4,&fifo,27);
						sheet_updown(sht_pause,3);
						sheet_updown(sht_mouse,4);
						break;
					default:
						break;
					}
					break;
				case pause:
					if(i-256==0x39){
						phase=ingame;
						timer_init(timer4,&fifo,25);
						timer_settime(timer4,20);
						sheet_updown(sht_pause,-1);
						sheet_updown(sht_mouse,3);
						//sheet_refresh(sht_game,0,0,sht_game->bxsize,sht_game->bysize);
						break;
					}
							
				case gameover:
					if(i-256==0x1C){//继续游戏
						phase=ingame;
						drt=right;
						initgameface(sht_game);
						timer_init(timer4,&fifo,25);
						timer_settime(timer4,70);
						RandFood(sht_game);
						sheet_refresh(sht_game,0,0,sht_game->bxsize,sht_game->bysize);
					}
					else if(i-256==0x01){//返回主菜单
						phase=game;
						make_game8(sht_game,sht_game->bxsize,sht_game->bysize,"Snake");
						sheet_refresh(sht_game,0,0,sht_game->bxsize,sht_game->bysize);
					}
					break;
				default:
					break;
				}
			} else if (512 <= i && i <= 767) { //mouse
				if (mouse_decode(&mdec, i - 512) == 0){
					continue;
				}
				int relx=mx-sht_game->vx0;
				int rely=my-sht_game->vy0;
				int xsize=sht_game->bxsize;
				if ((mdec.btn & 0x01) != 0) {
						s[1] = 'L';
				}
				if ((mdec.btn & 0x02) != 0) {
					s[3] = 'R';
				}
				if ((mdec.btn & 0x04) != 0) {
					s[2] = 'C';
				}
				//putfonts8_asc_sht(sht_back, 32, 16, COL8_FFFFFF, COL8_008484, s, 15);
				/* �}�E�X�J�[�\���̈ړ� */
				mx += mdec.x;
				my += mdec.y;
				if (mx < 0) {
					mx = 0;
				}
				if (my < 0) {
					my = 0;
				}
				if (mx > binfo->scrnx - 1) {
					mx = binfo->scrnx - 1;
				}
				if (my > binfo->scrny - 1) {
					my = binfo->scrny - 1;
				}
				switch (phase)
				{
				case kaiji:
					/* code */
					break;
				case acount:
					sheet_slide(sht_mouse, mx, my);
					break;
				case desktop:
					if(s[1]=='L'&&10<=mx&&mx<=100+175&&10<=my&&my<=100+92){//打开游戏
						
						if(count-lastime<=50){
							//char tmp[20];
							//sprintf(tmp,"%d - %d",count,lastime);
							//putfonts8_asc_sht(sht_game,300,200,Blue,Black,tmp,15);
							phase=game;
							sheet_updown(sht_mouse,-1);
							sheet_updown(sht_game,2);
							sheet_updown(sht_mouse,3);
							sheet_updown(sht_win2,2);
						}
						else {
							lastime=count;
						}
					}
					sheet_slide(sht_mouse, mx, my);
					break;
				case game:
					
					if(100<=relx && relx<=100+154 && 100<=rely&&rely<=100+24&&s[1]=='L'){//点击以开始游戏
						phase=ingame;
						drt=right;
						initgameface(sht_game);	
						timer_init(timer4,&fifo,25);
						timer_settime(timer4,70);
						RandFood(sht_game);
						sheet_refresh(sht_game,0,0,sht_game->bxsize,sht_game->bysize);
					}
					if(xsize-42<=relx&&relx<=xsize-42+16&&5<=rely&&rely<=5+14&&s[1]=='L'){//hide
						sheet_updown(sht_game,-1);
						phase=desktop;
					}	
					if(xsize-21<=relx&&relx<=xsize-21+16&&5<=rely&&rely<=5+14&&s[1]=='L'){//close
						sheet_updown(sht_game,-1);
						sheet_updown(sht_win2,-1);
						phase=desktop;
					}	
					if(260<=relx&&relx<=260+42&&240<=rely&&rely<=260&&s[1]=='L'){
						if(difficulty>1){
							difficulty--;
						}
						refresh_button(sht_game);
						sheet_refresh(sht_game,320,200,400,240);
					}
					else if(360<=relx&&relx<=360+28&&240<=rely&&rely<=260&&s[1]=='L'){
						if(difficulty<3){
							difficulty++;
						}
						refresh_button(sht_game);
						sheet_refresh(sht_game,320,200,400,240);
					}
					if(s[1]=='L'&&0<=relx&&relx<=xsize&&rely>=0&&rely<=18){//move
						if(lastmove==1){
							sheet_slide(sht_game,sht_game->vx0+mx-lastx,sht_game->vy0+my-lasty);
							lastx=mx,lasty=my;
						}
						else {
							lastx=mx;lasty=my;
						}
						lastmove=1;
					}
					else lastmove=0;
					
					sheet_slide(sht_mouse, mx, my);
					break;
				case ingame:
					
					
					sheet_slide(sht_mouse, mx, my);
					break;
				case gameover:
					relx=mx-sht_game->vx0,rely=my-sht_game->vy0;
					if(100<=relx&&relx<=100+14*8&&  100<=rely&&rely<=100+20  &&s[1]=='L'){//back to game
						phase=game;
						make_game8(sht_game,sht_game->bxsize,sht_game->bysize,"Snake");
						sheet_refresh(sht_game,0,0,sht_game->bxsize,sht_game->bysize);
					}
					if(sht_game->bxsize-200<=relx&&relx<=sht_game->bxsize-200+8*11&&  sht_game->bysize-100<=rely&&rely<=sht_game->bysize-100+20  &&s[1]=='L'){//continue game
						phase=ingame;
						drt=right;
						initgameface(sht_game);
						timer_init(timer4,&fifo,25);
						timer_settime(timer4,70);
						RandFood(sht_game);
						sheet_refresh(sht_game,0,0,sht_game->bxsize,sht_game->bysize);
					}
					sheet_slide(sht_mouse, mx, my);
					break;
				case pause:
				 	sheet_slide(sht_mouse, mx, my);
					 break;
				default:
					break;
				}
				
			}  else if (i == 5) { //finish kaiji
				//putfonts8_asc_sht(sht_back, 0, 80, COL8_FFFFFF, COL8_008484, "5[sec]", 6);
				//sheet_updown(sht_back,  0);
				boxfill8(buf_back,binfo->scrnx,DarkBlue,0,0,binfo->scrnx,binfo->scrny);
				sheet_updown(sht_kaiji,-1);
				sheet_updown(sht_back,1);	
				sheet_updown(sht_acount,   2);			
				sheet_updown(sht_mouse, 3);
				//init_screen8(buf_back, binfo->scrnx, binfo->scrny);
				sheet_refresh(buf_back,0,0,binfo->scrnx,binfo->scrny);
				timer_settime(timer3, 50);
				phase=acount;
			} else if (i <= 1) { //光标闪烁
				switch (phase)
				{
				case kaiji:
					
					break;
				case acount:
					if (i != 0) {
					timer_init(timer3, &fifo, 0); /* ����0�� */
					cursor_c = COL8_000000;
					} else {
					timer_init(timer3, &fifo, 1); /* ����1�� */
					cursor_c = COL8_FFFFFF;
					}
					timer_settime(timer3, 50);
					boxfill8(sht_acount->buf, sht_acount->bxsize, cursor_c, cursor_x+30, 310, cursor_x + 37, 310+15);
					sheet_refresh(sht_acount, cursor_x+30, 310, cursor_x + 38, 310+20);
					
					break;
				case desktop:
				 
				 	break;
					
				case game:

					break;

				default:
					break;
				}
					
				
				
				
			}
			else if(50<=i&&i<=69){//kaiji donghua
				make_kaiji(sht_kaiji,400,20,0,i-50);
				//make_kaiji2(buf_kaiji,400,100,100);
				sheet_refresh(sht_kaiji, 0,0,400,400);
			}else if(i==25){//蛇移动定时器
				if(phase==ingame){
					switch (drt)
				{
				case up:
					movesnake(sht_game,0,-1);
					break;
				case down:
					movesnake(sht_game,0,1);
					break;
				case right:
					movesnake(sht_game,1,0);
					break;
				case left:
					movesnake(sht_game,-1,0);
					break;
					
				default:
					break;
				}
				timer_settime(timer4,dif[difficulty-1]);
				sheet_refresh(sht_game,0,0,sht_game->bxsize,sht_game->bysize);
				}
				
			}else if(i==26){
				phase=gameover;
			}
			else if(i==30){//桌面时钟
				abt++;
				sec=abt%60;
				min=abt/60%60;
				hour=abt/3600%24;
				sprintf(s, "%d:%d:%d", hour,min,sec);
				tmp[0]=hour/10+48;tmp[1]=hour%10+48;tmp[2]=tmp[5]=':';
				tmp[3]=min/10+48;tmp[4]=min%10+48;
				tmp[6]=sec/10+48;tmp[7]=sec%10+48;
				
				putfonts8_asc_sht(sht_back, sht_back->bxsize-80+5, sht_back->bysize-20, COL8_000000, COL8_C6C6C6, tmp, 8);
				timer_settime(timer5, 100);
				sheet_refresh(sht_back, sht_back->bxsize-80, sht_back->bysize-18, sht_back->bxsize,sht_back->bysize);
			}
			else if(i==32){
				if(used)timer_settime(timer7,30);
				else continue;
				int tmpx=38;
				while(tmpx<cursor_x+30){
					putfonts8_asc_sht(sht_acount, tmpx, 310, COL8_000000, COL8_FFFFFF, "*", 1);
					tmpx+=8;
				}
				
				//putfonts8_asc_sht(sht_acount, cursor_x-8+30, 310, COL8_000000, COL8_FFFFFF, "*", 1);
			}
			else if(i==27){//time pause
				;
			}
			sprintf(s,"(%d)",count);
			//putfonts8_asc_sht(sht_back,50,700,0,7,s,10);
			//(sht_back,50,700,100,720);
		}
	}
}

void make_game8(struct SHEET *sht, int xsize, int ysize, char *title)
{
	char* buf=sht->buf;
	static char closebtn[14][16] = {
		"OOOOOOOOOOOOOOO@",
		"OQQQQQQQQQQQQQ$@",
		"OQQQQQQQQQQQQQ$@",
		"OQQQ@@QQQQ@@QQ$@",
		"OQQQQ@@QQ@@QQQ$@",
		"OQQQQQ@@@@QQQQ$@",
		"OQQQQQQ@@QQQQQ$@",
		"OQQQQQ@@@@QQQQ$@",
		"OQQQQ@@QQ@@QQQ$@",
		"OQQQ@@QQQQ@@QQ$@",
		"OQQQQQQQQQQQQQ$@",
		"OQQQQQQQQQQQQQ$@",
		"O$$$$$$$$$$$$$$@",
		"@@@@@@@@@@@@@@@@"
	};
	static char hidebtn[14][16]={
		"OOOOOOOOOOOOOOO@",
		"OQQQQQQQQQQQQQ$@",
		"OQQQQQQQQQQQQQ$@",
		"OQQQQQQQQQQQQQ$@",
		"OQQQQQQQQQQQQQ$@",
		"OQ@@@@@@@@@@@Q$@",
		"OQ@@@@@@@@@@@Q$@",
		"OQ@@@@@@@@@@@Q$@",
		"OQQQQQQQQQQQQQ$@",
		"OQQQQQQQQQQQQQ$@",
		"OQQQQQQQQQQQQQ$@",
		"OQQQQQQQQQQQQQ$@",
		"O$$$$$$$$$$$$$$@",
		"@@@@@@@@@@@@@@@@"
	};
	static char expbtn[14][16] = {
		"OOOOOOOOOOOOOOO@",
		"OQQQQQQQQQQQQQ$@",
		"OQQQQQQQQQQQQQ$@",
		"OQQQ@@@@@@@@QQ$@",
		"OQQQ@QQQQQQ@QQ$@",
		"OQQQ@QQQQQQ@QQ$@",
		"OQQQ@QQQQQQ@QQ$@",
		"OQQQ@QQQQQQ@QQ$@",
		"OQQQ@QQQQQQ@QQ$@",
		"OQQQ@@@@@@@@QQ$@",
		"OQQQQQQQQQQQQQ$@",
		"OQQQQQQQQQQQQQ$@",
		"O$$$$$$$$$$$$$$@",
		"@@@@@@@@@@@@@@@@"
	};
	int x, y;
	char c;
	boxfill8(buf, xsize, COL8_C6C6C6, 0,         0,         xsize - 1, 0        );
	boxfill8(buf, xsize, COL8_FFFFFF, 1,         1,         xsize - 2, 1        );
	boxfill8(buf, xsize, COL8_C6C6C6, 0,         0,         0,         ysize - 1);
	boxfill8(buf, xsize, COL8_FFFFFF, 1,         1,         1,         ysize - 2);
	boxfill8(buf, xsize, COL8_848484, xsize - 2, 1,         xsize - 2, ysize - 2);
	boxfill8(buf, xsize, COL8_000000, xsize - 1, 0,         xsize - 1, ysize - 1);
	boxfill8(buf, xsize, COL8_C6C6C6, 2,         2,         xsize - 3, ysize - 3);
	boxfill8(buf, xsize, COL8_000084, 3,         3,         xsize - 4, 20       );
	boxfill8(buf, xsize, COL8_848484, 1,         ysize - 2, xsize - 2, ysize - 2);
	boxfill8(buf, xsize, COL8_000000, 0,         ysize - 1, xsize - 1, ysize - 1);
	//close
	for (y = 0; y < 14; y++) {
		for (x = 0; x < 16; x++) {
			c = closebtn[y][x];
			if (c == '@') {
				c = COL8_000000;
			} else if (c == '$') {
				c = COL8_848484;
			} else if (c == 'Q') {
				c = COL8_C6C6C6;
			} else {
				c = COL8_FFFFFF;
			}
			buf[(5 + y) * xsize + (xsize - 21 + x)] = c;
		}
	}
	//hide
	for (y = 0; y < 14; y++) {
		for (x = 0; x < 16; x++) {
			c = hidebtn[y][x];
			if (c == '@') {
				c = COL8_000000;
			} else if (c == '$') {
				c = COL8_848484;
			} else if (c == 'Q') {
				c = COL8_C6C6C6;
			} else {
				c = COL8_FFFFFF;
			}
			buf[(5 + y) * xsize + (xsize - 21-21 + x)] = c;
		}
	}
	//expend
	/*for (y = 0; y < 14; y++) {
		for (x = 0; x < 16; x++) {
			c = expbtn[y][x];
			if (c == '@') {
				c = COL8_000000;
			} else if (c == '$') {
				c = COL8_848484;
			} else if (c == 'Q') {
				c = COL8_C6C6C6;
			} else {
				c = COL8_FFFFFF;
			}
			buf[(5 + y) * xsize + (xsize - 21-21 + x)] = c;
		}
	}*/
	//draw_startbutton(sht,100,100);
	/*char s[20];
	sprintf(s,"START");
	putfonts8_asc_sht(sht,100,100,9,7,s,6);*/
	char s[40];
	sprintf(s,"Current Difficulty:");
	//putfonts8_asc_sht(sht,100,200,Green,White,s,20);
	draw_button(sht,100,200,s,White,DarkYellow);
	
	switch (difficulty)
	{
	case 1:
		sprintf(s,"Easy");
		//putfonts8_asc_sht(sht,154+10,200,Green,White,s,5);
		draw_button(sht,320,200,s,White,DarkGreen);
		break;
	case 2:
		boxfill8(sht->buf,sht->bxsize,8,320,200,400,240);
		sprintf(s,"Mid");
		draw_button(sht,320,200,s,White,Blue);
		break;
	case 3:
		sprintf(s,"Hard");
		draw_button(sht,320,200,s,White,Red);
		break;
	default:
		break;
	}

	sprintf(s,"<<Down");
	draw_button(sht,260,240,s,White,Blue);
	sprintf(s,"Up>>");
	draw_button(sht,360,240,s,White,Red);
	draw_startbutton(sht,100,100);
	
	return;
}
void refresh_button(struct SHEET* sht)//只更改按钮部分
{
	char s[40];
	switch (difficulty)
	{
	case 1:
		sprintf(s,"Easy");
		//putfonts8_asc_sht(sht,154+10,200,Green,White,s,5);
		draw_button(sht,320,200,s,White,DarkGreen);
		break;
	case 2:
		boxfill8(sht->buf,sht->bxsize,8,320,200,400,240);
		sprintf(s,"Mid");
		draw_button(sht,320,200,s,White,Blue);
		break;
	case 3:
		sprintf(s,"Hard");
		draw_button(sht,320,200,s,White,Red);
		break;
	default:
		break;
	}

	sprintf(s,"<<Down");
	draw_button(sht,260,240,s,White,Blue);
	sprintf(s,"Up>>");
	draw_button(sht,360,240,s,White,Red);
}


void make_textbox8(struct SHEET *sht, int x0, int y0, int sx, int sy, int c)
{
	int x1 = x0 + sx, y1 = y0 + sy;
	boxfill8(sht->buf, sht->bxsize, COL8_848484, x0 - 2, y0 - 3, x1 + 1, y0 - 3);
	boxfill8(sht->buf, sht->bxsize, COL8_848484, x0 - 3, y0 - 3, x0 - 3, y1 + 1);
	boxfill8(sht->buf, sht->bxsize, COL8_FFFFFF, x0 - 3, y1 + 2, x1 + 1, y1 + 2);
	boxfill8(sht->buf, sht->bxsize, COL8_FFFFFF, x1 + 2, y0 - 3, x1 + 2, y1 + 2);
	boxfill8(sht->buf, sht->bxsize, COL8_000000, x0 - 1, y0 - 2, x1 + 0, y0 - 2);
	boxfill8(sht->buf, sht->bxsize, COL8_000000, x0 - 2, y0 - 2, x0 - 2, y1 + 0);
	boxfill8(sht->buf, sht->bxsize, COL8_C6C6C6, x0 - 2, y1 + 1, x1 + 0, y1 + 1);
	boxfill8(sht->buf, sht->bxsize, COL8_C6C6C6, x1 + 1, y0 - 2, x1 + 1, y1 + 1);
	boxfill8(sht->buf, sht->bxsize, c,           x0 - 1, y0 - 1, x1 + 0, y1 + 0);
	return;
}

void make_kaiji(struct SHEET *sht,int xsize, int x,int y,int time)//(X,Y) equ the start coordinate of icon
{	
	int leth=110;
	int gap=15;
	char* buf=sht->buf;
	boxfill8(buf,xsize,0,0,0,400,400);
	//draw_cycle(buf,xsize,400,xsize/2,100,100,3);
	boxfill8(buf,xsize,Red,x,y,x+leth,y+leth);
	boxfill8(buf,xsize,Green,x+leth+gap,y,x+leth+leth+gap,y+leth);
	boxfill8(buf,xsize,Blue,x,y+leth+gap,x+leth,y+leth+leth+gap);
	boxfill8(buf,xsize,Yellow,x+leth+gap,y+leth+gap,x+leth+leth+gap,y+leth+leth+gap);
	boxfill8(buf,xsize,White,x-20,y+2*leth+gap+20,x+2*leth+gap+20,y+2*leth+gap+20+30);
	boxfill8(buf,xsize,Black,x-20+2,y+2*leth+gap+20+2,x+2*leth+gap+20-2,y+2*leth+gap+20+30-2);
	
	//dymic
	int wide=(2*leth+gap+40)/10;
	int st1=time%10;
	boxfill8(buf,xsize,6,st1*wide,y+2*leth+gap+20+5,st1*wide+wide-5,y+2*leth+gap+20+30-5);
	int st2=(time+1)%10;
	boxfill8(buf,xsize,6,st2*wide,y+2*leth+gap+20+5,st2*wide+wide-5,y+2*leth+gap+20+30-5);
	int st3=(time+2)%10;
	boxfill8(buf,xsize,6,st3*wide,y+2*leth+gap+20+5,st3*wide+wide-5,y+2*leth+gap+20+30-5);
	char s[40]={"Starting Up... "};
	putfonts8_asc_sht(sht,x-20+20,y+2*leth+gap+80,White,Black,s,strlen(s));
}
void make_acount(struct SHEET *sht)
{
	int xsize=sht->bxsize,ysize=sht->bysize;
	boxfill8(sht->buf,xsize,DarkBlue,0,0,xsize,ysize);
	int r=120,high=20;
	draw_cycle(sht->buf,xsize,ysize,xsize/2,r+2,r,White);//touxiang
	draw_cycle(sht->buf,xsize,ysize,xsize/2,r*3/4,r/4,Grey);
	draw_cycle(sht->buf,xsize,ysize,xsize/2,r*3/4,r/4-10,White);
	int i,j;
	int tmpx=xsize/2,tmpy=3*r/2,tmpr=r/2-3;
	for(i=0;i<xsize;i++){
		for(j=r;j<r*2;j++){
			if(j>tmpy)continue;
			int l1=i-tmpx,l2=j-tmpy;
			if(l1*l1+l2*l2<=tmpr*tmpr){
				sht->buf[j*xsize+i]=Grey;
			}
		}
	}
	tmpr-=10;
	for(i=0;i<xsize;i++){
		for(j=r;j<r*2;j++){
			if(j>tmpy)continue;
			int l1=i-tmpx,l2=j-tmpy;
			if(l1*l1+l2*l2<=tmpr*tmpr){
				sht->buf[j*xsize+i]=White;
			}
		}
	}
	//jerry
	//boxfill8(sht->buf,xsize,3,xsize/2-r,2*r+20,xsize/2+r,2*r+60);
	putfonts8_asc_sht(sht,xsize/2-28,2*r+20,Blue,White," Jerry ",7);
	//boxfill8(sht->buf,xsize,4,0,2*r+90,xsize,2*r+90+high);
	make_textbox8(sht,30,2*r+70,xsize-30-30,high,7);
	draw_arrow(sht,xsize-70,2*r+70+3,xsize-30-2,r*2+73+high-5,3,8);
}
void init_kaijitimer(struct TIMER* timer[],int n,int gap)
{
	int i=0;
	for(;i<n;i++){
		timer[i] = timer_alloc();
		timer_init(timer[i], &fifo, i+50);
		timer_settime(timer[i], gap*i);
	}

}
void make_windows(unsigned char* buf,int xsize, int x,int y)
{
	//boxfill8(buf,400,0,0,0,400,400);
	int row=38,col=87;
	static char table[40][90]={
"..............................***,*,]]]`******.........................................",
"........................**]]/@@@@@@@@@@@@@@@@..].`*....................................",
"......................,,@@@@@OOOOOOO@@@@@@@@@@@@@@./*..................................",
"......................,@@OOOOOOOOOO@@@@@@@@@@@@@@@@@@/*................................",
".....................*O@@OOOOOOOOOO@@@@@@@@@@@@@@@@@@oo./,*............................",
"....................*=@@@OOOOOOOOO@@@@@@@@@@@@@@@@@@OOO@@@@]/.***...............**.o,@@",
"...................*=@@OOOOOOOOOOO@@@@@@@@@@@@@@@@@OOO@@@@@@@@@@Ooo*,]`*,]]]]]o]O@@@@@@",
"...................*O@@OOOOOOOOOO@@@@@@@@@@@@@@@@@@OO@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@O",
"..................*^@@OOOOOOOOOO@@@@@@@@@@@@@@@@@@OOO@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@O",
".................*=.@OOOOOOOOOOO@@@@@@@@@@@@@@@@@OOO@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@Oo",
".................*O@@OOOOOOOOOO@@@@@@@@@@@@@@@@@@OOO@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@O/",
"...............**o@@OOOOOOOOOO@@@@@@@@@@@@@@@@@@OOO@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@Oo*",
"...............*.O@@@OOOOOOOO@@@@@@@@@@@@@@@@@@OOOO@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@OO^*",
"..............,=o@@OOOOOOOOOO@@@@@@@@@@@@@@@@@@OOO@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@Oo**",
"..............*.@@@OO@@@@@@@@@@@@@@@@@@@@@@@@@OOOO@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@OO^*.",
".............**.@@@@@@@OO@@OOOOOOOOOOOOO@@@@@OOOO@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@Oo**.",
".............*,.OOOO@@@@@@@@@@@@@@@@OOOOOOO@OoooO@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@Oo^*..",
"............**/O@@@@@@@@@@@@@@@@@@@@@@@@OOo..^/o@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@OOo**..",
"...........**.@@@@@@@@@@@@@@@@@@@@@@@@@@@@Oo,^/o@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@O/***..",
"...........*o@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@Oo/O@/OO@@@@@@@@@@@@@@@@@@@@@@@@@@@@Oo^**...",
".........**=/@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@OOO@@@@OOOOOO@@@@@@@@@@@@@@@@@@OOOOOo**.....",
"........**^o@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@OOOO@@@@@@@OOOOOOOOOOOOOOOOOOOOOO@@@o^**.....",
"........**/o@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@OOO@@@@@OOO@@@@@@@@@@@@@@@@@@@@@@O@@o^***....",
"........**o@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@OOO@@@@@@OOOOOOOOOooooooooooooOOOO@@Oo^***....",
".......**oO@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@OOO@@@@@@OOOOOOOOOOooooooooooOOOOO@@Oo^**.....",
"......**=o@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@OOO@@@@@@@OOOOOOOOOOooooooooooOOOO@@OOo***.....",
"**...***oO@@@@@@@@@@@@@@@@@@@@@@@@@@@@@OOOO@@@@@@@OOOOOOOOOOooooooooooOOOO@OOo`**.*****",
"***.***oo@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@OOO@@@@@@@@OOOOOOOOOOooooooooooOOO@@OOo*********",
"**..**=/O@@@@@@@@@@@@@@@@@@@@@@@@@@@@@OOOO@@@@@@@@OOOOOOOOOOoooooooooooO@@OOo`*********",
"******^O@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@OOO@@@@@@@@@OOOOOOOOOOoooooooooooO@@Ooo**********",
"******..@@@@@@@OOOOOOOOOOOOOOO@@@@@@@OOOO@@@@@@@@@OOOOOOOOOOooooooooooO@@OOo`**********",
"******,@@OOOOooooo*********oooOoOOO@OOOO@@@@@@@@@@OOOOOOOOOOooooooooooO@@Oo^***********",
"*********************************oooooOO@@@@@@@@@@OOOOOOOOOOooooooooOO@@OOo************",
"**************************************.O@@@@@@@@@@OOOOOOOOOOoooooooo/O@OOoo************",
"**************************************,oOO@@@@@@@@OOOOOOOOOOoo[[oO@@@@@Oo.*************",
"***************************************,oo.OOO@@@@@@@@@@@@@@@@@@@OOOOOoo`**************",
".*****************************************.oooOOOOOOOOOOOOOOOOOOOOoooo^****************",
"..********************************************[.oooooooooooooo/[***********************"
	};
	int i,j;
	for(i=0;i<row;i++){
		for(j=0;j<col;j++){
			if(table[i][j]=='@'){
				buf[(y+i)*xsize+j+x]=Blue;
			}else if(table[i][j]=='O'){
				buf[(y+i)*xsize+j+x]=Red;
			}else if(table[i][j]=='o'){
				buf[(y+i)*xsize+j+x]=Green;
			}else if(table[i][j]=='.'){
				buf[(y+i)*xsize+j+x]=DarkBlue;
			}else if(table[i][j]=='*'){
				buf[(y+i)*xsize+j+x]=DarkBlue;
			}else {
				buf[(y+i)*xsize+j+x]=DarkBlue;
			}
		}
	}

}

void make_window2(unsigned char *vram,int x,int y)
{
	boxfill8(vram, x, COL8_FFFFFF,  1,     y - 24, 19,     y - 24);
	boxfill8(vram, x, COL8_FFFFFF,  0,     y - 24,  0,     y -  4);
	boxfill8(vram, x, COL8_848484,  1,     y -  4, 19,     y -  4);
	boxfill8(vram, x, COL8_848484, 19,     y - 23, 19,     y -  5);
	boxfill8(vram, x, COL8_000000,  0,     y -  3, 19,     y -  3);
	boxfill8(vram, x, COL8_000000, 20,     y - 24, 20,     y -  3);
	return ;
}

void make_snake(struct SHEET *sht,int x,int y)
{
	int row=92,col=175;
	static char table[92][175]={
"		                                .]]]]O@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@]]].                        " ,    
"                         ,]@@OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO@@@@@@@@@@@@@@@@@O@@@@@@@@@@@                       ",
"                     ,/@OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO@OOOOOOOOO@OOO@@@@@@@@@@@@@@@@@@@@@OOOOO@@@`                    ",
"                  /@OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOooooOO@OOOOOOOOOOOOOOO@@@@@@@@@@@@@@@OOO@OOOOO@@@                 ",
"               ]@OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOoooooooOOOOOOOOOOOOOOOOOO@@@@@@@@@@@@@OOO@OOOOOOOO@@@]              ",
"             /OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOoooooooOOOOOOOOOOOOOOOOOO@@@@@@@@@@@@@@OOOOOOOOOOOOOO@@            ",
"           /OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOoooOOOOOOOOOOOOOOOOOOOOOOOO@@@@@@@@@@@OOOOOOOOOOOOOOO@@          ",
"         /OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO@@@@@@@@@@OOOOOOOOOOOOOOOOO@@        ",
"       ,OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO@@O@@@@@O@@@@OOOOO@OOOOOOO@@`      ",
"      /OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO@@@@@@@@@@@@@OOOOOOOOOOOOOO@     ",
"     /OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO@@@@@@@O@@@@@OOOOOOOOOOOOOO@    ",
"    /OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO@@O@@@@O@@OOOOOOOOOOOOOO@   ",
"   OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO@@@@@@@@@@@OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO@@@@@@@@@@@OOOOOOOOOOOOOOOOOOOOOOOOOO@O@@@@@@@@@@OOOOOOOOOOO@  ",
"  =OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO@@@@@@@@@@@@@@@OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO@@@@@@@@@@@@@@@@OOOOOOOOOOOOOOOOOOOOOOO@O@@@@@@@@@@@OOOOOOOOOOO@^ ",
" ,OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO@@@@@@@@@@@@@@@@@@OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO@@@@@@@@@@@@@@@@@@OOOOOOOOOOOOOOOOOOOOOOOO@OO@@@@@@OOO@OOOOOOOO@`",
" /OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO@@@@@@@@@@@@@@@@OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO@@@@@@@@@@@@@@@@@OOOOOOOOOOOOOOOOOOOOOOO@@@@@@@@@OOOOOOOOOOO@",
".OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO@@@@@@@@@@OOOOOOOOOOOOOOOOOOOOO[[`                ,[[OOOOOOOOOOOOOOOOOOOOOOO@@@@@@@@@@@OOOOOOOOOOOOOOOOOOOOOOO@@@@@@@@@@OOOOOOO@@O@",
"=OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO@@@@@@@@@OOOOOOOOOOOOOOOOO[`                              ,OOOOOOOOOOOOOOOOOOOO@@@@@@@@@OOOOOOOOOOOOOOOOOOOOOOOOOOO@@@@@OO@@@@OOO@@",
"=OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO@@@@@@@OOOOOOOOOOOOOO/`                                        [OOOOOOOOOOOOOOOOO@@@@@@@@OOOOOOOOOOOOOOOOOOOOOOOOOOOO@@@@O@O@@@OO@@@",
"OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO@@@@OOOOOOOOOOOOO/.                                              ,OOOOOOOOOOOOOOOOO@@@@OOOOOOOOOOOOOOOOOOOOOOOOOOOOO@@@@O@O@@@@@@O@",
"OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO/`                                                    ,OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO@@@@@@@@@@@",
"OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO/                                                         ,OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO@@@@@@@@@@@",
"OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO`                                                             .OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO@@@@@@@",
"OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO/                                                                 .OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO@@@@@@@",
"OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO/.                                                                    ,OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO@@@@@@",
"OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO^                                                                        OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO@@O@@@",
"OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO`                                                                          =OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO@O@@",
"OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO.                                                                            ,OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO@OOO@@",
"OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO.                      .                                ..                     ,OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO",
"OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO[                           .                          .                           OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO",
"OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO/                                ..                    ..                                OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO",
"OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO/                                    ..                ..                                    OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO",
"OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO                                        .             ..                        .]]]           ,OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO",
"OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO/              ,/@@@@@@@@@@]               ..          ..                  ,]@@@@@@@@@@           OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO",
"OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO^            ,@@@@@@@@@@@@@OO@@             ...      ...               ,@@@@@@@@@@@@@@@`            OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO",
"OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO/           ,@@@@@@@@@OOOOO`    ,O           ..     ...             ]@@@@@@@@@@@@[[.                 OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO",
"OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO           =@@@@@@OOO@@@@@        O@`           ..   ...           ,@@@@@@@@@/`                        .OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO",
"OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO^          =@@@@@OO@@@@@@@@        O@@           ...  ...         ,@@@@@@@@[                             =OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO",
"OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO^          @@@@@@O@@@@@@@@@@`    .OO@@^          ........        @@@@@@@@]]                              =OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO",
"OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO.          @@@@@OO@@@@@@@@@@@@@@OOO@@@@           ......        @@@@@@@@@@@@@@@@@]`                      ,OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO",
"OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO.          @@@@@OO@@@@@@@@@@@@@@OO@@@@O           ......        ,@@@@@@@@@@@@@@@@@@@@@]                  ,OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO",
"OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOo^          @@@@Oo/  ,@@@@@@@@@@OOO@@@@^          ........          .. ..,[[[@@@@@@@@@@@@@`              =OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO",
"OoOOOOOOOoooooooooooooooOOOOOOOOOOOoO^          ,@@@@O^   O@@@@@@@@OO@@@@@@           ........                        [@@@@@@@@@            OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO",
"OooOoooooooooooooooooooooooooOOOOOOOoO.          ,@@@@OO/OOO@@@OOOOO@@@@@/           ..........                           ,@@@@@@@         ,OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO",
"OoooooooooooooooooooOOOOooooooooooOOooO            @@@@@@@@@@O@@@@@@@@@`            ..........                              ,@@@@`          OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO",
"OooooooooooooooooooooooooooooooooooOOoO             @@@@@@@@@@@@@@@@`             ............                                            /OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO",
"OoooooooooooooooooooooooooooooooooooooooO               ,[@@@@@@@O[                ..............                                         ,OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO",
"OoooooooooooooooooooooooooooooooooooooooOO`                                      ..................                                      ,OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO",
"OoooooooooooooooooooooooooooooooooooooooOOOO`                                  ......................                                  ,OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO",
"OoooooooooooooooooooooooooooooooooooooooOOOOO^.                              ..........................                              ,OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO",
"OoooooooooooooooooooooooooooooooooooooooOOOOOo...                        ,/@          ...................                        ....OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO",
"OooooooooooooooooooooooooooooooooooooooooOOOO^........               ,/@@@@@@@@.                     ,@@@@@@]               ........OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO",
"OoooooooooooooooooooooooooooooooooooooooOOOOO/........************..@@@@@@@@@@@@@@@@O]]].      .]]/@@@@@@@@@@@@@.*************........OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO",
"OooooooooooooooooooooooooooooooooooooooooOOOO^............*.........@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@........*.............OOOOOOOOOOOOOOOOOOOOOOOOO/`        .OOOOOOOO",
"Oooooooooooooooooo[`[[oooooooooooooooooooOOOO..... .. .............@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@............. .. .....OOOOOOOOOOOOOOOOOOOOOOO`     ....     ,OOOOOO",
"Oooooooooooooooo`.......oooooooooooooooooOOOO^...  .  ..............@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@^...........  .  ......OOOOOOOOOOOOOOOOOOOOO/   ............   OOOO",
"Ooooooooooooooo^........*oooooooooooooooooOOOo......................,@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@`......................OOOOOOOOOOOOOOOOOOOOO   ..............  .OOO",
"Ooooooooooooooo^........*oooooooooooooooooOOO^.......................=@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@^.......................OOOOOOOOOOOOOOOOOOOO^  ...............   =OOO",
"Oooooooooooooooo^......*ooooooooooooooooooOOO^....................... @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@/ .......................OOOOOOOOOOOOOOOOOOOO^  ................  =OOO",
"Oooooooooooooooooooo/oooooooooooooooooooooOOO........................ @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@/ ........................OOOOOOOOOOOOOOOOOOOO^   ..............   /OOO",
"OoooooooooooooooooooooooooooooooooooooooooOOO^ ........................ ,@@@@@@@@@@@@@@@@@@@@@@@Oooo/ooO@. ................... .....OOOOOOOOOOOOOOOOOOOOO^   ............   =OOOO",
"OoooooooooooooooooooooooooooooooooooooooooOOO^      ..................... ,@@@@@@@@@@OooooooooooooOOooooooo].................       .OOOOOOOOOOOOOOOOOOOOOO    .......     /OOOOO",
"OoooooooooooooooooooooooooooooooooooooooooOOO^        ....................  ,@@@@@ooooooooooooooooooOooooooo,^..............        .OOOOOOOOOOOOOOOOOOOOOOOO`          ,/OOOOOOO",
"OoooooooooooooooooooooooooooooooooooooooooOOO^         .....................   ,[@ooooooooooooooooooooOooooooo/`.............        .OOOOOOOOOOOOOOOOOOOOOOOOOOOOO]]/OOOOOOOOOOOO",
"OoooooooooooooooooooooooooooooooooooooooooOOO^        ..........................    ,oooooooooooooooooooo//ooo[`^............        .OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO",
"OoooooooooooooooooooooooooooooooooooooooooOOO^        ...............................=oooooooooooooooooo////[[=*...........        .OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO",
"OoooooooooooooooooooooooooooooooooooooooooOOO^        ................................oooooooooooooooooo`***`,]*[*...........        .OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO",
"OoooooooooooooooooooooooooooooooooooooooooOOO^        ................................*o^o]^//************o*****...........        .OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO",
"Oooooooooooo/o=^=,=,=^oooooooooooooooooooOOO^        ................................,,`,`*[[[/,[*,,^*`,^`/*,/***...........        .OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO",
"Oooooooooo/[************oooooooooooooooooOOO^        ................................***o=^********[*,`[[`,,=****...........        .OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO",
"Oooooooo[**`.       .`*,*o/ooooooooooooooOOO^        ................................*^*,`*`*`,*]`/*[,,],*,***.............       .OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO",
"oooooooo***.   ......   ,**^/oooooooooooooOOO^        .................................=oo]o*,/***`,^=^*`[`,**..............       .OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO",
"=ooooo^*`,*  ..........  **=,oooooooooooooOOO^        ...................................oooooo]=o`*]o/[oooo`................       .OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO",
"=ooooooo**. ...........  .*=*oooooooooooooOOO^        .....................................,oooooooooooo/[...................       .OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO@",
" ooooo[**.  ..........  .*=*/ooooooooooooOOO^        ........................................................................       .OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO@",
" =ooooo*o**.  ........  .**/*oooooooooooooOOO^        ........................................................................       .OOOOOOOOOOOOOO[*****,[OOOOOOOOOOOOOOOOOOOOOO^",
"  ooooo***`         .,**,]ooooooooooooooOOO^        ........................................................................       .OOOOOOOOOOO/**********,*OOOOOOOOOOOOOOOOOOO@ ",
"  ,Oooooooo]*****..******,ooooooooooooooooOOO^        ........................................................................       .OOOOOOOOOO**************`OOOOOOOOOOOOOOOO@` ",
"   ,ooooooo[[`*]]]],/*/oooooooooooooooooooOO^        ........................................................................       .OOOOOOOOO^****************OOOOOOOOOOOOOOO@`  ",
"    ,ooooooooo]]]]]]oooooooooooooooooooooooOO^        ........................................................................       .OOOOOOOOO^****************OOOOOOOOOOOOOO@`  " ,
"     ,OoooooooooooooooooooooooooooooooooooOO^        .........................................................................      .OOOOOOOOO***************,OOOOOOOOOOOOOO`    ",
"       oooooooooooooooooooooooooooooooooooOO^        .........................................................................      .OOOOOOOOOO*************=OOOOOOOOOOOOO/     " ,
"        =ooooooooooooooooooooooooooooooooOOO^        ........................................................................       .OOOOOOOOOOOO*********,OOOOOOOOOOOOOO^      " ,
"          oooooooooooooooooooooooooooooooOOO^        ........................................................................       .OOOOOOOOOOOOOOOO]]OOOOOOOOOOOOOOOO/        " ,
"            oooooooooooooooooooooooooooooOO^        ........................................................................       .OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO/          " ,
"              OoooooooooooooooooooooooooooOO^        ........................................................................       .OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO/            " ,
"                ,oooooooooooooooooooooooooOO^       .........................................................................       .OOOOOOOOOOOOOOOOOOOOOOOOOOOOO`              " ,
"                   ,OoooooooooooooooooooooOO^       .........................................................................       .OOOOOOOOOOOOOOOOOOOOOOOOO/`                 " ,
"                       ,Ooooo/oooooooooooOOO^       ......................................................................... .     .OOOOOOOOOOOOOOOOOOOOO/`                      ",
"                            ,[OoooooooooooOO^     . .........................................................................       .OOOOOOOOOOOOOOO/[`                           "
	};
	int xsize=sht->bxsize;
	char* buf=sht->buf;
	int i,j;
	for(i=0;i<row;i++){
		for(j=0;j<col;j++){
			if(table[i][j]=='@'){
				buf[(y+i)*xsize+j+x]=DarkRed;
			}else if(table[i][j]=='O'){
				buf[(y+i)*xsize+j+x]=Blue;
			}else if(table[i][j]=='o'){
				buf[(y+i)*xsize+j+x]=Blue2;
			}else if(table[i][j]=='.'){
				buf[(y+i)*xsize+j+x]=DarkYellow;
			}else {
				buf[(y+i)*xsize+j+x]=Yellow;
			}
		}
	}

}

void ani_err(struct SHEET *sht)
{
	char s[40]={"Error! Please Enter Again!"};
	putfonts8_asc_sht(sht,30,360,Red,White,s,strlen(s));
	boxfill8(sht->buf,sht->bxsize,White,38,310,400-100,330);

}
void ani_pas(struct SHEET *sht)
{
	
}

void make_pause(struct SHEET *sht)
{
	int xsize=sht->bxsize,ysize=sht->bysize;
	boxfill8(sht->buf,xsize,Black,0,0,xsize,ysize);
	draw_cycle(sht->buf,xsize,ysize,xsize/2,ysize/2,xsize/2,Blue);
	int gap=10,wid=15;
	boxfill8(sht->buf,xsize,White,xsize/2-gap-wid,ysize/2-wid*2,xsize/2-gap,ysize/2+wid*2);
	boxfill8(sht->buf,xsize,White,xsize/2+gap,ysize/2-wid*2,xsize/2+gap+wid,ysize/2+wid*2);
}

void draw_startbutton(struct SHEET *sht,int x,int y)
{
	int row=24,col=154;
	static char table[24][154]={                                                                                                                                             
    "         .....................................................................................................................................          ",
    "     ,*******************************************************************************************************************************************`      ",
    "   /OoooOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOoooO    ",
    " .OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO.  ",
    " OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO  ",
    ",OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO@@^ ",
    "=O@OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO@@@OOOOOOO@@@OOOOOOOO@@@OOO@@@OOO@@@OOOOOOOOOOOOOOO@@@@OO@OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO@@^ ",
    "=O@@OOOOOOOOOOOOOOOOOOOOOOOOOOOOO@@@@@@@@@@@@@@@@@@@OOOO/..OOOOOOO.,OOOOOOO@/[O@@/` @OO`.O@@@OOO@@@OO@@OO`.,O/[@@OOOOOOOOOOOOOOOOOOOOOOOOOOOOOO@OO^ ",
    "=O@OOOOOOOOOOOOOOOOOOOOOOOOOOOOOO^                 =OO@@^ .O@@@OO   O@O@OOOOO`   OO^  OOO  ,[[[OO[[[[[[[[OO  .O^   OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO@@^ ",
    "=OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO^  OOOOO  =OOOOOO/.   ..,OO   OO.  OOOOO@OO^              ,OO]]]]`   OO.  OOO`OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO^ ",
    "=OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO^  OOOOO  =OOOOOO`  ,   /   O[[[   OO/ [OOO  =@@^  ,[[[[OOO` =@O  ..          OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO^ ",
    "=OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO`  [OOO/  ,OOOOOO^ .O. ,.           OO`  /O      O`.    /OO   `  /O]`  OOO.,OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO^ ",
    "=OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO                   OO/  =O  =OOOOOOOOOOOOOOOO@@OO  ,`  O@O  =@@OOOO`   =OOO^  =O  ,OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO^ ",
    "=OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO  .OOOOO  =OOOOOOO`    ,OO          OOOOO` ^  O^         .OOOO/    OOOO     ,OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO^ ",
    "=OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO.  /OOOOO  =OOOOOOOOO`   O  =OOOO^  OOOO^  /` ,O^ .OOO  =OOOO@`  ,   O@^   /O.[OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO^ ",
    "=OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO[   /OOOOOO  =OOOOOOO/   ] ,O  ,[[[`  OOO/  =^  O@` =O@O  =OOO^   /OO`//`     ,`  /OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO^ ",
    "=OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO`  ,OOOOOOOO  =OOOOOO  ,OOOOO          OOO` .]  /.   /\   /OOOOO]OOOOOO ,/O`    ,OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO^ ",
    "=OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO^ ",
    ".OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOoOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOoOOOOOOOOOOOOOO^ ",
    ".OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOooooooOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOoooooOOOOOOOOOOOO. ",
    " ,OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOoooOoooooooOOOoOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOoOoooOOOOOOOoOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOoOoooooooooOOOOOOOOOO^  ",
    "  ,OOOOOoooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo,o,=oooooooooooooooooooooooooooooooooooooooooooooooooooooooOOOOO`   ",
    "    ,OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOooOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOooOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOooOOOOOOOOOOOOOOO[     ",
    "       ,[OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO[`        ",
                                                                                                                                                            
                                                                                                                                   
	};
	int i,j,xsize=sht->bxsize;
	char* buf=sht->buf;
	for(i=0;i<row;i++){
		for(j=0;j<col;j++){
			if(table[i][j]==' '){
				buf[(y+i)*xsize+j+x]=7;
			}else if(table[i][j]=='O'){
				buf[(y+i)*xsize+j+x]=2;
			}else if(table[i][j]=='o'){
				buf[(y+i)*xsize+j+x]=3;
			}else {
				buf[(y+i)*xsize+j+x]=7;
			}
		}
	}
}
