#include "bootpack.h"

//界面
struct SNAKE snake;
struct BODY body[game_row][game_col];
int face[game_row][game_col];//记录每个小格的状态（食物，墙，蛇）

void initgameface(struct SHEET* sht)
{
    boxfill8(sht->buf,sht->bxsize,11,0,0,sht->bxsize,sht->bysize);
    boxfill8(sht->buf,sht->bxsize,0,boxsize,boxsize,sht->bxsize,sht->bysize);
    int i,j;
    for(i=0;i<game_row;i++){
        for(j=0;j<game_col;j++){
            if(i==0||i==game_row-1||j==0||j==game_col-1){
                face[i][j]=wall;
            }
            else face[i][j]=snk;
        }
    }
}
void gaming()
{


}

void game_win()
{

}
