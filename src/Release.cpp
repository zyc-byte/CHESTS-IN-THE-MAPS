#include<bits/stdc++.h>
#include<windows.h>
#include<conio.h>
using namespace std;
const int N=55;
int n,a[N][N],sum[20],x,y,fx,money=0,hp=20,full=20,step=0;
int zombieX[5],zombieY[5],zombieS;
int skeletonX[5],skeletonY[5],skeletonS; 
int arrowX[200005],arrowY[200005],arrowFx[200005],arrowS;
int cx[4]= {0,1,0,-1};
int cy[4]= {1,0,-1,0};
bool vis[N][N],flag=0;
int thingMoney[20]= {1,5,100,100,200,300,1000,5000,100000,500000,300000,0,0,10};
bool in(int x,int y) {
	return (x>=1&&x<=n&&y>=1&&y<=n);
}
void checkRoad(int x,int y) { //����Ƿ���·
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
char printChar(int x) { //��ת�ַ�
	if(x<5){
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),0xF0);
		return ' ';
	}
	if(x>=5&&x<=7){
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),0xCF);
		return 'W';
	}
	if(x==8){
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),0xE0);
		return 'C';
	}
	if(x==9){
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),0xAF);
		return 'Z';
	}
	if(x==10){
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),0x9F);
		return 'Y';
	}
	if(x==11){
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),0x80);
		return 'D';
	}
	if(x==12){
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),0xAF);
		return '2';
	}
	if(x==13){
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),0xAF);
		return '1';
	}
	if(x==14){
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),0xF0);
		return 'S';
	}
	if(x==15){
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),0xF0);
		return '2'; 
	}
	if(x==16){
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),0xF0);
		return '1';
	}
	if(x==17){
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),0xF0);
		return 'A';
	}
}
void mp() { //�����ͼ
	printf(" �X�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�j�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�j�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�[ \n"); 
	printf(" �U ��Ǯ:%9d",money);
	printf(" �U ����:%9d",hp);
	printf("�U ��ʳ��:%7d�U \n",full);
	printf(" �^�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�m�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�m�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�a \n"); 
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
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),0x03);
		}
		printf("\n");
	}
}
void makeMp(int x,int y) { //������ͼ
	vis[x][y]=1;
	a[x][y]=rand()%8;
	for(int i=0; i<4; i++) {
		int tx=x+cx[i],ty=y+cy[i];
		if(in(tx,ty)&&a[tx][ty]==0) {
			makeMp(tx,ty);
		}
	}
}
void chest() { //������
	system("cls");
	printf("����\n");
	printf("�X�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�[ \n");
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
		if(i==0) printf("�U ԭʯ:%2d               �U ",ChestMoney),sum[0]+=ChestMoney;
		if(i==1) printf("�U ú̿:%2d               �U ",ChestMoney),sum[1]+=ChestMoney;
		if(i==2) printf("�U ����:%2d               �U ",ChestMoney),sum[2]+=ChestMoney;
		if(i==3) printf("�U ��:%2d               �U ",ChestMoney),sum[3]+=ChestMoney;
		if(i==4) printf("�U ��ʯ:%2d               �U ",ChestMoney),sum[4]+=ChestMoney;
		if(i==5) printf("�U ���ʯ:%2d             �U ",ChestMoney),sum[5]+=ChestMoney;
		if(i==6) printf("�U �̱�ʯ:%2d             �U ",ChestMoney),sum[6]+=ChestMoney;
		if(i==7) printf("�U ��ʯ:%2d               �U ",ChestMoney),sum[7]+=ChestMoney;
		if(i<7){
			printf("\n�d�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�g \n");
		}
	}
	printf("\n�^�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�a \n");
	printf("�ѹ���ϣ�\n");
	system("pause");
	return ;
}
void shop() { //�̵� 
	system("cls");
	while(1) {
		int shopSelect;
		printf("��ӭ�����̵�!\n");
		printf("��������Ҫ��ʲô:\n");
		printf("[0]�� [1]�� [2]�˳�\n");
		scanf("%d",&shopSelect);
		if(shopSelect==0) {
			printf("��Ǯ:%d\n",money);
			printf("1.ĩӰ����:$10,0000 ����������9*9�ķ�Χ��\n");
			printf("2.����ҩˮ:$50,0000 �ָ�����Ѫ\n");
			printf("3.�罦���˺�ҩˮ:$30,0000 ��5*5��Χ�ڵĹ����10���˺�\n");
			printf("4.���:$10 �ָ�3�㱥ʳ��\n");
			printf("��Ҫ���Ǹ�(���)?�����?");
			int buy,buyS;
			scanf("%d%d",&buy,&buyS);
			if(buy==1) {
				if(money>=thingMoney[8]*buyS){
					sum[8]+=buyS,money-=thingMoney[8]*buyS;
				} else{
					printf("�ϰ�:����͹���!���������ȥ!\n");
					Sleep(2000);
				}
			} else if(buy==2) {
				if(money>=thingMoney[9]*buyS){
					sum[9]+=buyS,money-=thingMoney[9]*buyS;
				} else{
					printf("�ϰ�:����͹���!���������ȥ!\n");
					Sleep(2000);
				}
			} else if(buy==3) {
				if(money>=thingMoney[10]*buyS){
					sum[10]+=buyS,money-=thingMoney[10]*buyS;
				} else{
					printf("�ϰ�:����͹���!���������ȥ!\n");
					Sleep(2000);
				}
			} else if(buy==4){
				if(money>=thingMoney[13]*buyS){
					sum[13]+=buyS,money-=thingMoney[13]*buyS;
				} else{
					printf("�ϰ�:���������,ȥ���������ȥ!\n");
				}
			}
		}
		if(shopSelect==1) {
			printf("��Ǯ:%d\n",money);
			for(int i=0; i<8; i++) {
				money+=sum[i]*thingMoney[i],sum[i]=0;
			}
			printf("�ɽ�! ��Ǯ:%d\n",money);
			Sleep(1000);
		}
		if(shopSelect==2){
			return ;
		}
		system("cls");
	}
}
void quickTeach(){//���ٽ�ѧ 
	system("cls");
	printf(" �X�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�[ \n");
	printf(" �U �̳�:                                                             �U \n");
	printf(" �U 1.' '=��,'W'=ǽ,'C'=����,'Y'=���,'Z'=��ʬ,'S'=����,'D'=��(����)  �U \n");
	printf(" �U 2.����:                                                           �U \n");
	printf(" �U   (1)w,a,s,d:��������                                             �U \n");
	printf(" �U   (2)o:����һ�η��������                                         �U \n");
	printf(" �U   (3)k:�������ܵĹ���                                             �U \n");
	printf(" �U   (4)r:�˳�                                                       �U \n");
	printf(" �U   (5)e:�򿪱���                                                   �U \n");
	printf(" �U   (6)t:�鿴�̳�                                                   �U \n");
	printf(" �U   (7)u:ʹ�õ���                                                   �U \n");
	printf(" �U   (8)q:�Զ���                                                     �U \n");
	printf(" �^�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�a \n"); 
}
void useThing(){
	printf("��������:\n");
	printf(" �X�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�[ \n");
	printf(" �U 1.ĩӰ����:%2d         �U \n",sum[8]);
	printf(" �d�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�g \n");
	printf(" �U 2.����ҩˮ:%2d         �U \n",sum[9]);
	printf(" �d�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�g \n");
	printf(" �U 3.�罦���˺�ҩˮ:%2d   �U \n",sum[10]);
	printf(" �^�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�a \n");
	printf("��Ҫʹ�õڼ�������?\n");
	int useSelect;
	scanf("%d",&useSelect);
	if(useSelect==1&&sum[8]>0){
		sum[8]--;
		printf("��Ҫ���͵�?:");
		int Ender_Pearl_X,Ender_Pearl_Y;
		scanf("%d%d",&Ender_Pearl_X,&Ender_Pearl_Y);
		Ender_Pearl_X=min(Ender_Pearl_X,x+4),Ender_Pearl_Y=min(Ender_Pearl_Y,y+4);
		Ender_Pearl_X=max(Ender_Pearl_X,x-4),Ender_Pearl_Y=max(Ender_Pearl_Y,y-4);
		a[x][y]=0;
		x=Ender_Pearl_X,y=Ender_Pearl_Y;
		printf("��� ʹ�� ĩӰ����,�ѽ� ��� ���͵� %d %d\n",x,y);
		Sleep(1000);
	}
	if(useSelect==2&&sum[9]>0){
		sum[9]--;
		hp=20;
		printf("��� ʹ�� ����ҩˮ,����ֵ �ָ��� 20/20\n");
	}
	if(useSelect==3&&sum[10]>0){
		sum[10]--;
		for(int i=max(1,x-2);i<=min(n,x+2);i++){
			for(int j=max(1,y-2);j<=min(n,y+2);j++){
				for(int k=1;k<=zombieS;k++){
					if(zombieX[k]==i&&zombieY[k]==j){
						a[zombieX[k]][zombieY[k]]=0;
						zombieX[k]=-100,zombieY[k]=-100;
						printf("��� ʹ�� �罦���˺�ҩˮ,chest:zombie %d��ɱ����\n",i);
					}
				}
			}
		}
		Sleep(1000);
	}
}
void seeBag(){
	system("cls");
	printf("����\n");
	printf("�X�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�[ \n");
	for(int i=0; i<13; i++) {
		if(i==0) printf("�U ԭʯ:%2d               �U ",sum[i]);
		if(i==1) printf("�U ú̿:%2d               �U ",sum[i]);
		if(i==2) printf("�U ����:%2d               �U ",sum[i]);
		if(i==3) printf("�U ��:%2d               �U ",sum[i]);
		if(i==4) printf("�U ��ʯ:%2d               �U ",sum[i]);
		if(i==5) printf("�U ���ʯ:%2d             �U ",sum[i]);
		if(i==6) printf("�U �̱�ʯ:%2d             �U ",sum[i]);
		if(i==7) printf("�U ��ʯ:%2d               �U ",sum[i]);
		if(i==8) printf("�U ĩӰ����:%2d           �U ",sum[i]);
		if(i==9) printf("�U ����ҩˮ:%2d           �U ",sum[i]);
		if(i==10) printf("�U �罦���˺�ҩˮ:%2d     �U ",sum[i]);
		if(i==11) printf("�U ����:%2d               �U ",sum[i]);
		if(i==12) printf("�U ��ͷ:%2d               �U ",sum[i]);
		if(i<12){
			printf("\n�d�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�g \n");
		}
	}
	printf("\n�^�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�a \n");
	system("pause");
}
void eatFood(){
	printf("ʳ��:\n");
	printf("�X�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�[ \n");
	printf("�U 1.����:%2d             �U \n",sum[11]);
	printf("�d�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�g \n");
	printf("�U 2.���:%2d             �U \n",sum[13]);
	printf("�^�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�a \n");
	int foodSelect,foodS;
	printf("����һ��ʳ��,�Զ���?");
	scanf("%d%d",&foodSelect,&foodS);
	if(foodSelect==1){
		foodS=min(foodS,sum[11]);
		sum[11]-=foodS;
		full=min(20,full+foodS*1);
	}
	if(foodSelect==2){
		foodS=min(foodS,sum[13]);
		sum[13]-=foodS;
		full=min(20,full+foodS*3);
	}
	return ;
}
void game() {
	system("color 03");
	printf("\n\n\n\n\n");
	printf("                                          �X�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�[ \n");
	printf("					  �U       CHESTS IN         �U\n");
	printf("					  �U                         �U\n");
	printf("					  �U            THE MAPS     �U\n");
	printf("					  �U                         �U\n");
	printf("					  �U       C++ EDITON        �U\n");
	printf("                                          �^�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�a \n");
	printf("\n\n\n\n\n");
	printf("						���������ʼ��Ϸ...\n\n\n\n\n\n\n\n\n\n\n\n\n");
	printf("Alpha 0.6.7(���԰�)\n");
	system("pause");
	system("cls");
	printf("��Ϸѡ��:\n");
	printf("��ͼ��С(5-20):[  ]\b\b\b");
	scanf("%d",&n);
	n=max(5,n),n=min(20,n);
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
			int MapX,MapY;//������ͼ�����
			MapX=rand()%(n+1),MapY=rand()%(n+1);
			if(MapX==0) MapX++;
			if(MapY==0) MapY++;
			makeMp(MapX,MapY);
			checkRoad(2,2);//�Ƿ���·�ܵ��յ�
			if(flag==1) {
				break;
			}
			flag=0;//��ʼ��
			memset(vis,false,sizeof(vis));
		}
		zombieS=3;
		skeletonS=2;
		for(int i=1; i<=rand()%n; i++){
			int gx=rand()%(n+1),gy=rand()%(n+1);
			if(gx==0) gx++;
			if(gy==0) gy++;
			a[gx][gy]=8;
		}
		for(int i=1; i<=zombieS; i++) {//���ɽ�ʬ  
			zombieX[i]=rand()%(n+1),zombieY[i]=rand()%(n+1);
			if(zombieX[i]==0) zombieX[i]++;
			if(zombieY[i]==0) zombieY[i]++;
			if(zombieX[i]==n-1) zombieX[i]--;
			if(zombieY[i]==n-1) zombieY[i]--;
			a[zombieX[i]][zombieY[i]]=9;
		}
		for(int i=1; i<=skeletonS; i++){//�������� 
			skeletonX[i]=rand()%(n+1),skeletonY[i]=rand()%(n+1);
			if(skeletonX[i]==0) skeletonX[i]++;
			if(skeletonY[i]==0) skeletonY[i]++;
			if(skeletonX[i]==n-1) skeletonX[i]--;
			if(skeletonY[i]==n-1) skeletonY[i]--;
			a[skeletonX[i]][skeletonY[i]]=14;
		}
		memset(arrowX,-100,sizeof(arrowX));
		memset(arrowY,-100,sizeof(arrowY));
		x=2,y=2;
		a[x][y]=10;
		a[n-1][n-1]=11;
//������ͼ 
		while(1) {
			if(step>0&&step%10==0&&full>0){//���±�ʳ��
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
				printf("                                    �����ˣ�\n\n\n\n\n\n");
				int YN;
				printf("                      [0]�˳���Ϸ [1]���� [2]���ر�����Ļ");
				arrowS=0;
				memset(arrowX,-100,sizeof(arrowX));
				memset(arrowY,-100,sizeof(arrowY));
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
				printf("                           666,�㵽���յ�!\n\n\n");
				int YN;
				printf("                      [0]�˳� [1]���� [2]�����̵�\n");
				scanf("%d",&YN);
				arrowS=0;
				memset(arrowX,-100,sizeof(arrowX));
				memset(arrowY,-100,sizeof(arrowY));
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
			for(int i=1;i<=zombieS;i++){//��ʬ�ƶ� 
				if(abs(zombieX[i]-x)<3&&abs(zombieY[i]-y)<3&&zombieX[i]!=-100&&zombieY[i]!=-100){
					int zombieType=a[zombieX[i]][zombieY[i]];
					a[zombieX[i]][zombieY[i]]=0;
					if(x<zombieX[i]&&a[zombieX[i]-1][zombieY[i]]<5) zombieX[i]--;
					else if(x>zombieX[i]&&a[zombieX[i]+1][zombieY[i]]<5) zombieX[i]++;
					else if(y<zombieY[i]&&a[zombieX[i]][zombieY[i]-1]<5) zombieY[i]--;
					else if(y>zombieY[i]&&a[zombieX[i]][zombieY[i]+1]<5) zombieY[i]++;
					a[zombieX[i]][zombieY[i]]=zombieType;
				}
			}
			for(int i=1;i<=skeletonS;i++){//�����ƶ�  
				if((skeletonX[i]!=x || skeletonY[i]!=y)&&skeletonX[i]!=-100&&skeletonY[i]!=-100){
					int skeletonType=a[skeletonX[i]][skeletonY[i]];
					a[skeletonX[i]][skeletonY[i]]=0;
					if(skeletonX[i]>x && a[skeletonX[i]-1][skeletonY[i]]<5) skeletonX[i]--;
					else if(skeletonX[i]<x && a[skeletonX[i]+1][skeletonY[i]]<5) skeletonX[i]++;
					else if(skeletonY[i]>y && a[skeletonX[i]][skeletonY[i]-1]<5) skeletonY[i]--;
					else if(skeletonY[i]<y && a[skeletonX[i]][skeletonY[i]+1]<5) skeletonY[i]++;
					a[skeletonX[i]][skeletonY[i]]=skeletonType;
				}
			}
			for(int i=1;i<=arrowS;i++){//��ʸ�ƶ� 
				if(arrowX[i]!=-100&&arrowY[i]!=-100&&a[arrowX[i]+cx[arrowFx[i]]][arrowY[i]+cy[arrowFx[i]]]<5){
					a[arrowX[i]][arrowY[i]]=0;
					arrowX[i]+=cx[arrowFx[i]],arrowY[i]+=cy[arrowFx[i]];
					a[arrowX[i]][arrowY[i]]=17;
				}
				else if(!in(arrowX[i]+cx[arrowFx[i]],arrowY[i]+cy[arrowFx[i]])||a[arrowX[i]+cx[arrowFx[i]]][arrowY[i]+cy[arrowFx[i]]]){
					a[arrowX[i]][arrowY[i]]=0;
					arrowX[i]=-100,arrowY[i]=-100;
				}
			}
			for(int i=1;i<=skeletonS;i++){//������� 
				if(skeletonX[i]==x){
					if(skeletonY[i]<y&&a[skeletonX[i]][skeletonY[i]+1]<5){
						arrowS++;
						arrowX[arrowS]=skeletonX[i],arrowY[arrowS]=skeletonY[i]+1,arrowFx[arrowS]=0;
						a[arrowX[arrowS]][arrowY[arrowS]]=17;
					}
					else if(skeletonY[i]>y&&a[skeletonX[i]][skeletonY[i]-1]<5){
						arrowS++;
						arrowX[arrowS]=skeletonX[i],arrowY[arrowS]=skeletonY[i]-1,arrowFx[arrowS]=2;
						a[arrowX[arrowS]][arrowY[arrowS]]=17;
					}
				}
				else if(skeletonY[i]==y){
					if(skeletonX[i]<x&&a[skeletonX[i]+1][skeletonY[i]]<5){
						arrowS++;
						arrowX[arrowS]=skeletonX[i]+1,arrowY[arrowS]=skeletonY[i],arrowFx[arrowS]=1;
						a[arrowX[arrowS]][arrowY[arrowS]]=17;
					}
					else if(skeletonX[i]>x&&a[skeletonX[i]-1][skeletonY[i]]<5){
						arrowS++;
						arrowX[arrowS]=skeletonX[i]-1,arrowY[arrowS]=skeletonY[i],arrowFx[arrowS]=3;
						a[arrowX[arrowS]][arrowY[arrowS]]=17;
					}
				}
			}
			mp();
			for(int i=1;i<=zombieS;i++){//��⽩ʬ���� 
				for(int j=0;j<4;j++){
					int zo_killX=zombieX[i]+cx[j],zo_killY=zombieY[i]+cy[j];
					if(x==zo_killX&&y==zo_killY){
						hp-=2;
						printf("��� �� ��ʬ%d(chest:zombie %d) ������\n",i,i);
					}
				}
			}
			for(int i=1;i<=arrowS;i++){
				int ar_killX=arrowX[i]+cx[arrowFx[i]],ar_killY=arrowY[i]+cy[arrowFx[i]];
				if(x==ar_killX&&y==ar_killY){
					hp--;
					printf("��� �� ��ʸ%d(chest:arrow %d) ������\n",i,i);
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
				if(in(goX,goY)&&(a[goX][goY]<5||a[goX][goY]==11||a[goX][goY]==17)) {
					a[goX][goY]=10;
					a[x][y]=0;
					x=goX,y=goY;
					step++;
				} else {
					printf("��Чָ��!\n");
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
					printf("��Чָ��!\n");
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
						for(int j=1;j<=zombieS;j++){
							if(killX==zombieX[j] && killY==zombieY[j]){
								int getZombieMeat=rand()%6;
								zombieX[j]=-100,zombieY[j]=-100;
								printf("��� ��ɱ�� ��ʬ %d,��ȡ���� * %d\n",j,getZombieMeat);
								sum[11]+=getZombieMeat;
								Sleep(1000);
								break;
							}
						}
					}
					else if(a[killX][killY]==14) a[killX][killY]=15;
					else if(a[killX][killY]==15) a[killX][killY]=16;
					else if(a[killX][killY]==16){
						a[killX][killY]=0;
						for(int j=1;j<=skeletonS;j++){
							if(killX==skeletonX[j]&&killY==skeletonY[j]){
								int getBones=rand()%4;
								skeletonX[j]=-100,skeletonY[j]=-100;
								printf("��� ��ɱ�� ���� %d,��ȡ ��ͷ * %d\n",i,getBones);
								sum[12]+=getBones;
								Sleep(1000);
								break;
							}
						}
					}
					else{
						continue;
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
//��Ϸ
	}
}
int main() {
	srand(time(0));
	game();
	return 0;
}
