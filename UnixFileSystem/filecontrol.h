#pragma once
#include"struct.h"
#include<iostream>
#include <time.h>
#include <string>
#include <vector>
#include <conio.h>
#include<stack>
using namespace std;

#define blocknum 150  //盘块数
#define blocksize 512 //盘块大小
#define finodeblocknum  20//索引节点盘块数
#define finodenum 110
#define fileblockstart 22// finodeblocknum+2

struct inode* root;//内存根节点索引节点
struct superblock* super;//超级块信息
FILE* disk;//模拟的磁盘，本质为文本文件
struct inode usedinode[blocknum];// 其中存储内存中的索引节点
struct user currentuser;
struct inode* current;//当前目录位置对应i节点
bool commandover = false;//用来退出

//写入超级块
int superin()
{
	time_t timer;
	time(&timer);
	super->modifytime = timer;//修改时间设为当前
	super->dirt = 0;
	fseek(disk, blocksize, SEEK_SET);//移动文件指针到盘块#1
	int write = fwrite(super, sizeof(struct superblock), 1, disk);
	fflush(disk);//刷新文件缓存
	if (write != 1)
	{
		cout << "超级块写入失败"<<endl;
		return 0;
	}
	//fclose(disk);
	return 1;
}

/*void entertest(const char* path)
{
	fopen_s(&disk, path, "rb+");
	int p=fseek(disk, blocksize, SEEK_SET);
	super = (struct superblock*)calloc(1, sizeof(struct superblock));
	superin();
}*/

//初始化磁盘，写入超级块和必要索引信息
int initialize(const char* path)
{
	 fopen_s(&disk, path, "rb+");
	if (disk== NULL)
	{
		return -1;
	}
	cout << "1";
	
	super = (struct superblock*)calloc(1, sizeof(struct superblock));	//分配超级块对象空间
	int t=fseek(disk, blocksize, SEEK_SET);//使用#1盘块，#0不用
	//cout << t << "ttt" << endl;
	fread(super, sizeof(struct superblock), 1, disk);		//读出超级块的数据

	for (int i = 0; i < maxfreeblock; i++)//初始化空闲盘块栈
	{
		super->freeblock[i] = 2 + finodeblocknum + i;//盘块0，1为不用块和超级块，二十个inode盘块
	}
	super->nextfreeblock = maxfreeblock;//栈顶

	for (int i = 0; i<maxfreeinode; i++)//初始化空闲节点栈
	{
		super->freeinode[i] = i;//空闲索引号从0开始
	}
	super->nextfreeinode = maxfreeinode;

	super->freeblocknum = blocknum - 2 - finodeblocknum;//可用文件盘块
	super->freeinodenum = finodenum;//可用空闲节点数目

	//cout << "2";
	superin();
	return 1;
}

//成组链接法配置盘块信息
int formatting(const char * path)
{
	fopen_s(&disk,path, "rb+");
	if (disk == NULL)
	{
		cout << "找不到磁盘"<<endl;
		//return -1;
	}
	fseek(disk, fileblockstart * blocksize, SEEK_SET);//指向文件盘块位置
	unsigned int group[maxfreeblock];//成组轮转法20块一组
	for (int i = 0; i<maxfreeblock; i++)
	{
		group[i] = i + fileblockstart;
	}
	for (int i = 0; i<(blocknum-2-finodeblocknum)/20; i++)//所有盘块分组
	{
		for (int j = 0; j<maxfreeblock; j++)
		{
			group[j] += maxfreeblock;
			cout << group[j] << " ";
		}
		cout << endl;
		fseek(disk, (fileblockstart + i * maxfreeblock) * blocksize, SEEK_SET);	//每个盘块组最后一个盘块开头有那个组所有盘块信息
		fwrite(group, sizeof(unsigned int), maxfreeblock, disk);//成组链接法初始化
	}
	return 1;
}

//系统启动 磁盘加载 读取超级块信息
int superload(const char* path)
{
	 fopen_s(&disk,path,"rb+");//读写方式，文件必须存在
	if (disk == NULL)
	{
		cout << "磁盘文件不存在"<<endl;
		return 0;
	}
	super = (struct superblock*)calloc(1, sizeof(struct superblock));//开辟空间给超级块结构
	fseek(disk,blocksize,SEEK_SET);//移动文件指针到盘块#1
	int read = fread(super,sizeof(struct superblock),1,disk);//读取具体数据
	if (read != 1)
	{
		cout << "超级块读取失败"<<endl;
		return 0;
	}
	return 1;
}

//输出超级块信息
void superInfo()
{
	cout << "freeBlock:" << endl;
	for (int i = 0; i<super->nextfreeblock; i++)
		cout << super->freeblock[i] << " ";
	cout << endl;
	cout << "freeInode:" << endl;
	for (int i = 0; i<super->nextfreeinode; i++)
		cout << super->freeinode[i] << " ";
	cout <<endl;
	cout << "nextFreeInode:" << super->nextfreeinode << endl;
	cout << "nextFreeBlock:" << super->nextfreeblock<< endl;
	cout << "freeBlockNum:" << super->freeblocknum << endl;
	cout << "freeInodeNum:" << super->freeinodenum << endl;
	cout << "time" << super->modifytime << endl;
}

//输出索引节点信息
void inodeinfo(inode* inode)
{
	cout << "ino:" << inode->inodeID << endl;
	cout << "owner:";
	cout << inode->finode.owner;
	cout << endl;
	cout << "group:";
	cout<<inode->finode.group;
	cout << endl;
	cout << "fileLink:" << inode->finode.filelink << endl;
	cout << "fileSize:" << inode->finode.size<< endl;
	for (int i = 0; i<6; i++)
		cout << "arr" << i << ":" << inode->finode.arr[i] << endl;
	cout << "power type:"<<inode->finode.power[0]<<endl;
}

//inode写入磁盘
int finodein(struct inode* inode)
{
	int ino;
	int place;
	place = blocksize + blocksize + inode->inodeID*sizeof(struct finode);
	fseek(disk,place,SEEK_SET);
	int result = fwrite(&(inode->finode),sizeof(struct finode),1,disk);
	fflush(disk);
	if (result != 1)
	{
		cout << "磁盘写入inode失败"<<endl;
		return 0;
	}
	return 1;
}


//分配内存i节点
struct inode *iget(int ino)//参数为对应索引节点编号
{
	int place;
	if (usedinode[ino].usercount != 0)//该节点被使用过，在内存中有记录
	{
		usedinode[ino].usercount++;
		return &usedinode[ino];
	}

	if (disk == NULL)
		return NULL;

	place = blocksize + blocksize + ino * sizeof(struct finode);//#0+#1+对应索引号*索引节点大小=索引节点位置
	fseek(disk,place,SEEK_SET);
	int read = fread(&usedinode[ino],sizeof(struct finode),1,disk);
	if (read != 1)
	{
		cout << "内存索引分配失败"<<endl;
		return NULL;
	}

	if (usedinode[ino].finode.filelink == 0)//该节点未被文件link过 即开启新文件
	{	
		//索引节点初始化
		usedinode[ino].finode.filelink++;
		usedinode[ino].finode.size = 0;
		usedinode[ino].inodeID = ino;
		time_t timer;
		time(&timer);
		usedinode[ino].finode.modifytime = timer;
		super->freeinodenum--;//总的磁盘空闲inode减少
		finodein(&usedinode[ino]);//写入磁盘
	}
	
	usedinode[ino].usercount++;//修改内存索引节点信息
	usedinode[ino].inodeID=ino;

	return &usedinode[ino];//返回内存索引节点
}

//节点分配
struct inode* ialloc()
{
	int ino;
	if (super->nextfreeinode != 0)//超级块空闲索引栈不为空
	{
		ino = super->freeinode[--(super->nextfreeinode)];//当前栈顶拿出分配,同时剩余栈节点-1
		super->dirt = 1;
		superin();//寻思直接把super写回算了
		return iget(ino);
	}
	else//索引栈为空时
	{
		finode *tmple = (struct finode*)calloc(1, sizeof(struct finode));
		fseek(disk,blocksize+blocksize,SEEK_SET);
		int num=0;
		for (int i = 0; i < finodenum; i++)//查找所有索引节点项
		{
			fread(tmple,sizeof(struct finode),1,disk);//将空栈用新的节点填满
			if (tmple->filelink == 0)
			{
				super->freeinode[num] = i;
				super->nextfreeinode++;//栈中可用
				num++;
			}
			if (num == maxfreeinode)
				break;
		}
		if (super->nextfreeinode == 0)
			return NULL;

		ino = super->freeinode[--(super->nextfreeinode)];//当前栈顶拿出分配
		super->dirt = 1;
		superin();//寻思直接把super写回算了
		return iget(ino);
	}
}


//节点回收
int ifree(struct inode* inode)
{
	if (super->nextfreeinode != 20)//空闲栈未满则放入栈
	{
		super->freeinode[super->nextfreeinode] = inode->inodeID;
		super->nextfreeinode++;
	}

	inode->usercount = 0;
	finodein(inode);

	super->freeinodenum++;
	super->dirt = 1;
	superin();//写回
	return 1;
}

//写盘块 根据盘块号
int bwrite(void * _Buf, unsigned short int bno, long int offset, int size, int count = 1)//数据流 盘块号 偏移量 数据大小 元素个数
{
	long place;
	int result;
	place = bno * blocksize + offset;//对应物理地址
	fseek(disk, place, SEEK_SET);
	result = fwrite(_Buf, size, count, disk);//返回写入元素个数
	fflush(disk);
	if (result != count)
	{
		cout << "盘块写入失败" << endl;
		return 0;
	}
	return 1;
}

//读盘块 根据盘块号
int bread(void * _Buf, unsigned short int bno, int offset, int size, int count = 1)//数据流 盘块号 偏移量 数据大小
{
	long place;
	int result;
	place = bno * blocksize + offset;//对应物理地址
	fseek(disk, place, SEEK_SET);
	result = fread(_Buf, size, count, disk);
	if (result != count)
	{
		cout << "盘块读取失败" << endl;
		return 0;
	}
	return 1;
}

//分配空闲盘块
int balloc()//返回盘块号
{
	int bno;
	if (super->freeblocknum <= 0)//磁盘完全满了
		return 0;
	if (super->nextfreeblock == 1)//当前空闲盘块栈只剩下一个元素 其中包含下一组盘块信息
	{
		super->nextfreeblock--;//取出盘块
		bno = super->freeblock[super->nextfreeblock];

		fseek(disk,bno*blocksize,SEEK_SET);//指针指向组最后的盘块
		int count = fread(super->freeblock,sizeof(int),maxfreeblock,disk);//读取下一组盘块号，最多20个
		
		super->nextfreeblock = count;//空闲栈中现有的盘块数
		super->freeblocknum--;//总的空闲盘块-1
		superin();//寻思直接写入修改

		return bno;//上组最后一个被分配
	}

	//直接从空闲栈中拿
	super->freeblocknum--;
	super->nextfreeblock--;
	superin();

	return super->freeblock[super->nextfreeblock];
}

//回收盘块
int bfree(int bno)//根据盘块号进行盘块回收
{
	if (super->nextfreeblock == 20)//空闲栈满了
	{
		bwrite(super->freeblock, bno, 0, sizeof(int), 20);//空闲栈中一组盘块都写到该回收块中
		super->nextfreeblock = 1;
		super->freeblock[0] = bno;//回收块变为栈底
	}
	else
	{
		super->freeblock[super->nextfreeblock] = bno;//放到栈顶
		super->nextfreeblock++;
	}
	super->freeblocknum++;
	super->dirt = 1;
	superin();
	return 1;
}

//初始化根目录
void makeroot()
{
	root = ialloc();//给根目录分配一个索引节点
	cout << "root ino:" << root->inodeID << endl;
					//dir * dir = (struct dir*)calloc(1, sizeof(struct dir));//root是根目录文件
					//bread(dir, 1+finodeblocknum+1, 0, sizeof(struct dir));//22块开始是文件盘块

	root->finode.arr[0] = balloc();//分配更目录盘块
	cout << "root block" << root->finode.arr[0] << endl;

	strcpy_s(root->finode.power,"drw-rw---");
	strcpy_s(root->finode.owner,"root");
	strcpy_s(root->finode.group,"all");
	finodein(root);//根目录写入盘块
}

//将开始位置s2拷贝到s1中
int strCpy(char *s1, char *s2, int offset)
{
	int len = strlen(s2);
	if (len <= offset)//无后续参数
		return 0;
	int i;
	for (i = 0; i<len - offset; i++)
	{
		s1[i] = s2[i + offset];
	}
	s1[i] = 0;//放置空字符以便进行长度计算
	return 1;
}

//切割 s1到s2
int substring(char* s1, char* s2, int start, int end)
{
	int place = 0;
	if (end == -1)
		end = strlen(s1);
	for (int i = start; i<end; i++)
		s2[place++] = s1[i];
	s2[place] = 0;
	return 1;
}

//找到字符在串中位置
int findpos(char* str, int start, const char needle)
{
	for (int i = start; i<strlen(str); i++)
	{
		if (str[i] == needle)
			return i;
	}
	return -1;
}

//登录 晚点再写
int login()
{
	int num;//用户数
	user *content;
	const char* filename = "user";//读取user文件中用户信息
	struct inode* temple = NULL;//存放找到的文件inode
	dir * dir = (struct dir*)calloc(1, sizeof(struct dir));

	int count = current->finode.size / sizeof(struct direct);//当前文件项数目

	int addrnum = count / dirnum + (count%dirnum >= 1 ? 1 : 0);
	addrnum>4 ? addrnum = 4 : NULL;
	bool find = false;
	for (int addr = 0; addr < addrnum; addr++)//查看当前目录是否已经存在
	{
		bread(dir, current->finode.arr[addr], 0, sizeof(struct dir));
		for (int i = 0; i < dir->dirNum; i++)
			if (strcmp(dir->direct[i].directName, filename) == 0)//找到目录
			{
				temple = iget(dir->direct[i].inodeID);//获取user文件的索引
				find = true;
				temple = iget(dir->direct[i].inodeID);
				//cout << dir->direct[i].inodeID << endl;
			}
		if (find)
			break;
	}
	if (!find)
	{
		cout << "未在该目录下找到文件" << endl;
		return -1;
	}
	else//读取用户
	{
		num = temple->finode.size / sizeof(struct user);
		int bno = temple->finode.arr[0];
		content = (user*)calloc(num, sizeof(struct user));//保存用户信息
		int result = bread(content, bno, 0, num * sizeof(struct user));
		/*for (int i = 0; i < num; i++)
		{
			cout << content[i].userName << endl;
			cout << content[i].userGroup << endl;
			cout << content[i].userPwd << endl;
		}*/
		//cout << temple->inodeID << endl;
	}
	//开始登录

	char username[maxname] = { 0 }, password[maxname] = { 0 };
	char ch;
	int i = 0;
	cout << "输入用户名:";
	ch = getchar();//获取输入
	while (ch != 10)//不为回车
	{
		username[i] = ch;//继续读取
		ch = getchar();
		i++;
	}
	bool findname = false;
	for (int i = 0; i < num; i++)
	{
		if (strcmp(username,content[i].userName)==0)
		{
			findname = true;
			cout << "输入密码:";
			int p = 0;
			ch = getchar();//获取输入
			while (ch != 10)//不为回车
			{
				password[p] = ch;//继续读取
				ch = getchar();
				p++;
			}
			/*for (int k = 0; k < maxname; k++)
			{
				ch = _getch();
				if (ch == 13)//为换行符
				{
					break;
				}
				password[i] = ch;
				cout << "*";
			}*/
			if (strcmp(password,content[i].userPwd) == 0)
			{
				//system("cls");
				cout << "登录成功" << endl;
				currentuser = content[i];//设置当前用户
				return 0;
			}
		}
	}
	if (!findname)
	{
		cout << "该用户不存在" << endl;
		return -1;
	}
	return -1;
}

//更换当前用户密码
int passwd()
{
	//user节点号为15
	inode* temple = iget(15);//获取user文件的索引节点

	int num = temple->finode.size / sizeof(struct user);
	int bno = temple->finode.arr[0];
	user* content = (user*)calloc(num, sizeof(struct user));//获取用户
	int result = bread(content, bno, 0, num * sizeof(struct user));

	int i ;
	for (i=0; i < num; i++)
	{
		if (strcmp(content[i].userName, currentuser.userName)==0)
			break;
	}

	char oldpwd[maxname] = { 0 }, newpwd[maxname] = { 0 }, repwd[maxname] = { 0 };

	cout << "输入原密码:" << endl;
	char ch;
	int p = 0;
		ch = getchar();//获取输入
		while (ch != 10)//不为回车
		{
			oldpwd[p] = ch;//继续读取
			ch = getchar();
			p++;
		}
	//cout << content[i].userPwd << endl;
	if (strcmp(oldpwd, content[i].userPwd) == 0)
	{
		cout << "输入新密码" << endl;

		int n = 0;
		ch = getchar();//获取新密码输入
		while (ch != 10)//不为回车
		{
			newpwd[n] = ch;//继续读取
			ch = getchar();
			n++;
		}

		cout << "确认新密码:" << endl;

		int ne = 0;
		ch = getchar();//获取新密码输入
		while (ch != 10)//不为回车
		{
			repwd[ne] = ch;//继续读取
			ch = getchar();
			ne++;
		}

		if (strcmp(newpwd,repwd) == 0)//完全成功，写入磁盘
		{
			strcpy_s(content[i].userPwd,newpwd);//改写
			bwrite(content, bno, 0, num*sizeof(user));//写入磁盘
		}
		else
		{
			cout << "两次密码输入不同" << endl;
			return -2;
		}
		
		return 0;
	}
	else
	{
		cout << "原密码错误，修改失败" << endl;
		return -1;
	}

	return 0;
}

//pwd 显示当前位置
int pwd()
{
	stack<char*> pwd;//栈存放找到的文件名
	struct inode * inode = current;
	struct inode * parent = NULL;

	while (inode != root)//一级一级找上去直到根目录
	{
		int ino = inode->inodeID;
		parent = inode->parent;//找到父节点

		int count = parent->finode.size / sizeof(struct direct);
		dir * dir = (struct dir*)calloc(1, sizeof(struct dir));

		int addrnum = count / dirnum + (count % dirnum >= 1 ? 1 : 0);
		addrnum>4 ? addrnum = 4 : NULL;
		for (int addr = 0; addr<addrnum; addr++)//从父目录目录项中找到当前文件名
		{
			bread(dir, parent->finode.arr[addr], 0, sizeof(struct dir));
			for (int i = 0; i<dir->dirNum; i++)
			{
				if (dir->direct[i].inodeID == ino)//ino匹配
				{
					pwd.push(dir->direct[i].directName);//放入文件名
					count = -1;
					break;
				}
			}
			if (count == -1)
				break;
		}

		inode = inode->parent;//对父节点重复操作
	}

	if (pwd.empty())//当前为root
		cout << "/";
	else
	{
		while (!pwd.empty())
		{
			cout << "/";
			cout.write(pwd.top(), strlen(pwd.top()));//避免输出后面的空字符
			pwd.pop();//栈遍历取出对应名字并输出
		}
	}
	cout << endl;
	return 1;
}

//目录跳转，修改current
inode* cd(char* path,inode* inode)
{
	int start;
	int more;
	char path1[directname] = { 0 };//存放下一级地址的名称

	if (path[0] == '/'&&strlen(path) == 1)//输入/则跳转回根目录
	{
		current = root;
		return NULL;
	}

	if (path[0] == '/')//绝对地址
		start = 1;
	else
		start = 0;
	
	more = findpos(path,1,'/');//寻找是否有后续地址
	//cout << more << endl;
	substring(path,path1,start,more);//切分当前地址并传入path1
	//cout << path1 << endl;

	char type = inode->finode.power[0];//获取文件类型

	if (type == 'd')//目录
	{
		if ((!strcmp(path, ".")==0) && (!strcmp(path, "..")==0))
		{
			int count = inode->finode.size / sizeof(struct direct);//地址项数目
			int addr = count / dirnum;
			if (count%dirnum >= 1)//有偏移量
				addr++;
			if (addr > 4)
				addr = 4;

			dir *dir = (struct dir*)calloc(1, sizeof(struct dir));
			bool find = false;//判断文件是否存在

			for (int i = 0; i < addr; i++)
			{
				bread(dir, inode->finode.arr[i], 0, sizeof(struct dir));//在各级地址项中进行查找
				for (int j = 0; j < dir->dirNum; j++)//遍历根据目录名进行查找
				{
					if (strcmp(dir->direct[j].directName, path1) == 0)//比较名称正确
					{
						find = true;
						struct inode* temple = iget(dir->direct[j].inodeID);//取出下一个文件的节点
						if (temple->finode.power[0] == '-')//下个文件地址对应是普通文件
						{
							cout << "无法移动到普通文件" << endl;
							return inode;
						}
						temple->parent = inode;
						inode = temple;
						break;
					}
				}
				if (find == true)
					break;
			}
			if (find == false)
			{
				cout << "未发现预跳转目录" << endl;
				return NULL;
			}
		}
		else if (strcmp(path, ".")==0)//跳转自身
		{
			inode = inode;
		}
		else if (strcmp(path, "..")==0)//跳转上层
		{
			inode = inode->parent;
		}

		if (more != -1 && inode != NULL)//有后续地址
		{
			substring(path, path1, more + 1, -1);
			return cd(path1, inode);//递归查找
		}
		else
			return inode;
	}
}

//显示当前目录中文件和子目录
int ls()
{
	char type = current->finode.power[0];//根据权限判断是否为目录
	if (type != 'd')
	{
		cout << "不是目录文件" << endl;
		return 0;
	}
	int count = current->finode.size/ sizeof(struct direct);
	dir * dir = (struct dir*)calloc(1, sizeof(struct dir));

	int addrnum = count / dirnum + (count%dirnum >= 1 ? 1 : 0);
	addrnum>4 ? addrnum = 4 : NULL;
	for (int addr = 0; addr<addrnum; addr++)
	{
		bread(dir, current->finode.arr[addr], 0, sizeof(struct dir));
		for (int i = 0; i<dir->dirNum; i++)//文件项数
			cout<<dir->direct[i].directName<<"  ";//输出文件名
		cout << endl;
	}
	return 1;
}

//把所有者所有组也显示出来
int lsl()
{
	char type = current->finode.power[0];//根据权限判断是否为目录
	if (type != 'd')
	{
		cout << "不是目录文件" << endl;
		return 0;
	}
	cout << "文件名" << "      " << "所有者" << "      " << "所有组" <<"       "<<"权限"<< "       " << "文件大小" << endl;
	int count = current->finode.size / sizeof(struct direct);
	dir * dir = (struct dir*)calloc(1, sizeof(struct dir));

	int addrnum = count / dirnum + (count%dirnum >= 1 ? 1 : 0);
	addrnum>4 ? addrnum = 4 : NULL;
	for (int addr = 0; addr<addrnum; addr++)
	{
		bread(dir, current->finode.arr[addr], 0, sizeof(struct dir));
		for (int i = 0; i < dir->dirNum; i++)//文件项数
		{
			inode* temple = iget(dir->direct[i].inodeID);
			cout << dir->direct[i].directName <<"     "<<temple->finode.owner<<"     "<<temple->finode.group << "     " <<temple->finode.power << "     " <<temple->finode.size<<endl;//输出文件详细信息
		}
	}
	return 1;
}

//创建目录
int mkdir(const char*dirname)
{
	if (current->finode.power[0] != 'd')
	{
		cout << "当前位置为不是目录" << endl;
		return -1;
	}
	
	dir * dir = (struct dir*)calloc(1, sizeof(struct dir));

	int count = current->finode.size/ sizeof(struct direct);//当前文件项数目
	if (count >= dirnum-1)
	{
		cout << "当前目录文件太多，别再增加了" << endl;
		return -1;
	}

	int addrnum = count / dirnum + (count%dirnum >= 1 ? 1 : 0);
	addrnum>4 ? addrnum = 4 : NULL;
	for (int addr = 0; addr < addrnum; addr++)//查看当前目录是否已经存在
	{
		bread(dir, current->finode.arr[addr], 0, sizeof(struct dir));
		for (int i = 0; i<dir->dirNum; i++)
			if (strcmp(dir->direct[i].directName, dirname) == 0)
			{
				cout.write(dirname, strlen(dirname));
				cout << "文件已存在" << endl;
				return -1;
			}
	}
	cout << "开始创建" << endl;
	current->finode.size += sizeof(struct direct);//当前目录加一个目录项

	int addrnum1 = count / dirnum;
	bread(dir, current->finode.arr[addrnum1], 0, sizeof(struct dir));//读取该目录最后一个块

	strcpy_s(dir->direct[dir->dirNum].directName, dirname);//最后一个目录项为新目录


	struct inode * newdocument = ialloc();//给新目录分配空闲节点
	newdocument->finode.arr[0] = balloc();//分配新的空间

	//newdocument->finode.power[0] = 'd';//设置权限
	strcpy_s(newdocument->finode.power, "drw-rw---");//设置权限
	newdocument->finode.power[9] = '-';
	strcpy_s(newdocument->finode.owner, currentuser.userName);//所有者
	strcpy_s(newdocument->finode.group, currentuser.userGroup);//所有组
	finodein(newdocument);

	dir->direct[dir->dirNum].inodeID = newdocument->inodeID;//写入索引号
	dir->dirNum += 1;//目录项数+1
	bwrite(dir, current->finode.arr[addrnum1], 0, sizeof(struct dir));//原目录文件修并写回

	finodein(current);//写回目录信息

	return 0;
}

//删除空目录
int rmdir(char*dirname)
{
	struct inode* temple=NULL;//存放找到的文件inode
	dir * dir = (struct dir*)calloc(1, sizeof(struct dir));

	int count = current->finode.size / sizeof(struct direct);//当前文件项数目

	int addrnum = count / dirnum + (count%dirnum >= 1 ? 1 : 0);
	addrnum>4 ? addrnum = 4 : NULL;
	bool find = false;
	for (int addr = 0; addr < addrnum; addr++)//查看当前目录是否已经存在
	{
		bread(dir, current->finode.arr[addr], 0, sizeof(struct dir));
		for (int i = 0; i<dir->dirNum; i++)
			if (strcmp(dir->direct[i].directName, dirname) == 0)//找到目录
			{
				temple= iget(dir->direct[i].inodeID);//获取要删除目录的索引号
				if (temple->finode.power[0] != 'd')
				{
					cout << "该文件不为目录" << endl;
					return -1;
				}
				else
				{
					if (temple->finode.size != 0)//非空
					{
						cout << "当前要删除目录不为空，请先删除子目录和文件" << endl;
						return -1;
					}
					else//找到空目录
					{
						find = true;
						for (int j = i; j < dir->dirNum; j++)
						{
							dir->direct[j] = dir->direct[j + 1];//前移，删除该目录项
						}
						dir->dirNum--;//目录数-1
						//cout << temple->inodeID<< endl;
						//cout << dir->dirNum << endl;
						//cout << "确实进行了删除"<<endl;
						bwrite(dir, current->finode.arr[addr], 0, sizeof(struct dir));
						count = -1;
						break;
					}
				}
			}
		if (find)
			break;
	}
	if (!find)
	{
		cout << "当前目录下没有该文件" << endl;
	}
	else
	{
		cout << "开始删除" << endl;
		bfree(temple->finode.arr[0]);
		temple->finode.filelink--;//文件链接计数-1
		finodein(temple);
		if(temple->finode.filelink==0)//没有文件链接则删除
			ifree(temple); 

		current->finode.size -= sizeof(struct direct);//当前目录减去一个目录项
		finodein(current);
	}
	return 0;
}

//更换文件所有者 chown ownername filename
int chown(char* command)//包括新所有者名和文件名
{
	char filename[directname] = { 0 };
	char ownername[maxname] = {0};
	int place = findpos(command,0,' ');//找到空字符位置
	substring(command,ownername,0,place);//截取所有者名
	substring(command,filename,place+1,-1);//截取文件名
	//cout << filename << endl;
	//cout << ownername << endl;

	struct inode* temple = NULL;//存放找到的文件inode
	dir * dir = (struct dir*)calloc(1, sizeof(struct dir));

	int count = current->finode.size / sizeof(struct direct);//当前文件项数目

	int addrnum = count / dirnum + (count%dirnum >= 1 ? 1 : 0);
	addrnum>4 ? addrnum = 4 : NULL;
	bool find = false;
	for (int addr = 0; addr < addrnum; addr++)//查看当前目录是否已经存在
	{
		bread(dir, current->finode.arr[addr], 0, sizeof(struct dir));
		for (int i = 0; i < dir->dirNum; i++)
			if (strcmp(dir->direct[i].directName, filename) == 0)//找到文件
			{
				temple = iget(dir->direct[i].inodeID);//获取要修改文件的索引号
				strcpy_s(temple->finode.owner, ownername);//修改所有者
				finodein(temple);
				find = true;
				break;
			}
		if (find)
			break;
	}
	if (!find)
	{
		cout << "未在该目录下找到文件" << filename << endl;
		return -1;
	}
	else 
	{
		cout << "文件" << filename << "拥有者修改为" << ownername << endl;
		return 0;
	}	
	return 0;
}

//更换文件所有组 chgrp groupname filename
int chgrp(char* command)//包括新所有组名和文件名
{
	char filename[directname] = { 0 };
	char groupname[maxname] = { 0 };
	int place = findpos(command, 0, ' ');//找到空字符位置
	substring(command,groupname , 0, place);//截取所有组名
	substring(command, filename, place + 1, -1);//截取文件名
	//cout << filename << endl;
	//cout << groupname << endl;

	struct inode* temple = NULL;//存放找到的文件inode
	dir * dir = (struct dir*)calloc(1, sizeof(struct dir));

	int count = current->finode.size / sizeof(struct direct);//当前文件项数目

	int addrnum = count / dirnum + (count%dirnum >= 1 ? 1 : 0);
	addrnum>4 ? addrnum = 4 : NULL;
	bool find = false;
	for (int addr = 0; addr < addrnum; addr++)//查看当前目录是否已经存在
	{
		bread(dir, current->finode.arr[addr], 0, sizeof(struct dir));
		for (int i = 0; i < dir->dirNum; i++)
			if (strcmp(dir->direct[i].directName, filename) == 0)//找到文件
			{
				temple = iget(dir->direct[i].inodeID);//获取要修改文件的索引号
				strcpy_s(temple->finode.group, groupname);//修改所有者
				finodein(temple);
				find = true;
				break;
			}
		if (find)
			break;
	}
	if (!find)
	{
		cout << "未在该目录下找到文件" << filename << endl;
		return -1;
	}
	else
	{
		cout << "文件" << filename << "拥有组修改为" << groupname << endl;
		return 0;
	}
	return 0;
}

//修改文件权限
int chmod(char*command)//全部的权限
{
	char filename[directname] = { 0 };
	char power[20] = { 0 };//多来点防止错误
	int place = findpos(command, 0, ' ');//找到空字符位置
	substring(command, power, 0, place);//截取权限
	substring(command, filename, place + 1, -1);//截取文件名
	if (strlen(power) != 9)
	{
		cout << "请输入相关文件的所有权限"<<endl;
		return -1;
	}
	for (int j = 0; j < strlen(power); j++)
	{
		if (power[j] != 'r' && power[j] != 'w' && power[j] != 'x'&&power[j]!='-')
		{
			cout << "请不要输入无关字符" << endl;
			return -1;
		}
	}

	struct inode* temple = NULL;//存放找到的文件inode
	dir * dir = (struct dir*)calloc(1, sizeof(struct dir));

	int count = current->finode.size / sizeof(struct direct);//当前文件项数目

	int addrnum = count / dirnum + (count%dirnum >= 1 ? 1 : 0);
	addrnum>4 ? addrnum = 4 : NULL;
	bool find = false;
	for (int addr = 0; addr < addrnum; addr++)//查看当前目录是否已经存在
	{
		bread(dir, current->finode.arr[addr], 0, sizeof(struct dir));
		for (int i = 0; i < dir->dirNum; i++)
			if (strcmp(dir->direct[i].directName, filename) == 0)//找到文件
			{
				temple = iget(dir->direct[i].inodeID);//获取要修改文件的索引号
				for (int i = 1; i <= 9; i++)
				{
					temple->finode.power[i] = power[i-1];	
				}
				finodein(temple);//写入
				find = true;
					break;		
			}
		if (find)
			break;
	}
	if (!find)
	{
		cout << "未在该目录下找到文件" << filename << endl;
		return -1;
	}
	else
	{
		cout << "文件" << filename << "权限修改为" << power << endl;
		return 0;
	}
	return 0;
}

//改变文件名 mv oldname newname
int mv(char* command)
{
	char oldname[directname] = { 0 };
	char newname[directname] = { 0 };//多来点防止错误
	int place = findpos(command, 0, ' ');//找到空字符位置
	substring(command, oldname, 0, place);//截取旧文件名
	substring(command, newname, place + 1, -1);//截取新文件名
	//cout << oldname << endl;
	//cout << newname << endl;

	dir * dir = (struct dir*)calloc(1, sizeof(struct dir));

	int count = current->finode.size / sizeof(struct direct);//当前文件项数目

	int addrnum = count / dirnum + (count%dirnum >= 1 ? 1 : 0);
	addrnum>4 ? addrnum = 4 : NULL;
	bool find = false;
	for (int addr = 0; addr < addrnum; addr++)//查看当前目录是否已经存在
	{
		bread(dir, current->finode.arr[addr], 0, sizeof(struct dir));
		for (int i = 0; i < dir->dirNum; i++)
			if (strcmp(dir->direct[i].directName, oldname) == 0)//找到文件
			{
				strcpy_s(dir->direct[i].directName,newname);
				bwrite(dir,current->finode.arr[addr],0,sizeof(struct dir));//写回目录盘块
				find = true;
				break;
			}
		if (find == true)
			break;
	}
	if (!find)
	{
		cout << "未在该目录下找到文件" << oldname << endl;
		return -1;
	}
	else
	{
		cout << "文件" << oldname << "名称修改为" << newname << endl;
		return 0;
	}
	return 0;
}

//创建文件(空)（我寻思可以用命令来输入文件内容）
int touch(char* dirname)//文件名 （之后考虑文件内容）
{
	dir * dir = (struct dir*)calloc(1, sizeof(struct dir));

	int count = current->finode.size / sizeof(struct direct);//当前文件项数目
	if (count >= dirnum - 1)
	{
		cout << "当前目录文件太多，别再增加了" << endl;
		return -1;
	}

	int addrnum = count / dirnum + (count%dirnum >= 1 ? 1 : 0);
	addrnum>4 ? addrnum = 4 : NULL;
	for (int addr = 0; addr < addrnum; addr++)//查看当前目录是否已经存在
	{
		bread(dir, current->finode.arr[addr], 0, sizeof(struct dir));
		for (int i = 0; i<dir->dirNum; i++)
			if (strcmp(dir->direct[i].directName, dirname) == 0)
			{
				cout.write(dirname, strlen(dirname));
				cout << "文件已存在" << endl;
				return -1;
			}
	}

	cout << "开始创建普通空文件" << endl;
	current->finode.size += sizeof(struct direct);//当前目录加一个目录项

	int addrnum1 = count / dirnum;
	bread(dir, current->finode.arr[addrnum1], 0, sizeof(struct dir));//读取该目录最后一个块

	strcpy_s(dir->direct[dir->dirNum].directName, dirname);//最后一个目录项为新目录


	struct inode * newdocument = ialloc();//给新目录分配空闲节点
	newdocument->finode.arr[0] = balloc();//分配新的空间

	//看看分配的索引号和盘块
	cout << newdocument->inodeID << endl;
	cout << newdocument->finode.arr[0]<< endl;

	strcpy_s(newdocument->finode.power, "-rw-rw---");//设置权限
	newdocument->finode.power[9] = '-';
	strcpy_s(newdocument->finode.owner, currentuser.userName);//所有者
	strcpy_s(newdocument->finode.group, currentuser.userGroup);//所有组
	finodein(newdocument);

	dir->direct[dir->dirNum].inodeID = newdocument->inodeID;//写入索引号
	dir->dirNum += 1;//目录项数+1
	bwrite(dir, current->finode.arr[addrnum1], 0, sizeof(struct dir));//原目录文件修并写回

	finodein(current);//写回目录信息

	return 0;

}

//删除文件
int rm(char* dirname)
{
	struct inode* temple = NULL;//存放找到的文件inode
	dir * dir = (struct dir*)calloc(1, sizeof(struct dir));

	int count = current->finode.size / sizeof(struct direct);//当前文件项数目
	
	int addrnum = count / dirnum + (count%dirnum >= 1 ? 1 : 0);
	addrnum>4 ? addrnum = 4 : NULL;
	bool find = false;
	for (int addr = 0; addr < addrnum; addr++)//查看当前目录是否已经存在
	{
		bread(dir, current->finode.arr[addr], 0, sizeof(struct dir));
		for (int i = 0; i<dir->dirNum; i++)
			if (strcmp(dir->direct[i].directName, dirname) == 0)//找到目录
			{
				temple = iget(dir->direct[i].inodeID);//获取要删除文件的索引号
				if (temple->finode.power[0] != '-')
				{
					cout << "该文件不为普通文件" << endl;
					return -1;
				}
				else
				{
					find = true;
					for (int j = i; j < dir->dirNum; j++)
					{
						dir->direct[j] = dir->direct[j + 1];//前移，删除该目录项
					}
					dir->dirNum--;//目录数-1;
					bwrite(dir, current->finode.arr[addr], 0, sizeof(struct dir));//所在目录文件处理完成
					count = -1;
					break;
				}
			}
		if (find)
			break;
	}
	if (!find)
	{
		cout << "当前目录下没有该文件" << endl;
	}
	else//找到该文件
	{
		cout << "开始删除" << endl;//主要这边有多个地址对应盘块要删除

		temple->finode.filelink--;
		finodein(temple);
		if (temple->finode.filelink == 0)
		{
			ifree(temple);
			//把所有盘块都free掉
			//temple->finode.size

			int bcount = temple->finode.size / blocksize + (temple->finode.size%blocksize > 0 ? 1 : 0);//计算文件内容占了多少盘块
			if (bcount == 0)
				bcount = bcount + 1;
			if (bcount <= 4)//只有直接寻址
			{
				for (int adrr = 0; adrr < bcount; adrr++)
				{
					bfree(temple->finode.arr[adrr]);//bfree所有直接寻址的盘块
				}
			}
			else
			{
				if (bcount <= 4 + maxbno)//一次寻址，4 + blocksize/sizeof(int)
				{
					for (int i = 0; i < 4; i++)
					{
						bfree(temple->finode.arr[i]);//bfree所有直接寻址的盘块
					}
					//算出剩下一级寻址盘块数
					int other = bcount - 4;

					//开辟盘块号空间，bread盘块号，全部free，之后free放盘块号的那个盘块
					struct bnoblock* bno = (struct bnoblock*)calloc(1, sizeof(struct bnoblock));
					int place = temple->finode.arr[4] * blocksize;//对应物理地址
					fseek(disk, place, SEEK_SET);
					int count = fread(bno, sizeof(int), maxbno, disk);//读取盘块号，最多128个

					for (int j = 0; j < other; j++)
					{
						bfree(bno->bno[j]);//依次释放盘块	
					}
				}
				else if (bcount > (4 + maxbno))//二次寻址，重复数次一次寻址
				{
					for (int i = 0; i < 4; i++)
					{
						bfree(temple->finode.arr[i]);//bfree所有直接寻址的盘块
					}
					//算出剩下一级寻址盘块数
					int other = bcount - 4;

					//先释放一次寻址中的盘块
					struct bnoblock* bno = (struct bnoblock*)calloc(1, sizeof(struct bnoblock));
					int place = temple->finode.arr[4] * blocksize;//对应物理地址
					fseek(disk, place, SEEK_SET);
					int count = fread(bno, sizeof(int), maxbno, disk);//读取盘块号，最多128个
					for (int j = 0; j < maxbno; j++)
					{
						bfree(bno->bno[j]);//依次释放盘块	
					}

					//释放二次寻址中的盘块
					other = other - maxbno;//剩下的盘块数
					//int sum = 0;
					int bcount2 = other / maxbno + (other % maxbno > 0 ? 1 : 0);

					struct bnoblock* bno2 = (struct bnoblock*)calloc(1, sizeof(struct bnoblock));//二次寻址存放后续盘块的盘块
					int place2 = temple->finode.arr[5] * blocksize;//对应物理地址
					fseek(disk, place2, SEEK_SET);
					fread(bno2, sizeof(int), maxbno, disk);//读取盘块号，最多128个

					for (int i = 0; i < bcount2 - 1; i++)//先处理bcount-1个盘块
					{
						struct bnoblock* bno = (struct bnoblock*)calloc(1, sizeof(struct bnoblock));
						int place = bno2->bno[i] * blocksize;//对应物理地址
						fseek(disk, place, SEEK_SET);
						int count = fread(bno, sizeof(int), maxbno, disk);//读取盘块号，最多128个
						for (int j = 0; j < maxbno; j++)
						{
							bfree(bno->bno[j]);//依次释放盘块	
						}
					}

					other -= (bcount2 - 1) * 128;//最后一个盘块记载的盘块数
					struct bnoblock* otherbno = (struct bnoblock*)calloc(1, sizeof(struct bnoblock));
					int otherplace = bno2->bno[bcount2 - 1] * blocksize;//对应物理地址
					fseek(disk, place, SEEK_SET);
					int othercount = fread(bno, sizeof(int), maxbno, disk);//读取盘块号，最多128个
					for (int o = 0; o < other; o++)
					{
						bfree(otherbno->bno[o]);
					}
				}
			}
			current->finode.size -= sizeof(struct direct);//当前目录减去一个目录项
			finodein(current);
			superin();//写入超级块
		}
	}
	return 0;
}

//增加用户
int adduser(const char* filename,user* user,int num)
{
	struct inode* temple = NULL;//存放找到的文件inode
	dir * dir = (struct dir*)calloc(1, sizeof(struct dir));

	int count = current->finode.size / sizeof(struct direct);//当前文件项数目

	int addrnum = count / dirnum + (count%dirnum >= 1 ? 1 : 0);
	addrnum>4 ? addrnum = 4 : NULL;
	bool find = false;
	for (int addr = 0; addr < addrnum; addr++)//查看当前目录是否已经存在
	{
		bread(dir, current->finode.arr[addr], 0, sizeof(struct dir));
		for (int i = 0; i < dir->dirNum; i++)
			if (strcmp(dir->direct[i].directName, filename) == 0)//找到目录
			{
				temple = iget(dir->direct[i].inodeID);//获取要删除文件的索引号
				if (temple->finode.power[0] != '-')
				{
					cout << "该文件不为普通文件" << endl;
					return -1;
				}
				else
				{
					find = true;
					temple= iget(dir->direct[i].inodeID);
				}
			}
		if (find)
			break;
	}
	if (!find)
	{
		cout << "未在该目录下找到文件"<< endl;
		return -1;
	}
	else
	{
		temple->finode.size += sizeof(struct user)*num;//增加num个用户结构的长度
		int bno=temple->finode.arr[0];
		struct user users[3];
		for (int i = 0; i < 3; i++)
			users[i] = user[i];
		bwrite(users,bno,0,sizeof(users));//写入磁盘
		finodein(temple);
	}
}

//初始化用户信息
int inituser()
{
	user u1;
	const char *group = "all";
	const char *name = "root";
	const char *pwd1 = "123456";
	strcpy_s(u1.userGroup, group);
	strcpy_s(u1.userName, name);
	strcpy_s(u1.userPwd, pwd1);

	user u2;
	const char *group2 = "all";
	const char *name2 = "jacky";
	const char *pwd12 = "123456";
	strcpy_s(u2.userGroup, group2);
	strcpy_s(u2.userName, name2);
	strcpy_s(u2.userPwd, pwd12);

	user u3;
	const char *group3 = "all";
	const char *name3 = "mage";
	const char *pwd13 = "123456";
	strcpy_s(u3.userGroup, group3);
	strcpy_s(u3.userName, name3);
	strcpy_s(u3.userPwd, pwd13);

	user users[3];
	users[0] = u1;
	users[1] = u2;
	users[2] = u3;

	adduser("user",users,3);
	return 0;
}

//查看文件信息
int cat(char*filename)//差不多，再写个用例
{
	struct inode* temple = NULL;//存放找到的文件inode
	dir * dir = (struct dir*)calloc(1, sizeof(struct dir));

	int count = current->finode.size / sizeof(struct direct);//当前文件项数目

	int addrnum = count / dirnum + (count%dirnum >= 1 ? 1 : 0);
	addrnum>4 ? addrnum = 4 : NULL;
	bool find = false;
	for (int addr = 0; addr < addrnum; addr++)//查看当前目录是否已经存在
	{
		bread(dir, current->finode.arr[addr], 0, sizeof(struct dir));
		for (int i = 0; i < dir->dirNum; i++)
			if (strcmp(dir->direct[i].directName, filename) == 0)//找到目录
			{
				temple = iget(dir->direct[i].inodeID);//获取要删除文件的索引号
				if (temple->finode.power[0] != '-')
				{
					cout << "该文件不为普通文件" << endl;
					return -1;
				}
				else
				{
					find = true;
					temple = iget(dir->direct[i].inodeID);
				}
			}
		if (find)
			break;
	}
	if (!find)
	{
		cout << "未在该目录下找到文件" << endl;
		return -1;
	}
	else//
	{
		int bno = temple->finode.arr[0];
		int place = 0;
		char contain[blocksize] = { 0 };//临时存放内容
		char* content = (char*)calloc(temple->finode.size,sizeof(char));//存放所有的内容
		
		int bcount = temple->finode.size / blocksize + (temple->finode.size%blocksize > 0 ? 1 : 0);//计算文件内容占了多少盘块
		if (bcount <= 4&&(bcount != 0))//只有直接寻址
		{
			for (int adrr = 0; adrr < bcount-1; adrr++)//前面放满的先取出来
			{
				bread(contain,temple->finode.arr[adrr],0,blocksize,1);
				for (int i = 0; i < blocksize; i++)
				{
					content[place] = contain[i];//将contain数组中的内容复制到content中，方便之后输出
					place++;
				}
			}
			bread(contain,temple->finode.arr[bcount-1],0,temple->finode.size-(bcount-1)*blocksize);//读取在最后不满的盘块内容
			for (int k = 0; k < temple->finode.size - (bcount - 1)*blocksize; k++)
			{
				content[place]=contain[k];
				place++;
			}
			//输出
			for (int i = 0; i < temple->finode.size; i++)
			{
				if ((int)content[i] < 0)//有为-2的占位符，不输出
					cout << " ";
				else
					_putch(content[i]);
			}
			cout << endl;
		}
		else if(bcount > 4 && (bcount != 0))//一级寻址
		{
			for (int adrr = 0; adrr < 4; adrr++)
			{
				bread(contain, temple->finode.arr[adrr], 0, blocksize, 1);
				for (int i = 0; i < blocksize; i++)
				{
					content[place] = contain[i];
					place++;
				}
			}
			//还没写完
			bcount = bcount - 4;//后续盘块数量

			struct bnoblock* bno = (struct bnoblock*)calloc(1, sizeof(struct bnoblock));
			int place1 = temple->finode.arr[4] * blocksize;//对应物理地址
			fseek(disk, place1, SEEK_SET);
			
			int count = fread(bno, sizeof(int), maxbno, disk);//读取盘块号，最多128个
			for (int b = 0; b < bcount-1; b++)//除了最后一个都是512字节
			{
				bread(contain, bno->bno[b], 0, blocksize, 1);
				for (int i = 0; i < blocksize; i++)
				{
					content[place] = contain[i];
					place++;
				}
			}
			int other = temple->finode.size - (bcount - 1)*blocksize-4 * blocksize;//同时减去直接寻址盘块
			bread(contain, bno->bno[bcount-1], 0, other, 1);
			for (int i = 0; i < other; i++)//读取最后一个盘块信息
			{
				content[place] = contain[i];
				place++;
			}
			//输出
			for (int i = 0; i < temple->finode.size; i++)
			{
				if ((int)content[i] < 0)//有为-2的占位符，不输出
					cout << " ";
				else
					_putch(content[i]);
			}
			cout << endl;
		}
	}
	return 0;
}


//往空文件第一个盘块加点内容
int addcontent(char* command)
{
	char filename[directname] = { 0 };
	char content[200] = { 0 };//多来点防止错误
	int place = findpos(command, 0, ' ');//找到空字符位置
	substring(command, filename, 0, place);//截取文件名
	substring(command, content, place + 1, -1);//截取插入内容
	cout << strlen(content) << endl;
	cout << filename << endl;
	cout << content << endl;

	struct inode* temple = NULL;//存放找到的文件inode
	dir * dir = (struct dir*)calloc(1, sizeof(struct dir));

	int count = current->finode.size / sizeof(struct direct);//当前文件项数目

	int addrnum = count / dirnum + (count%dirnum >= 1 ? 1 : 0);
	addrnum>4 ? addrnum = 4 : NULL;
	bool find = false;
	for (int addr = 0; addr < addrnum; addr++)//查看当前目录是否已经存在
	{
		bread(dir, current->finode.arr[addr], 0, sizeof(struct dir));
		for (int i = 0; i < dir->dirNum; i++)
			if (strcmp(dir->direct[i].directName, filename) == 0)//找到文件
			{
				temple = iget(dir->direct[i].inodeID);//获取要删除文件的索引号
				if (temple->finode.power[0] != '-')
				{
					cout << "该文件不为普通文件" << endl;
					return -1;
				}
				else
				{
					temple = iget(dir->direct[i].inodeID);//获取要修改文件的索引号
					temple->finode.size = strlen(content);//修改文件大小
					finodein(temple);
					find = true;
					break;
				}
			}
		if (find)
			break;
	}
	if (!find)
	{
		cout << "未在该目录下找到文件" << filename << endl;
		return -1;
	}
	else
	{
		bwrite(content, temple->finode.arr[0], 0, strlen(content));//写入盘块
		cout << "写入文件完成" << endl;
	}
	return 0;
}

//大文件，用于测试
int bigfile()
{
	//节点号9 记得把size改了
	inode* temple = iget(9);//根节点下的bigfile文件
	for (int i = 1; i < 4; i++)//给直接盘块全搞满
	{
		temple->finode.arr[i] = balloc();//分就完事了
		cout << temple->finode.arr[i] << endl;
	}
	char content[blocksize];
	for(int i=0;i<blocksize;i++)
		content[i] ='a';//一个盘块的a
	for(int k=0;k<4;k++)
		bwrite(content, temple->finode.arr[k], 0, blocksize);

	//往一级索引塞东西
	temple->finode.arr[4]=balloc();
	int bno = balloc();//放到一级索引对应盘块中
	bwrite(&bno,temple->finode.arr[4],0,sizeof(int));
	bwrite(content, bno, 0, blocksize);//第五个盘块塞满

	temple->finode.size = blocksize * 5;
	finodein(temple);
	return 0;
}

//硬链接
int ln(char* command)//原文件 目标文件
{
	char filename[directname] = {0};//原文件名
	char target[200] = {0};//目标位置和新文件名
	int place = findpos(command, 0, ' ');//找到空字符位置
	substring(command, filename, 0, place);//截取旧文件名
	substring(command, target, place + 1, -1);//截取新文件名和地址
		//cout <<filename<<endl;
		//cout << target << endl;

	struct inode* temple = NULL;//存放找到的文件inode
	dir * dir = (struct dir*)calloc(1, sizeof(struct dir));

	int count = current->finode.size / sizeof(struct direct);//当前文件项数目

	int addrnum = count / dirnum + (count%dirnum >= 1 ? 1 : 0);
	addrnum>4 ? addrnum = 4 : NULL;
	bool find = false;
	for (int addr = 0; addr < addrnum; addr++)//查看当前目录是否已经存在
	{
		bread(dir, current->finode.arr[addr], 0, sizeof(struct dir));
		for (int i = 0; i < dir->dirNum; i++)
			if (strcmp(dir->direct[i].directName, filename) == 0)//找到目录
			{
				temple = iget(dir->direct[i].inodeID);//获取要链接文件的索引号
				if (temple->finode.power[0] != '-')
				{
					cout << "该文件不为普通文件" << endl;
					return -1;
				}
				else
				{
					find = true;
					temple = iget(dir->direct[i].inodeID);
				}
			}
		if (find)
			break;
	}
	if (!find)
	{
		cout << "未在该目录下找到文件" << endl;
		return -1;
	}
	else
	{
		//开始链接
		//用cd返回位置，同时判断是否已经存在
		//创建增加文件目录项
		inode* targetinode = NULL;
		root->parent = root;
		if (target[0] == '/')//绝对地址
			targetinode = root;
		else
			targetinode = current;
		int lastplace;
		for (int i = strlen(target); i > 0; i--)//寻找最后一个/的位置来分割文件名
		{
			if (target[i] == '/')
			{
				lastplace = i;
				break;
			}
		}
		char newfilename[directname] = { 0 };
		substring(target,newfilename,lastplace+1,-1);//新文件名
		substring(target,target,0,lastplace);//跳转地址
			//cout << newfilename << endl;
			//cout << target<<endl;
		targetinode = cd(target,targetinode);//跳转
		if (targetinode == NULL)
		{
			cout << "链接失败" << endl;
			return -1;
		}
		else//开始增加目录项
		{
			struct dir * targetdir = (struct dir*)calloc(1, sizeof(struct dir));
			int targetcount = targetinode->finode.size / sizeof(struct direct);//目标目录文件项数目
			int targetaddrnum = targetcount / dirnum + (count%dirnum >= 1 ? 1 : 0);
			targetaddrnum>4 ? targetaddrnum = 4 : NULL;
			for (int addr = 0; addr <targetaddrnum; addr++)//查看当前目录是否已经存在
			{
				bread(targetdir, targetinode->finode.arr[addr], 0, sizeof(struct dir));
				for (int i = 0; i<targetdir->dirNum; i++)
					if (strcmp(targetdir->direct[i].directName,newfilename) == 0)
					{
						cout << newfilename << endl;
						cout << "文件已存在" << endl;
						return -1;
					}
			}
			int addrnum1 = count / dirnum;
			bread(targetdir, targetinode->finode.arr[addrnum1], 0, sizeof(struct dir));//读取目标目录

			targetinode->finode.size += sizeof(struct direct);//目标目录加一个目录项
			strcpy_s(targetdir->direct[targetdir->dirNum].directName, newfilename);//最后一个目录项
			targetdir->direct[targetdir->dirNum].inodeID = temple->inodeID;//链接索引号
			targetdir->dirNum += 1;
			bwrite(targetdir,targetinode->finode.arr[addrnum1],0,sizeof(struct dir));//写入最后一个盘块

			temple->finode.filelink++;
			finodein(temple);//更新索引节点信息
		}
	}
	cout << "文件" << filename << "和链接文件" << target << "链接成功" << endl;
	return 0;
}

//文件复制
int cp(char* command)
{
	char filename[directname] = { 0 };//原文件名
	char target[200] = { 0 };//目标位置和新文件名
	int place = findpos(command, 0, ' ');//找到空字符位置
	substring(command, filename, 0, place);//截取旧文件名
	substring(command, target, place + 1, -1);//截取地址
											  //cout <<filename<<endl;
											  //cout << target << endl;

	struct inode* temple = NULL;//存放找到的文件inode
	dir * dir = (struct dir*)calloc(1, sizeof(struct dir));

	int count = current->finode.size / sizeof(struct direct);//当前文件项数目

	int addrnum = count / dirnum + (count%dirnum >= 1 ? 1 : 0);
	addrnum>4 ? addrnum = 4 : NULL;
	bool find = false;
	for (int addr = 0; addr < addrnum; addr++)//查看当前目录是否已经存在
	{
		bread(dir, current->finode.arr[addr], 0, sizeof(struct dir));
		for (int i = 0; i < dir->dirNum; i++)
			if (strcmp(dir->direct[i].directName, filename) == 0)//找到目录
			{
				temple = iget(dir->direct[i].inodeID);//获取要删除文件的索引号
				if (temple->finode.power[0] != '-')
				{
					cout << "该文件不为普通文件" << endl;
					return -1;
				}
				else
				{
					find = true;
					temple = iget(dir->direct[i].inodeID);
				}
			}
		if (find)
			break;
	}
	if (!find)
	{
		cout << "未在该目录下找到文件" << endl;
		return -1;
	}
	else//开始复制
	{
		cout << filename << endl;
		cout << target << endl;

		inode* targetinode = NULL;//目标目录inode
		root->parent = root;
		if (target[0] == '/')//绝对地址
			targetinode = root;
		else
			targetinode = current;

		targetinode = cd(target, targetinode);//跳转
		if (targetinode == NULL)
		{
			cout << "查找目录失败" << endl;
			return -1;
		}
		else//开始增加目录项
		{
			struct dir * targetdir = (struct dir*)calloc(1, sizeof(struct dir));
			int targetcount = targetinode->finode.size / sizeof(struct direct);//目标目录文件项数目
			int targetaddrnum = targetcount / dirnum + (count%dirnum >= 1 ? 1 : 0);
			targetaddrnum > 4 ? targetaddrnum = 4 : NULL;
			for (int addr = 0; addr < targetaddrnum; addr++)//查看当前目录是否已经存在
			{
				bread(targetdir, targetinode->finode.arr[addr], 0, sizeof(struct dir));
				for (int i = 0; i < targetdir->dirNum; i++)
					if (strcmp(targetdir->direct[i].directName, filename) == 0)
					{
						cout << filename << endl;
						cout << "文件已存在" << endl;
						return -1;
					}
			}
			int addrnum1 = count / dirnum;
			bread(targetdir, targetinode->finode.arr[addrnum1], 0, sizeof(struct dir));//读取目标目录

			targetinode->finode.size += sizeof(struct direct);//目标目录加一个目录项
			strcpy_s(targetdir->direct[targetdir->dirNum].directName, filename);//最后一个目录项
			struct inode* newinode = ialloc();//新文件inode
			targetdir->direct[targetdir->dirNum].inodeID =newinode->inodeID;//链接索引号
			targetdir->dirNum += 1;
			bwrite(targetdir, targetinode->finode.arr[addrnum1], 0, sizeof(struct dir));//写入最后一个盘块

			//复制索引节点内容
			newinode->finode.size = temple->finode.size;
			for (int i = 0; i < 10; i++)//复制文件权限
			{
				newinode->finode.power[i] = temple->finode.power[i];
			}
			for (int i = 0; i < maxname; i++)//复制拥有者和所有组
			{
				newinode->finode.owner[i] = temple->finode.owner[i];
				newinode->finode.group[i] = temple->finode.group[i];
			}

			//处理盘块内容
			int bcount = temple->finode.size / blocksize + (temple->finode.size%blocksize > 0 ? 1 : 0);//计算文件内容占了多少盘块
			if (bcount == 0)
				bcount = bcount + 1;
			if (bcount <= 4)//只有直接寻址
			{
				for (int adrr = 0; adrr < bcount; adrr++)
				{
					newinode->finode.arr[adrr] = balloc();//分配新盘块
					char contain[blocksize] = { 0 };//临时存放内容
					bread(contain,temple->finode.arr[adrr],0,blocksize);//原内容
					bwrite(contain,newinode->finode.arr[adrr],0,blocksize);//新盘块写入
				}
			}
			else if (bcount <= 4 + maxbno)//一次寻址，4 + blocksize/sizeof(int)
			{
				for (int adrr = 0; adrr < 4; adrr++)//先把直接的都复制了
				{
					newinode->finode.arr[adrr] = balloc();//分配新盘块
					char contain[blocksize] = { 0 };//临时存放内容
					bread(contain, temple->finode.arr[adrr], 0, blocksize);//原内容
					bwrite(contain, newinode->finode.arr[adrr], 0, blocksize);//新盘块写入
				}
				//算出剩下一级寻址盘块数
				int other = bcount - 4;

				//开辟盘块号空间
				struct bnoblock* bno = (struct bnoblock*)calloc(1, sizeof(struct bnoblock));//一级盘块号
				int place = temple->finode.arr[4] * blocksize;//对应物理地址
				fseek(disk, place, SEEK_SET);
				int count = fread(bno, sizeof(int), maxbno, disk);//读取盘块号，最多128个

				newinode->finode.arr[4] = balloc();//一级寻址
				
				struct bnoblock* newbno = (struct bnoblock*)calloc(1, sizeof(struct bnoblock));//一级盘块号
				for (int i = 0; i < other; i++)
				{
					newbno->bno[i] = balloc();
				}
				bwrite(newbno, newinode->finode.arr[4], 0, sizeof(struct bnoblock));//一级寻址盘块写入
				for (int i = 0; i < other; i++)
				{
					char contain[blocksize] = { 0 };//临时存放内容
					bread(contain, bno->bno[i], 0, blocksize);//原内容
					bwrite(contain,newbno->bno[i],0,blocksize);//写入新盘块
				}
			}
			finodein(newinode);
		}
	}
	cout << "文件" << filename << "复制为文件" << target <<"/"<<filename<< endl;
	return 0;
}

//帮助表
void help(char*command)
{
	//cout << "没事不要help"<<endl;
	cout << "ls: 显示当前目录下文件名 输入ls-l显示详细信息"<<endl;
	cout << "chmod: 修改权限，输入为九个权限字符和当前目录下文件名" << endl;
	cout << "chown: 修改所有者，输入为新所有者名和当前目录下文件名" << endl;
	cout << "chgrp: 修改所有组，输入为新所有组名和当前目录下文件名" << endl;
	cout << "pwd: 显示当前所在目录绝对地址" << endl;
	cout << "cd: 输入想要跳转的目录地址，输入错误会留在原地" << endl;
	cout << "mkdir: 输入想要创建的目录名，在当前目录下创建对应空目录" << endl;
	cout << "rmdir: 输入想要删除的空目录名，当前目录下存在且为空则删除" << endl;
	cout << "mv: 输入想要改名的原文件和新文件名即可改名" << endl;
	cout << "cp: 输入想复制的原文件和目标目录即可在目标目录创建新文件" << endl;
	cout << "rm: 删除当前目录下普通文件，输入文件名即可" << endl;
	cout << "ln: 硬链接，输入原文件名和目标新文件创建链接，不能链接目录文件" << endl;
	cout << "cat: 显示文件内容，输入当前目录下文件名即可" << endl;
	cout << "passwd: 修改当前用户密码" << endl;
	cout<<"touch: 创建空文件"<<endl;
}

//获取命令
int getcommand()
{
	cout << "[ ";
	cout << currentuser.userName << "  @loaclhost :$";
	char command[2000] = {0};//前置命令
	char enter[2000] = {0};//总输入
	char **com = new char*[5];
	char ch;
	int i = 0;

	ch = getchar();//获取输入
	if (ch == 10)//为回车则返回
		return 0;
	while (ch != 10)//不为回车
	{
		enter[i] = ch;//继续读取
		ch = getchar();
		i++;
	}
	strcpy_s(command,enter);
	strtok_s(command," ",com);//第一个空格进行切断
	//cout << strlen(enter)<<endl;
	//cout << strlen(command)<<endl;

	if (strcmp(command, "cd") == 0)
	{
		root->parent = root;
		inode* temple;
		strCpy(command,enter,strlen(command)+1);//获得后续命令command
		if (command[0] == '/')
			temple = root;//从根目录跳转
		else
			temple = current;//从当前目录跳转
		temple = cd(command, temple);
		if (temple != NULL)
			current =temple;
		cout << "当前位置为:";
		pwd();
	}
	/*else if (strcmp(command,"inituser") == 0)//用来初始化用户
	{
		inituser();
	}*/
	/*else if (strcmp(command, "bigfile") == 0)
	{
		bigfile();
	}*/
	else if (strcmp(command,"passwd")==0 )
	{
		passwd();
	}
	else if (strcmp(command, "super") == 0)
	{
		superInfo();
	}
	else if (strcmp(command, "ls") == 0)
	{
		ls();
	}
	else if (strcmp(command, "ls-l") == 0)
	{
		lsl();
	}
	else if (strcmp(command, "pwd") == 0)
	{
		pwd();
	}
	else if (strcmp(command, "mkdir") == 0)
	{
		strCpy(command, enter, strlen(command) + 1);//获得后续命令command
		if (strlen(command) <= 13)
			mkdir(command);
		else
			cout << "目录名过大" << endl;
	}
	else if (strcmp(command,"rmdir")==0)
	{
		strCpy(command, enter, strlen(command) + 1);//获得后续命令command
		if (strlen(command) <= 13)
			rmdir(command);
		else
			cout << "目录名过大" << endl;
	}
	else if (strcmp(command, "touch") == 0)
	{
		strCpy(command, enter, strlen(command) + 1);//获得后续命令command
		if (strlen(command) <= 13)
			touch(command);
		else
			cout << "目录名过大" << endl;
	}
	else if (strcmp(command, "cat") == 0)
	{
		strCpy(command, enter, strlen(command) + 1);//获得后续命令command
		if (strlen(command) <= 13)
			cat(command);
		else
			cout << "目录名过大" << endl;
	}
	else if (strcmp(command,"chown")==0)
	{
		strCpy(command, enter, strlen(command) + 1);//获得后续命令command
		chown(command);
	}
	else if (strcmp(command, "chgrp") == 0)
	{
		strCpy(command, enter, strlen(command) + 1);//获得后续命令command
		chgrp(command);
	}
	else if (strcmp(command, "chmod") == 0)
	{
		strCpy(command, enter, strlen(command) + 1);//获得后续命令command
		chmod(command);
	}
	else if (strcmp(command, "ln") == 0)
	{
		strCpy(command, enter, strlen(command) + 1);//获得后续命令command
		ln(command);
	}
	else if (strcmp(command, "cp") == 0)
	{
		strCpy(command, enter, strlen(command) + 1);//获得后续命令command
		cp(command);
	}
	else if (strcmp(command, "mv") == 0)
	{
		strCpy(command, enter, strlen(command) + 1);//获得后续命令command
		mv(command);
	}
	else if (strcmp(command, "add") == 0)
	{
		strCpy(command,enter,strlen(command)+1 );
		addcontent(command);
	}
	else if (strcmp(command, "rm") == 0)
	{
		strCpy(command, enter, strlen(command) + 1);//获得后续命令command
		if (strlen(command) <= 13)
			rm(command);
		else
			cout << "目录名过大" << endl;
	}
	else if (strcmp(command, "exit") == 0)
	{
		commandover = true;
	}
	else if (strcmp(command, "help") == 0)
	{
		if (strlen(enter) != 4)//有后续	
		{
			strCpy(command, enter, strlen(command) + 1);//获得后续命令command
			help(command);
		}
		else
		{
			help(command);
		}
	}
	return 0;
}