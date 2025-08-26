#include<bits/stdc++.h>
#include<windows.h>
#include<conio.h>
using namespace std;
const int N=55;
int n,a[N][N],sum[15],x,y,fx,money=0,hp=20,full=20,step=0;
int zombieX[5],zombieY[5],zombieS;
int cx[4]= {0,1,0,-1};
int cy[4]= {1,0,-1,0};
bool vis[N][N],flag=0;
int thingMoney[15]= {1,5,100,100,200,300,1000,5000,10000000,50000000,30000000};
bool in(int x,int y) {
	return (x>=1&&x<=n&&y>=1&&y<=n);
}
void checkRoad(int x,int y) { //检测是否有路
	if(x==n-1&&y==n-1) {
		flag=1;
		return ;
	}
	if(a[x][y]>5) {
		return ;
	}
	vis[x][y]=1;
	for(int i=0; i<4; i++) {
		int tx=x+cx[i],ty=y+cy[i];
		if(in(tx,ty)&&(a[tx][ty]<5||a[tx][ty]==11)&&vis[tx][ty]==0) {
			checkRoad(tx,ty);
		}
	}
	return ;
}
char printChar(int x) { //数转字符
	if(x<5) {
		return ' ';
	}
	if(x>=5&&x<=7) {
		return 'W';
	}
	if(x==8) {
		return 'C';
	}
	if(x==9) {
		return 'Z';
	}
	if(x==10) {
		return 'Y';
	}
	if(x==11) {
		return 'D';
	}
	if(x==12){
		return '2';
	}
	if(x==13){
		return '1';
	}
}
void mp() { //输出地图
	printf(" ╔════════════════╦═══════════════╦═══════════════╗ \n"); 
	printf(" ║ 金钱:%9d",money);
	printf(" ║ 生命:%9d",hp);
	printf("║ 饱食度:%7d║ \n",full);
	printf(" ╚════════════════╩═══════════════╩═══════════════╝ \n"); 
	printf("\n");
	printf("  ");
	for(int i=1; i<=n; i++) {
		printf("%3d",i);
	}
	printf("\n");
	for(int i=1; i<=n; i++) {
		printf("%2d ",i);
		for(int j=1; j<=n; j++) {
			printf("[%c]",printChar(a[i][j]));
		}
		printf("\n");
	}
}
void makeMp(int x,int y) { //构建地图
	vis[x][y]=1;
	a[x][y]=rand()%8;
	for(int i=0; i<4; i++) {
		int tx=x+cx[i],ty=y+cy[i];
		if(in(tx,ty)&&a[tx][ty]==0) {
			makeMp(tx,ty);
		}
	}
}
void chest() { //开箱子
	system("cls");
	printf("箱子\n");
	printf("╔═══════════════════════╗ \n");
	for(int i=0; i<8; i++) {
		int ChestMoney;
		if(i==1){
			ChestMoney=rand()%64;
		}
		if(i<=3) {
			ChestMoney=rand()%20;
		}
		if(i>3&&i<=5) {
			ChestMoney=rand()%10;
		}
		if(i>5){
			ChestMoney=rand()%3;
		}
		if(i==0) printf("║ 原石:%2d               ║ ",ChestMoney),sum[0]+=ChestMoney;
		if(i==1) printf("║ 煤炭:%2d               ║ ",ChestMoney),sum[1]+=ChestMoney;
		if(i==2) printf("║ 铁锭:%2d               ║ ",ChestMoney),sum[2]+=ChestMoney;
		if(i==3) printf("║ 金锭:%2d               ║ ",ChestMoney),sum[3]+=ChestMoney;
		if(i==4) printf("║ 红石:%2d               ║ ",ChestMoney),sum[4]+=ChestMoney;
		if(i==5) printf("║ 青金石:%2d             ║ ",ChestMoney),sum[5]+=ChestMoney;
		if(i==6) printf("║ 绿宝石:%2d             ║ ",ChestMoney),sum[6]+=ChestMoney;
		if(i==7) printf("║ 钻石:%2d               ║ ",ChestMoney),sum[7]+=ChestMoney;
		if(i<7){
			printf("\n╠═══════════════════════╣ \n");
		}
	}
	printf("\n╚═══════════════════════╝ \n");
	printf("搜刮完毕！\n");
	system("pause");
	return ;
}
void shop() { //商店
	system("cls");
	while(1) {
		int shopSelect;
		printf("欢迎光临商店!\n");
		printf("请输入你要干什么:\n");
		printf("[0]买 [1]卖 [2]退出\n");
		scanf("%d",&shopSelect);
		if(shopSelect==0) {
			printf("金钱:%d\n",money);
			printf("1.末影珍珠:$1000,0000 传送至附近8*8的范围内\n");
			printf("2.治疗药水:$5000,0000 恢复至满血\n");
			printf("3.喷溅型伤害药水:$3000,0000 对5*5范围内的鬼造成10点伤害\n");
			printf("你要买那个(编号)?");
			int buy;
			scanf("%d",&buy);
			if(buy==1) {
				if(money>=thingMoney[8]){
					sum[8]++,money-=thingMoney[8];
				} else{
					printf("老板:买不起就滚吧!开你的箱子去!\n");
					Sleep(2000);
				}
			} else if(buy==2) {
				if(money>=thingMoney[9]){
					sum[9]++,money-=thingMoney[9];
				} else{
					printf("老板:买不起就滚吧!开你的箱子去!\n");
					Sleep(2000);
				}
			} else if(buy==3) {
				if(money>=thingMoney[10]){
					sum[10]++,money-=thingMoney[10];
				} else{
					printf("老板:买不起就滚吧!开你的箱子去!\n");
					Sleep(2000);
				}
			}
		}
		if(shopSelect==1) {
			printf("金钱:%d\n",money);
			for(int i=0; i<11; i++) {
				money+=sum[i]*thingMoney[i],sum[i]=0;
			}
			printf("成交! 金钱:%d\n",money);
			Sleep(1000);
		}
		if(shopSelect==2){
			return ;
		}
		system("cls");
	}
}
void quickTeach(){//快速教学 
	system("cls");
	printf(" ╔══════════════════════════════════════════════════════════╗ \n");
	printf(" ║ 教程:                                                    ║ \n");
	printf(" ║ 1.' '=空,'W'=墙,'C'=箱子,'Y'=玩家,'Z'=僵尸,'D'=门(出口)  ║ \n");
	printf(" ║ 2.操作:                                                  ║ \n");
	printf(" ║   (1)w,a,s,d:上下左右                                    ║ \n");
	printf(" ║   (2)o:开上一次方向的箱子                                ║ \n");
	printf(" ║   (3)k:攻击四周的怪物                                    ║ \n");
	printf(" ║   (4)r:退出                                              ║ \n");
	printf(" ║   (5)e:打开背包                                          ║ \n");
	printf(" ║   (6)t:查看教程                                          ║ \n");
	printf(" ║   (7)u:使用道具                                          ║ \n");
	printf(" ║   (8)q:吃东西                                            ║ \n");
	printf(" ╚══════════════════════════════════════════════════════════╝ \n"); 
}
void useThing(){
	printf("背包道具:\n");
	printf(" ╔═══════════════════════╗ \n");
	printf(" ║ 1.末影珍珠:%2d         ║ \n",sum[8]);
	printf(" ╠═══════════════════════╣ \n");
	printf(" ║ 2.治疗药水:%2d         ║ \n",sum[9]);
	printf(" ╠═══════════════════════╣ \n");
	printf(" ║ 3.喷溅型伤害药水:%2d   ║ \n",sum[10]);
	printf(" ╚═══════════════════════╝ \n");
	printf("你要使用第几个道具?\n");
	int useSelect;
	scanf("%d",&useSelect);
	if(useSelect==1&&sum[8]>0){
		sum[8]--;
		printf("你要传送到?:");
		int Ender_Pearl_X,Ender_Pearl_Y;
		scanf("%d%d",&Ender_Pearl_X,&Ender_Pearl_Y);
		a[x][y]=0;
		x=Ender_Pearl_X,y=Ender_Pearl_Y;
		printf("玩家 使用 末影珍珠,已将 玩家 传送到 %d %d\n",x,y);
		Sleep(1000);
	}
	if(useSelect==2&&sum[9]>0){
		sum[9]--;
		hp=20;
		printf("玩家 使用 治疗药水,生命值 恢复至 20/20\n");
	}
	if(useSelect==3&&sum[10]>0){
		sum[10]--;
		for(int i=max(1,x-2);i<=min(n,x+2);i++){
			for(int j=max(1,y-2);j<=min(n,y+2);j++){
				for(int k=1;k<=zombieS;k++){
					if(zombieX[k]==i&&zombieY[k]==j){
						printf("玩家 使用 喷溅型伤害药水,chest:zombie %d\n",i);
					}
				}
			}
		}
	}
}
void seeBag(){
	system("cls");
	printf("背包\n");
	printf("╔═══════════════════════╗ \n");
	for(int i=0; i<12; i++) {
		if(i==0) printf("║ 原石:%2d               ║ ",sum[i]);
		if(i==1) printf("║ 煤炭:%2d               ║ ",sum[i]);
		if(i==2) printf("║ 铁锭:%2d               ║ ",sum[i]);
		if(i==3) printf("║ 金锭:%2d               ║ ",sum[i]);
		if(i==4) printf("║ 红石:%2d               ║ ",sum[i]);
		if(i==5) printf("║ 青金石:%2d             ║ ",sum[i]);
		if(i==6) printf("║ 绿宝石:%2d             ║ ",sum[i]);
		if(i==7) printf("║ 钻石:%2d               ║ ",sum[i]);
		if(i==8) printf("║ 末影珍珠:%2d           ║ ",sum[i]);
		if(i==9) printf("║ 治疗药水:%2d           ║ ",sum[i]);
		if(i==10) printf("║ 喷溅型伤害药水:%2d     ║ ",sum[i]);
		if(i==11) printf("║ 腐肉:%2d               ║ ",sum[i]);
		if(i<11){
			printf("\n╠═══════════════════════╣ \n");
		}
	}
	printf("\n╚═══════════════════════╝ \n");
	system("pause");
}
void eatFood(){
	printf("食物:\n");
	printf("╔═══════════════════════╗ \n");
	printf("║ 1.腐肉:%2d             ║ \n",sum[11]);
	printf("╚═══════════════════════╝ \n");
	int foodSelect,foodS;
	printf("吃哪一个食物,吃多少?");
	scanf("%d%d",&foodSelect,&foodS);
	if(foodSelect==1){
		foodS=min(foodS,sum[11]);
		sum[11]-=foodS;
		full=min(20,full+foodS*1);
	}
	return ;
}
void game() {
	system("color 03");
	printf("\n\n\n\n\n");
	printf("                                          ╔═════════════════════════╗ \n");
	printf("					  ║       CHESTS IN         ║\n");
	printf("					  ║                         ║\n");
	printf("					  ║            THE MAPS     ║\n");
	printf("					  ║                         ║\n");
	printf("					  ║       C++ EDITON        ║\n");
	printf("                                          ╚═════════════════════════╝ \n");
	printf("\n\n\n\n\n");
	printf("						按任意键开始游戏...\n\n\n\n\n\n\n\n\n\n\n\n\n");
	printf("Alpha 0.5.1(测试版)\n");
	system("pause");
	system("cls");
	printf("游戏选项:\n");
	printf("地图大小(5-20):[  ]\b\b\b");
	scanf("%d",&n);
	n=max(5,n),n=min(20,n);
	printf("地图难度(1-5):[  ]\b\b\b");
	quickTeach();
	system("pause");
	system("cls");
	while(1) {
		hp=20;
		full=20;
		flag=0;
		memset(a,0,sizeof(a));
		memset(vis,false,sizeof(vis));
		while(1) {
			int MapX,MapY;//构建地图的起点
			MapX=rand()%(n+1),MapY=rand()%(n+1);
			if(MapX==0) MapX++;
			if(MapY==0) MapY++;
			makeMp(MapX,MapY);
			checkRoad(2,2);//是否有路能到终点
			if(flag==1) {
				break;
			}
			flag=0;//初始化
			memset(vis,false,sizeof(vis));
		}
		zombieS=3;
		x=2,y=2;
		a[x][y]=10;
		for(int i=1; i<=rand()%n; i++){
			int gx=rand()%(n+1),gy=rand()%(n+1);
			if(gx==0) gx++;
			if(gy==0) gy++;
			a[gx][gy]=8;
		}
		for(int i=1; i<=zombieS; i++) {
			zombieX[i]=rand()%(n+1),zombieY[i]=rand()%(n+1);
			if(zombieX[i]==0) zombieX[i]++;
			if(zombieY[i]==0) zombieY[i]++;
			a[zombieX[i]][zombieY[i]]=9;
		}
		a[n-1][n-1]=11;
//构建地图 
		while(1) {
			if(step>0&&step%10==0&&full>0){//更新饱食度
				full--;
			}
			if(full==20){
				hp++;
				hp=min(hp,20);
			}
			if(full==0){
				hp--;
			}
			if(hp<=0){
				memset(sum,0,sizeof(sum));
				money=0;
				hp=20;
				a[x][y]=0;
				system("cls");
				system("color CF");
				printf("\n\n\n\n\n\n\n");
				printf("                                    你死了！\n\n\n\n\n\n");
				int YN;
				printf("                      [0]退出游戏 [1]重生 [2]返回标题屏幕");
				scanf("%d",&YN);
				if(YN==0) {
					return ;
				}
				if(YN==1) {
					system("color 03");
					break;
				}
				if(YN==2) {
					game();
					return ;
				}
			}
			if(x==n-1&&y==n-1) {
				printf("\n\n\n\n\n\n");
				printf("                           666,你到了终点!\n\n\n");
				int YN;
				printf("                      [0]退出 [1]继续 [2]进入商店\n");
				scanf("%d",&YN);
				if(YN==0) {
					return ;
				}
				if(YN==1) {
					break;
				}
				if(YN==2) {
					shop();
					break;
				}
			}
			system("cls");
			a[x][y]=10;
			mp();
			for(int i=1;i<=zombieS;i++){
				if(abs(zombieX[i]-x)<2&&abs(zombieY[i]-y)<2){
					hp-=2;
					printf("玩家 被 僵尸%d(chest:zombie %i) 攻击了\n",i,i);
				}
			}
			char todo;
			todo=getch();
			int goX=x,goY=y;
			if(todo=='d'||todo=='s'||todo=='a'||todo=='w') {
				if(todo=='d') goY++,fx=0;
				if(todo=='s') goX++,fx=1;
				if(todo=='a') goY--,fx=2;
				if(todo=='w') goX--,fx=3;
				if(in(goX,goY)&&(a[goX][goY]<5||a[goX][goY]==11)) {
					a[goX][goY]=10;
					a[x][y]=0;
					x=goX,y=goY;
					step++;
				} else {
					printf("无效指令!\n");
					Sleep(500);
				}
			} else if(todo=='r') {
				return ;
			} else if(todo=='o') {
				goX=x,goY=y;
				if(fx==0) goY++,fx=0;
				if(fx==1) goX++,fx=1;
				if(fx==2) goY--,fx=2;
				if(fx==3) goX--,fx=3;
				if(in(goX,goY)&&(abs(goX-x)<2&&abs(goY-y)<2)&&a[goX][goY]==8) {
					chest();
					a[goX][goY]=0;
				} else {
					printf("无效指令!\n");
					Sleep(500);
				}
			} else if(todo=='t'){
				quickTeach();
				system("pause");
			} else if(todo=='k'){
				for(int i=0;i<4;i++){
					int killX=x+cx[i],killY=y+cy[i];
					if(a[killX][killY]==9) a[killX][killY]=12;
					else if(a[killX][killY]==12) a[killX][killY]=13;
					else if(a[killX][killY]==13){
						a[killX][killY]=0;
						for(int i=1;i<=zombieS;i++){
							if(killX==zombieX[i] && killY==zombieY[i]){
								int getZombieMeat=rand()%6;
								zombieX[i]=-1,zombieY[i]=-1;
								printf("玩家 击杀了 僵尸 %d,获取腐肉 * %d\n",i,getZombieMeat);
								sum[11]+=getZombieMeat;
								Sleep(1000);
								break;
							}
						}
					}
				}
			} else if(todo=='u'){
				system("cls");
				useThing();
			} else if(todo=='e'){
				seeBag();
			} else if(todo=='q'){
				eatFood();
			}
			Sleep(1);
			system("cls");
		}
//游戏
	}
}
int main() {
	srand(time(0));
	game();
	return 0;
}
