#pragma once
#define maxfreeblock 20
#define maxfreeinode 20
#define maxname 20
#define maxgroupname 20
#define directname 14
#define dirnum 31//512/16-1
#define maxbno 128  //512/sizeof(int)

struct superblock//������
{
	unsigned int blocknum;//�̿����
	unsigned int freeblocknum;//�����п��п���Ŀ
	unsigned int nextfreeinode;//ջ�п��ÿ��нڵ�
	unsigned int freeblock[maxfreeblock];//���п�ջ,�������
	unsigned int freeinodenum;//�����п��нڵ����Ŀ
	unsigned int nextfreeblock;//ջ�п��ÿ��п�
	unsigned int freeinode[maxfreeinode];//����i�ڵ�ջ���������
	long int modifytime;//����޸�ʱ��
	short int dirt;//�޸ı�־
};

struct finode//����i�ڵ� 88�ֽ�
{
	long int size;//�ļ���С
	int filelink;//�ļ����Ӽ���
	int arr[6];//�ļ���ַ �ĸ�ֱ�ӿ�� һ��һ�� һ������
	char owner[maxname];//�ļ�����
	char group[maxgroupname];//�ļ�����
	long int modifytime;//�ļ��޸�ʱ��
	char power[10];//�ļ���ȡȨ�޺�����
};

struct inode//�ڴ�i�ڵ�
{
	struct finode finode;//��Ӧ�����ڵ�
	unsigned short int inodeID;//�ڴ�i�ڵ���
	int usercount;//���ʼ��� �û��򿪸��ļ�����
	struct inode*parent;//ָ�򸸽ڵ� ����ȷ���ļ����
};

struct direct//�ļ�Ŀ¼��
{
	char directName[directname];//�ļ��� 14�ֽ�
	unsigned short int inodeID;//i�ڵ�� 2�ֽ�
};

struct dir//Ŀ¼�ṹ
{
	int	dirNum;//Ŀ¼��Ŀ
	struct	direct direct[dirnum];//Ŀ¼����
};

struct user
{
	char userName[maxname];//�û���
	char userPwd[maxname];//�û�����
	char userGroup[maxgroupname];//�û���
};

struct bnoblock
{
	int bno[maxbno];
};

struct test
{
	char content[512];
};

