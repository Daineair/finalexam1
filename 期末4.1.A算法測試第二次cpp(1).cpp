#include <iostream>
#include <time.h>
#include <windows.h>
#include <conio.h> 
#include <stdlib.h>

#define WIDTH 82
#define HIGHT 27
#define MonsterTime 5000//生怪時間間隔 
#define Prop_Time 10000//道具生成間隔 
#define MonsterMoveTime 200//怪物移動速度間隔 
// Hello
using namespace std;

void gotoxy(int x, int y) {
    COORD pos = {x,y};
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleCursorPosition(hOut, pos);
} 

void HideCursor(){
	CONSOLE_CURSOR_INFO cursor_info = {1,0};
	SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE),&cursor_info);
}

class Map;
class Node;
class Unit; 
class Player;
class Monster;
class FakerMonster;
class Linkedlist;
class UI;
class Lightning;//閃電招式 
class MonsterHouse;//生怪屋 
class Zipline;//滑索招式
class F;//道具F 
class S;//道具S 
class B;//道具B 
class T;//道具T 
class Prop;//生成道具的類別 
class Score;//分數類別 ( 加扣分就call這個類別 ) 

class Map{
	private:
		int wall[WIDTH][HIGHT];
	public:
		Map(){
			wall[WIDTH][HIGHT]={0};
			for(int i=0;i<WIDTH;i++){//遍歷安心 
				for(int j=0;j<HIGHT;j++){
					wall[i][j]=0;
				}
			}
			for(int i=0;i<WIDTH;i++){
				gotoxy(i,0);
				wall[i][0]=1;
				printf("*");
			}
			for(int i=0;i<HIGHT;i++){
				gotoxy(0,i);
				wall[0][i]=1;
				printf("*\n");
			}
			for(int i=0;i<HIGHT;i++){
				gotoxy(WIDTH-1,i);
				wall[WIDTH-1][i]=1;
				printf("*\n");
			}
			for(int i=0;i<WIDTH;i++){
				gotoxy(i,HIGHT);
				wall[i][HIGHT-1];
				printf("*");
			}
			
			NewWall(3,3,22,20,5,3);
			NewWall(28,3,22,20,5,3);
			NewWall(53,3,22,20,5,3);
		}
		//判斷牆壁 回傳 1代表是牆 回傳 0代表不是 
		int isWall(int x,int y){
			return wall[x][y];
		}
		//新建牆壁 
		void NewWall(int origin_x,int origin_y,int width_max,int hight_max,int width_min,int hight_min){
			int width = width_min+rand()%(width_max-width_min+1);
			int hight = hight_min+rand()%(hight_max-hight_min+1);
			origin_x+=rand()%(width_max-width+1);
			origin_y+=rand()%(hight_max-hight+1);
			for(int i=0;i<hight;i++){
				for(int j=0;j<width;j++){
					gotoxy(origin_x+j,origin_y+i);
					printf("*");
					wall[origin_x+j][origin_y+i]=1;
				}
			}
		}
};
////////////////////////// A* 算法做 Monster Move
 
int Min_Distance(int x,int y,int targetX,int targetY){//計算A算法( 8方向 )的移動成本 
	int xD = abs(x-targetX);
	int yD = abs(y-targetY);
	return xD+yD;
}

class Check{  //A 算法所注意的方格(地圖上的每一格 
	private:
		int x;
		int y;
		int cost;//距離成本 (與 player 的距離 
		int originX;//紀錄該點的源頭是哪點? 最後當找出最佳路近時，怪獸將移動到此點 (最佳路近的第一步 
		int originY;
		static bool Map_Again[WIDTH][HIGHT];
	public:
		Check *next;
	public:
		Check(){
			next = NULL;
		}
		Check(int a,int b,int targetX,int targetY,int ox,int oy){
			x = a;
			y = b;
			cost = Min_Distance(x,y,targetX,targetY);
			Map_Again[x][y]=1;
			originX = ox;
			originY = oy;
			next=NULL;
		}
		int x_Value(){ return x;
		}
		int y_Value(){ return y;
		}
		int OriginX(){
			return originX;
		}
		int OriginY(){
			return originY;
		}
		int Cost(){
			return cost;
		}
		int Again(int x,int y){
			return Map_Again[x][y];
		}
		void Die(){
			delete this;
		}
		void SetZero(){
			for(int i=0;i<WIDTH;i++){
				for(int j=0;j<HIGHT;j++){
					Map_Again[i][j]=0;
				}
			}
		}
		void SetOne(int a,int b){
			Map_Again[a][b]=1;
		}
};
bool Check::Map_Again[WIDTH][HIGHT] = {0};

class QueueList{//先進先出 ， 直線的串鏈  
	private:
		Check *head;
		Check *p;
		Map *map;
		Check *control;
	public:
		QueueList(int x,int y,Map *m){
			head=p=NULL;
			map = m;
			Check *c = new Check;
			control = c;
			c->SetZero();
			c->SetOne(x,y);
		}
		void GetNew(Check *n){
			if(head==NULL){//串鏈裡面沒東西，直接放即可 
				head = n;
			}
			else{//裡面已有東西，放入要做排序，依照移動成本大小 
				PutWhere(n);
			}
		}
		void PutWhere(Check *n){
			int cost = n->Cost();
			if(cost<head->Cost()){
				n->next = head;
				head = n;
			}
			else{
				p = head;
				while(1){
					if(p->next==NULL){
						p->next=n;
						break;
					}
					if(cost<p->next->Cost()){
						n->next = p->next;
						p->next = n;
						break;
					}
					p = p->next;
				}
			}
		}
		Check* GetHead(){//提取頭 ， Queue的先進先出特性 
			p = head;
			if(head->next==NULL){
				head = NULL;
			}
			else{
				head=head->next;
			}
			return p;
		}
		void DieALL(){//刪掉整 "條" 串鏈 
			while(head!=NULL){
				p = head;
				head = head->next;
				p->Die();
			}
			delete this;
		}
		void Add(int i,int j,int targetX,int targetY,int ox,int oy){
			if(map->isWall(i,j)==0&&control->Again(i,j)==0){
				Check *n = new Check(i,j,targetX,targetY,ox,oy);
				GetNew(n);
			}
		}
};

int MoveBy_A_Star(int x,int y,int targetX,int targetY,Map *map,char name,int *ax,int *ay){
	if(x==targetX&&y==targetY) return 0;
	QueueList *Qlink = new QueueList(x,y,map); 
	////建立 8 原點 
	Qlink->Add(x-1,y-1,targetX,targetY,x-1,y-1);
	Qlink->Add(x,y-1,targetX,targetY,x,y-1);
	Qlink->Add(x+1,y-1,targetX,targetY,x+1,y-1);
	Qlink->Add(x-1,y,targetX,targetY,x-1,y);
	Qlink->Add(x+1,y,targetX,targetY,x+1,y);
	Qlink->Add(x-1,y+1,targetX,targetY,x-1,y+1);
	Qlink->Add(x,y+1,targetX,targetY,x,y+1);
	Qlink->Add(x+1,y+1,targetX,targetY,x+1,y+1);
	
	while(1){
		Check *c = Qlink->GetHead();
		gotoxy(90,15);
		cout<<"hello";
		if(c->x_Value()==targetX&&c->y_Value()==targetY){
			gotoxy(x,y);
			printf(" ");
			gotoxy(c->OriginX(),c->OriginY());
			printf("%c",name);
			*ax = c->OriginX();
			*ay = c->OriginY();
			break;
		}
		int x= c->x_Value();
		int y= c->y_Value();
		int ox= c->OriginX();
		int oy= c->OriginY();
		
		Qlink->Add(x-1,y-1,targetX,targetY,ox,oy);
		Qlink->Add(x,y-1,targetX,targetY,ox,oy);
		Qlink->Add(x+1,y-1,targetX,targetY,ox,oy);
		Qlink->Add(x-1,y,targetX,targetY,ox,oy);
		Qlink->Add(x+1,y,targetX,targetY,ox,oy);
		Qlink->Add(x-1,y+1,targetX,targetY,ox,oy);
		Qlink->Add(x,y+1,targetX,targetY,ox,oy);
		Qlink->Add(x+1,y+1,targetX,targetY,ox,oy);
	}
	Qlink->DieALL();

} 





///////////////////////////////////////
class Node{
	protected:
		bool life;
	public:
		Node *next;
		Node(){
			life = 1;
			next = NULL;
		}
		virtual void Action()=0;
		void Die(){delete this ;} 
		int Life_Value(){return life ;}
};
class Unit{
	protected:
		int x;
		int y;
		Map *map;
		clock_t start;
		clock_t finish;
		char name;
	public:
		Unit(){
			start = clock();
			x=y=name=start=finish=0;
			map=NULL;
		}
		Unit(int xValue,int yValue,Map *m,char n){//在 xValue,yValue printf name, 且讓此單位獲得地圖(才知道障礙物) 
			start = clock();
			x = xValue;
			y = yValue;
			map = m;
			name = n;
			gotoxy(x,y);
			printf("%c",name);
		}
		int CanMove(int a,int b){//判斷 (a,b) 是否為牆壁，是->不能移動，不是->可以移動 
			if(map->isWall(a,b)) return 0;
			else return 1;
		}
		int x_Value(){return x ;}
		int y_Value(){return y ;}
};

class Linkedlist{
	private:
		Node *arrow;
		Node *head;
		Node *p;
		Node *die;
	public:
		Linkedlist(){
			arrow = NULL;
			head = NULL;
			p = NULL;
		}
		void NewNode(Node *n){
			if(head==NULL){
				head = n;
				arrow = n;
				head->next = head;
			}
			else{
				n->next = arrow->next;
				arrow->next = n;
				arrow = n;
			}
		}
		void Link_Run(){
			p = head;
			do{
				p->Action();
				if(p->next->Life_Value()==0){
					die = p->next;
					if(arrow==die) arrow = die->next;
					if(head==die) head = die->next;
					p->next=p->next->next;	
					die->Die();
				}
				p = p->next;
			}while(p!=head);
		}
};

class Zipline:public Unit,public Node{
	private:
		Player *player;
		int x;
		int y;
		char face;
	public:
		Zipline(Player *p,int a,int b,char c,Map *m){
			player = p;
			x = a;
			y = b;
			face = c;
			map = m;
			start = clock();
		}
		void Action();
		void Fly(); 
};

class Player:public Unit,public Node{
	private:
		char face;
		int skill;
		Linkedlist *link;
	public:
		Player():Unit(0,0,NULL,'P'){
			face = 'd';//初始面相朝右
			skill = 0; //skill 為技能狀態，設計施放技能時不能移動。 
		}
		Player(int xValue,int yValue,Map *m,char n,Linkedlist *l):Unit(xValue,yValue,m,n){
			face = 'd';
			skill = 0;
			link = l; 
		}
		char Input(){
			if(kbhit()){
				return getch();
			}
			return NULL;
		}
		void Move(char input){
			switch(input){
				case 'w':
				case 72://up
					if(CanMove(x,y-1)){
						gotoxy(x,y);
						printf(" ");
						gotoxy(x,--y);
						printf("%c",name);
						face = 'w';//設定面相，向上 
					}
					break;
				case 's':
				case 80://down
					if(CanMove(x,y+1)){
						gotoxy(x,y);
						printf(" ");
						gotoxy(x,++y);
						printf("%c",name);
						face = 's';//設定面相，向下 
					}
					break;
				case 'a':
				case 75://left
					if(CanMove(x-1,y)){
						gotoxy(x,y);
						printf(" ");
						gotoxy(--x,y);
						printf("%c",name);
						face = 'a';//設定面相，向左 
					}
					break;
				case 'd':
				case 77://right
					if(CanMove(x+1,y)){
						gotoxy(x,y);
						printf(" ");
						gotoxy(++x,y);
						printf("%c",name);
						face = 'd';//設定面向，向右 
					}
					break;
			} 
		}	
		void FireZipline(char input){
			if(input==' '){
				Zipline *z = new Zipline(this,x,y,face,map); 
				link->NewNode(z);
				skill = 1;
			}
		}
		void Set_Skill(int n){
			skill=n;
		}
		void Action(){
			char input = Input();
			if(input!=NULL&&skill==0){
				Move(input);
				FireZipline(input);
			}
		}
};

class Monster:public Unit,public Node{
	private:
		Player *player;//怪物要知道 Player在哪
		int targetX;
		int targetY;
		static int distance;//怪物行走速度控制 
	public:
		Monster(){
			player = NULL;
			targetX = targetY = 0;
		}
		Monster(int a,int b,Map *m,char n,Player *p):Unit(a,b,m,n){
			player = p;
			targetX = targetY = 0;
		}
		void Action(){
			Move();
		}
		void Move(){////////////////////////////////////////////////////////////////////
			finish= clock();
			if(finish-start>= distance){
				MoveBy_A_Star(x,y,player->x_Value(),player->y_Value(),map,name,&x,&y);
				start = clock();
			}
		}
		void Set_distance(int t){
			distance = t;
		}
};
int Monster::distance = MonsterMoveTime;

class FakerMonster:public Monster{
	private:
		
	public:
		void Action(){
			Move();
		}
		void IsTimeToShow(){
			
		}
		void GetHelper(){
			
		}
};
/* 
class FakerMonsterHelper:Unit(){
	private:
		
	public:
		Move(){
			
		}
} */ 
class MonsterHouse{//生怪屋 
	private:
		Player *player;
		Map *map;
		Linkedlist *link;
		clock_t start;
		clock_t finish;
	public:
		MonsterHouse(){
			start = clock();
			player = NULL;
			map = NULL;
			link = NULL;
		}
		MonsterHouse(Map *m,Player *p,Linkedlist *l){
			start = clock();
			map = m;
			player = p;
			link = l;
			Monster *mo = new Monster(WIDTH-2,HIGHT-1,map,'M',player);//開局即有怪物 
			link->NewNode(mo); 
		}
		void NewMonster(){
			finish = clock();
			if(finish-start>=MonsterTime){ 
				Monster *m = new Monster(WIDTH-2,HIGHT-1,map,'M',player);
				link->NewNode(m); 
				start = clock();
			}
		} 
};

class S:public Monster{
	private:
		Player *player;
		int x;
		int y;
		bool open;
	public:
		S(int a,int b,Player *p){
			x = a;
			y = b;
			player = p;
			open = 0;
			gotoxy(x,y);
			printf("%c",'S');
		}
		void Action(){
			if(x==player->x_Value()&&y==player->y_Value()){
				if(open==0){
					open=1;
					Set_distance(MonsterMoveTime*2);
					start = clock();
				}
			}
			if(open==1){
				finish = clock();
				if(finish-start>=5000){
					Set_distance(MonsterMoveTime);
					life=0;
				}
			} 
		}
};

class Prop{//生成道具的類別 
	private:
		Map *map;
		Player *player;
		Linkedlist *link;
		clock_t start;
		clock_t finish;
	public:
		Prop(){
			map=NULL;
			player=NULL;
		}
		Prop(Map *m,Player *p,Linkedlist *l){
			map = m;
			player = p;
			link = l;
			start = clock();
		}
		void Action(){
			finish = clock();
			if(finish-start>=Prop_Time){
				switch(1){
					case 0:
						NewF();
						break;
					case 1:
						NewS();
						break;
					case 2:
						NewB();
						break;
					case 3:
						NewT();
						break;
				}
				start = clock();
			}
		}
		void NewF(){
			
		}
		void NewS(){
			S *s = new S(10,1,player);
			link->NewNode(s);
		}
		void NewB(){
			
		}
		void NewT(){
		}
};

class Score:public Node{
	private:
		clock_t start;
		clock_t finish;
		clock_t timeScoreStart;
		clock_t timeScoreFinish;
		int score;
	public:
		Score(){
			start = clock();
			timeScoreStart = clock();
			score = 0;
			ShowScore();
			ShowTime();
			gotoxy(82,5);
			printf("======================================");
			gotoxy(90,7);
			printf(" WASD或方向鍵移動");
			gotoxy(90,9);
			printf("按下空白鍵發射繩索");
		}
		void Action(){
			ShowTime();
			ShowScore();
		}
		void ShowTime(){//印出遊戲經過時間 
			gotoxy(90,3);
			finish = clock();
			timeScoreFinish = clock();
			printf("存活時間: %5d 秒",(finish-start)/1000);
			if(timeScoreFinish-timeScoreStart>=5000){
				GetScore(100);
				timeScoreStart = clock();
			}
		}
		void ShowScore(){//印出分數 
			gotoxy(90,1);
			printf("Score: %8d 分",score);
		}
		void GetScore(int n){//增加分數的函數 
			score+=n;
			ShowScore();
		}
};

class UI{
	private:
		Map *map;
		Linkedlist *link;
		Player *pl;
		Node *player;
		Node *monster;
		Node *arrow;
		MonsterHouse *monsterHouse;
		Prop *prop;
		Score *score;
	public:
		UI(){
			Map *m = new Map;
			map = m;
			
			Linkedlist *l = new Linkedlist;
			link = l;
			
			Player *p = new Player(1,1,map,'P',link);
			player = p;
			pl=p;
			arrow = p;
			link->NewNode(player);
			
			MonsterHouse *h = new MonsterHouse(map,p,link);
			monsterHouse = h;
			
			Prop *pr = new Prop(map,pl,link);
			prop = pr;
			
			Score *s = new Score;
			score = s;			
		}
		void Run(){
			while(1){
				link->Link_Run();
				monsterHouse->NewMonster();
				prop->Action();
				score->Action();
			}
		}
};
void Zipline::Fly(){
	player->Move(face);
}
void Zipline::Action(){
	finish = clock();
	if(finish-start>=30){
		switch(face){
			case 'w':
				if(CanMove(x,y-1)){
					gotoxy(x,--y);
					cout<<'^';
				}
				else {
					Fly();//代表碰到牆壁了 要飛過去了 
					if(player->y_Value()==y){
						life=0;
						player->Set_Skill(0);
					}
				}
				break;
			case 'd':
				if(CanMove(x+1,y)){
					gotoxy(++x,y);
					cout<<'>';
				}
				else {
					Fly();//代表碰到牆壁了 要飛過去了 
					if(player->x_Value()==x){
						life=0;
						player->Set_Skill(0);
					}
				}
				break;
			case 's':
				if(CanMove(x,y+1)){
					gotoxy(x,++y);
					cout<<'v';
				}
				else {
					Fly();//代表碰到牆壁了 要飛過去了 
					if(player->y_Value()==y){
						life=0;
						player->Set_Skill(0);
					}
				}
				break;
			case 'a':
				if(CanMove(x-1,y)){
					gotoxy(--x,y);
					cout<<'<';
				}
				else {
					Fly();//代表碰到牆壁了 要飛過去了 
					if(player->x_Value()==x){
						life=0;
						player->Set_Skill(0);
					}
				}
				break;
		}
		start = clock();
	}
}
int main(){
	HideCursor();
	srand(time(NULL));
	
	UI ui;
	ui.Run();
	
	return 0;
}
