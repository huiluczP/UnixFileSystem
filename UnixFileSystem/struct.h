#pragma once
#define maxfreeblock 20
#define maxfreeinode 20
#define maxname 20
#define maxgroupname 20
#define directname 14
#define dirnum 31//512/16-1
#define maxbno 128  //512/sizeof(int)

struct superblock//超级块
{
	unsigned int blocknum;//盘块块数
	unsigned int freeblocknum;//磁盘中空闲块数目
	unsigned int nextfreeinode;//栈中可用空闲节点
	unsigned int freeblock[maxfreeblock];//空闲块栈,数组代替
	unsigned int freeinodenum;//磁盘中空闲节点块数目
	unsigned int nextfreeblock;//栈中可用空闲块
	unsigned int freeinode[maxfreeinode];//空闲i节点栈，数组代替
	long int modifytime;//最后修改时间
	short int dirt;//修改标志
};

struct finode//磁盘i节点 88字节
{
	long int size;//文件大小
	int filelink;//文件连接计数
	int arr[6];//文件地址 四个直接块号 一个一次 一个两次
	char owner[maxname];//文件主名
	char group[maxgroupname];//文件组名
	long int modifytime;//文件修改时间
	char power[10];//文件存取权限和类型
};

struct inode//内存i节点
{
	struct finode finode;//对应索引节点
	unsigned short int inodeID;//内存i节点编号
	int usercount;//访问计数 用户打开该文件数量
	struct inode*parent;//指向父节点 用来确定文件层次
};

struct direct//文件目录项
{
	char directName[directname];//文件名 14字节
	unsigned short int inodeID;//i节点号 2字节
};

struct dir//目录结构
{
	int	dirNum;//目录数目
	struct	direct direct[dirnum];//目录数组
};

struct user
{
	char userName[maxname];//用户名
	char userPwd[maxname];//用户密码
	char userGroup[maxgroupname];//用户组
};

struct bnoblock
{
	int bno[maxbno];
};

struct test
{
	char content[512];
};

