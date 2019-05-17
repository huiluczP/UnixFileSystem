#pragma once
#include"struct.h"
#include<iostream>
#include <time.h>
#include <string>
#include <vector>
#include <conio.h>
#include<stack>
using namespace std;

#define blocknum 150  //�̿���
#define blocksize 512 //�̿��С
#define finodeblocknum  20//�����ڵ��̿���
#define finodenum 110
#define fileblockstart 22// finodeblocknum+2

struct inode* root;//�ڴ���ڵ������ڵ�
struct superblock* super;//��������Ϣ
FILE* disk;//ģ��Ĵ��̣�����Ϊ�ı��ļ�
struct inode usedinode[blocknum];// ���д洢�ڴ��е������ڵ�
struct user currentuser;
struct inode* current;//��ǰĿ¼λ�ö�Ӧi�ڵ�
bool commandover = false;//�����˳�

//д�볬����
int superin()
{
	time_t timer;
	time(&timer);
	super->modifytime = timer;//�޸�ʱ����Ϊ��ǰ
	super->dirt = 0;
	fseek(disk, blocksize, SEEK_SET);//�ƶ��ļ�ָ�뵽�̿�#1
	int write = fwrite(super, sizeof(struct superblock), 1, disk);
	fflush(disk);//ˢ���ļ�����
	if (write != 1)
	{
		cout << "������д��ʧ��"<<endl;
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

//��ʼ�����̣�д�볬����ͱ�Ҫ������Ϣ
int initialize(const char* path)
{
	 fopen_s(&disk, path, "rb+");
	if (disk== NULL)
	{
		return -1;
	}
	cout << "1";
	
	super = (struct superblock*)calloc(1, sizeof(struct superblock));	//���䳬�������ռ�
	int t=fseek(disk, blocksize, SEEK_SET);//ʹ��#1�̿飬#0����
	//cout << t << "ttt" << endl;
	fread(super, sizeof(struct superblock), 1, disk);		//���������������

	for (int i = 0; i < maxfreeblock; i++)//��ʼ�������̿�ջ
	{
		super->freeblock[i] = 2 + finodeblocknum + i;//�̿�0��1Ϊ���ÿ�ͳ����飬��ʮ��inode�̿�
	}
	super->nextfreeblock = maxfreeblock;//ջ��

	for (int i = 0; i<maxfreeinode; i++)//��ʼ�����нڵ�ջ
	{
		super->freeinode[i] = i;//���������Ŵ�0��ʼ
	}
	super->nextfreeinode = maxfreeinode;

	super->freeblocknum = blocknum - 2 - finodeblocknum;//�����ļ��̿�
	super->freeinodenum = finodenum;//���ÿ��нڵ���Ŀ

	//cout << "2";
	superin();
	return 1;
}

//�������ӷ������̿���Ϣ
int formatting(const char * path)
{
	fopen_s(&disk,path, "rb+");
	if (disk == NULL)
	{
		cout << "�Ҳ�������"<<endl;
		//return -1;
	}
	fseek(disk, fileblockstart * blocksize, SEEK_SET);//ָ���ļ��̿�λ��
	unsigned int group[maxfreeblock];//������ת��20��һ��
	for (int i = 0; i<maxfreeblock; i++)
	{
		group[i] = i + fileblockstart;
	}
	for (int i = 0; i<(blocknum-2-finodeblocknum)/20; i++)//�����̿����
	{
		for (int j = 0; j<maxfreeblock; j++)
		{
			group[j] += maxfreeblock;
			cout << group[j] << " ";
		}
		cout << endl;
		fseek(disk, (fileblockstart + i * maxfreeblock) * blocksize, SEEK_SET);	//ÿ���̿������һ���̿鿪ͷ���Ǹ��������̿���Ϣ
		fwrite(group, sizeof(unsigned int), maxfreeblock, disk);//�������ӷ���ʼ��
	}
	return 1;
}

//ϵͳ���� ���̼��� ��ȡ��������Ϣ
int superload(const char* path)
{
	 fopen_s(&disk,path,"rb+");//��д��ʽ���ļ��������
	if (disk == NULL)
	{
		cout << "�����ļ�������"<<endl;
		return 0;
	}
	super = (struct superblock*)calloc(1, sizeof(struct superblock));//���ٿռ��������ṹ
	fseek(disk,blocksize,SEEK_SET);//�ƶ��ļ�ָ�뵽�̿�#1
	int read = fread(super,sizeof(struct superblock),1,disk);//��ȡ��������
	if (read != 1)
	{
		cout << "�������ȡʧ��"<<endl;
		return 0;
	}
	return 1;
}

//�����������Ϣ
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

//��������ڵ���Ϣ
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

//inodeд�����
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
		cout << "����д��inodeʧ��"<<endl;
		return 0;
	}
	return 1;
}


//�����ڴ�i�ڵ�
struct inode *iget(int ino)//����Ϊ��Ӧ�����ڵ���
{
	int place;
	if (usedinode[ino].usercount != 0)//�ýڵ㱻ʹ�ù������ڴ����м�¼
	{
		usedinode[ino].usercount++;
		return &usedinode[ino];
	}

	if (disk == NULL)
		return NULL;

	place = blocksize + blocksize + ino * sizeof(struct finode);//#0+#1+��Ӧ������*�����ڵ��С=�����ڵ�λ��
	fseek(disk,place,SEEK_SET);
	int read = fread(&usedinode[ino],sizeof(struct finode),1,disk);
	if (read != 1)
	{
		cout << "�ڴ���������ʧ��"<<endl;
		return NULL;
	}

	if (usedinode[ino].finode.filelink == 0)//�ýڵ�δ���ļ�link�� ���������ļ�
	{	
		//�����ڵ��ʼ��
		usedinode[ino].finode.filelink++;
		usedinode[ino].finode.size = 0;
		usedinode[ino].inodeID = ino;
		time_t timer;
		time(&timer);
		usedinode[ino].finode.modifytime = timer;
		super->freeinodenum--;//�ܵĴ��̿���inode����
		finodein(&usedinode[ino]);//д�����
	}
	
	usedinode[ino].usercount++;//�޸��ڴ������ڵ���Ϣ
	usedinode[ino].inodeID=ino;

	return &usedinode[ino];//�����ڴ������ڵ�
}

//�ڵ����
struct inode* ialloc()
{
	int ino;
	if (super->nextfreeinode != 0)//�������������ջ��Ϊ��
	{
		ino = super->freeinode[--(super->nextfreeinode)];//��ǰջ���ó�����,ͬʱʣ��ջ�ڵ�-1
		super->dirt = 1;
		superin();//Ѱ˼ֱ�Ӱ�superд������
		return iget(ino);
	}
	else//����ջΪ��ʱ
	{
		finode *tmple = (struct finode*)calloc(1, sizeof(struct finode));
		fseek(disk,blocksize+blocksize,SEEK_SET);
		int num=0;
		for (int i = 0; i < finodenum; i++)//�������������ڵ���
		{
			fread(tmple,sizeof(struct finode),1,disk);//����ջ���µĽڵ�����
			if (tmple->filelink == 0)
			{
				super->freeinode[num] = i;
				super->nextfreeinode++;//ջ�п���
				num++;
			}
			if (num == maxfreeinode)
				break;
		}
		if (super->nextfreeinode == 0)
			return NULL;

		ino = super->freeinode[--(super->nextfreeinode)];//��ǰջ���ó�����
		super->dirt = 1;
		superin();//Ѱ˼ֱ�Ӱ�superд������
		return iget(ino);
	}
}


//�ڵ����
int ifree(struct inode* inode)
{
	if (super->nextfreeinode != 20)//����ջδ�������ջ
	{
		super->freeinode[super->nextfreeinode] = inode->inodeID;
		super->nextfreeinode++;
	}

	inode->usercount = 0;
	finodein(inode);

	super->freeinodenum++;
	super->dirt = 1;
	superin();//д��
	return 1;
}

//д�̿� �����̿��
int bwrite(void * _Buf, unsigned short int bno, long int offset, int size, int count = 1)//������ �̿�� ƫ���� ���ݴ�С Ԫ�ظ���
{
	long place;
	int result;
	place = bno * blocksize + offset;//��Ӧ�����ַ
	fseek(disk, place, SEEK_SET);
	result = fwrite(_Buf, size, count, disk);//����д��Ԫ�ظ���
	fflush(disk);
	if (result != count)
	{
		cout << "�̿�д��ʧ��" << endl;
		return 0;
	}
	return 1;
}

//���̿� �����̿��
int bread(void * _Buf, unsigned short int bno, int offset, int size, int count = 1)//������ �̿�� ƫ���� ���ݴ�С
{
	long place;
	int result;
	place = bno * blocksize + offset;//��Ӧ�����ַ
	fseek(disk, place, SEEK_SET);
	result = fread(_Buf, size, count, disk);
	if (result != count)
	{
		cout << "�̿��ȡʧ��" << endl;
		return 0;
	}
	return 1;
}

//��������̿�
int balloc()//�����̿��
{
	int bno;
	if (super->freeblocknum <= 0)//������ȫ����
		return 0;
	if (super->nextfreeblock == 1)//��ǰ�����̿�ջֻʣ��һ��Ԫ�� ���а�����һ���̿���Ϣ
	{
		super->nextfreeblock--;//ȡ���̿�
		bno = super->freeblock[super->nextfreeblock];

		fseek(disk,bno*blocksize,SEEK_SET);//ָ��ָ���������̿�
		int count = fread(super->freeblock,sizeof(int),maxfreeblock,disk);//��ȡ��һ���̿�ţ����20��
		
		super->nextfreeblock = count;//����ջ�����е��̿���
		super->freeblocknum--;//�ܵĿ����̿�-1
		superin();//Ѱ˼ֱ��д���޸�

		return bno;//�������һ��������
	}

	//ֱ�Ӵӿ���ջ����
	super->freeblocknum--;
	super->nextfreeblock--;
	superin();

	return super->freeblock[super->nextfreeblock];
}

//�����̿�
int bfree(int bno)//�����̿�Ž����̿����
{
	if (super->nextfreeblock == 20)//����ջ����
	{
		bwrite(super->freeblock, bno, 0, sizeof(int), 20);//����ջ��һ���̿鶼д���û��տ���
		super->nextfreeblock = 1;
		super->freeblock[0] = bno;//���տ��Ϊջ��
	}
	else
	{
		super->freeblock[super->nextfreeblock] = bno;//�ŵ�ջ��
		super->nextfreeblock++;
	}
	super->freeblocknum++;
	super->dirt = 1;
	superin();
	return 1;
}

//��ʼ����Ŀ¼
void makeroot()
{
	root = ialloc();//����Ŀ¼����һ�������ڵ�
	cout << "root ino:" << root->inodeID << endl;
					//dir * dir = (struct dir*)calloc(1, sizeof(struct dir));//root�Ǹ�Ŀ¼�ļ�
					//bread(dir, 1+finodeblocknum+1, 0, sizeof(struct dir));//22�鿪ʼ���ļ��̿�

	root->finode.arr[0] = balloc();//�����Ŀ¼�̿�
	cout << "root block" << root->finode.arr[0] << endl;

	strcpy_s(root->finode.power,"drw-rw---");
	strcpy_s(root->finode.owner,"root");
	strcpy_s(root->finode.group,"all");
	finodein(root);//��Ŀ¼д���̿�
}

//����ʼλ��s2������s1��
int strCpy(char *s1, char *s2, int offset)
{
	int len = strlen(s2);
	if (len <= offset)//�޺�������
		return 0;
	int i;
	for (i = 0; i<len - offset; i++)
	{
		s1[i] = s2[i + offset];
	}
	s1[i] = 0;//���ÿ��ַ��Ա���г��ȼ���
	return 1;
}

//�и� s1��s2
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

//�ҵ��ַ��ڴ���λ��
int findpos(char* str, int start, const char needle)
{
	for (int i = start; i<strlen(str); i++)
	{
		if (str[i] == needle)
			return i;
	}
	return -1;
}

//��¼ �����д
int login()
{
	int num;//�û���
	user *content;
	const char* filename = "user";//��ȡuser�ļ����û���Ϣ
	struct inode* temple = NULL;//����ҵ����ļ�inode
	dir * dir = (struct dir*)calloc(1, sizeof(struct dir));

	int count = current->finode.size / sizeof(struct direct);//��ǰ�ļ�����Ŀ

	int addrnum = count / dirnum + (count%dirnum >= 1 ? 1 : 0);
	addrnum>4 ? addrnum = 4 : NULL;
	bool find = false;
	for (int addr = 0; addr < addrnum; addr++)//�鿴��ǰĿ¼�Ƿ��Ѿ�����
	{
		bread(dir, current->finode.arr[addr], 0, sizeof(struct dir));
		for (int i = 0; i < dir->dirNum; i++)
			if (strcmp(dir->direct[i].directName, filename) == 0)//�ҵ�Ŀ¼
			{
				temple = iget(dir->direct[i].inodeID);//��ȡuser�ļ�������
				find = true;
				temple = iget(dir->direct[i].inodeID);
				//cout << dir->direct[i].inodeID << endl;
			}
		if (find)
			break;
	}
	if (!find)
	{
		cout << "δ�ڸ�Ŀ¼���ҵ��ļ�" << endl;
		return -1;
	}
	else//��ȡ�û�
	{
		num = temple->finode.size / sizeof(struct user);
		int bno = temple->finode.arr[0];
		content = (user*)calloc(num, sizeof(struct user));//�����û���Ϣ
		int result = bread(content, bno, 0, num * sizeof(struct user));
		/*for (int i = 0; i < num; i++)
		{
			cout << content[i].userName << endl;
			cout << content[i].userGroup << endl;
			cout << content[i].userPwd << endl;
		}*/
		//cout << temple->inodeID << endl;
	}
	//��ʼ��¼

	char username[maxname] = { 0 }, password[maxname] = { 0 };
	char ch;
	int i = 0;
	cout << "�����û���:";
	ch = getchar();//��ȡ����
	while (ch != 10)//��Ϊ�س�
	{
		username[i] = ch;//������ȡ
		ch = getchar();
		i++;
	}
	bool findname = false;
	for (int i = 0; i < num; i++)
	{
		if (strcmp(username,content[i].userName)==0)
		{
			findname = true;
			cout << "��������:";
			int p = 0;
			ch = getchar();//��ȡ����
			while (ch != 10)//��Ϊ�س�
			{
				password[p] = ch;//������ȡ
				ch = getchar();
				p++;
			}
			/*for (int k = 0; k < maxname; k++)
			{
				ch = _getch();
				if (ch == 13)//Ϊ���з�
				{
					break;
				}
				password[i] = ch;
				cout << "*";
			}*/
			if (strcmp(password,content[i].userPwd) == 0)
			{
				//system("cls");
				cout << "��¼�ɹ�" << endl;
				currentuser = content[i];//���õ�ǰ�û�
				return 0;
			}
		}
	}
	if (!findname)
	{
		cout << "���û�������" << endl;
		return -1;
	}
	return -1;
}

//������ǰ�û�����
int passwd()
{
	//user�ڵ��Ϊ15
	inode* temple = iget(15);//��ȡuser�ļ��������ڵ�

	int num = temple->finode.size / sizeof(struct user);
	int bno = temple->finode.arr[0];
	user* content = (user*)calloc(num, sizeof(struct user));//��ȡ�û�
	int result = bread(content, bno, 0, num * sizeof(struct user));

	int i ;
	for (i=0; i < num; i++)
	{
		if (strcmp(content[i].userName, currentuser.userName)==0)
			break;
	}

	char oldpwd[maxname] = { 0 }, newpwd[maxname] = { 0 }, repwd[maxname] = { 0 };

	cout << "����ԭ����:" << endl;
	char ch;
	int p = 0;
		ch = getchar();//��ȡ����
		while (ch != 10)//��Ϊ�س�
		{
			oldpwd[p] = ch;//������ȡ
			ch = getchar();
			p++;
		}
	//cout << content[i].userPwd << endl;
	if (strcmp(oldpwd, content[i].userPwd) == 0)
	{
		cout << "����������" << endl;

		int n = 0;
		ch = getchar();//��ȡ����������
		while (ch != 10)//��Ϊ�س�
		{
			newpwd[n] = ch;//������ȡ
			ch = getchar();
			n++;
		}

		cout << "ȷ��������:" << endl;

		int ne = 0;
		ch = getchar();//��ȡ����������
		while (ch != 10)//��Ϊ�س�
		{
			repwd[ne] = ch;//������ȡ
			ch = getchar();
			ne++;
		}

		if (strcmp(newpwd,repwd) == 0)//��ȫ�ɹ���д�����
		{
			strcpy_s(content[i].userPwd,newpwd);//��д
			bwrite(content, bno, 0, num*sizeof(user));//д�����
		}
		else
		{
			cout << "�����������벻ͬ" << endl;
			return -2;
		}
		
		return 0;
	}
	else
	{
		cout << "ԭ��������޸�ʧ��" << endl;
		return -1;
	}

	return 0;
}

//pwd ��ʾ��ǰλ��
int pwd()
{
	stack<char*> pwd;//ջ����ҵ����ļ���
	struct inode * inode = current;
	struct inode * parent = NULL;

	while (inode != root)//һ��һ������ȥֱ����Ŀ¼
	{
		int ino = inode->inodeID;
		parent = inode->parent;//�ҵ����ڵ�

		int count = parent->finode.size / sizeof(struct direct);
		dir * dir = (struct dir*)calloc(1, sizeof(struct dir));

		int addrnum = count / dirnum + (count % dirnum >= 1 ? 1 : 0);
		addrnum>4 ? addrnum = 4 : NULL;
		for (int addr = 0; addr<addrnum; addr++)//�Ӹ�Ŀ¼Ŀ¼�����ҵ���ǰ�ļ���
		{
			bread(dir, parent->finode.arr[addr], 0, sizeof(struct dir));
			for (int i = 0; i<dir->dirNum; i++)
			{
				if (dir->direct[i].inodeID == ino)//inoƥ��
				{
					pwd.push(dir->direct[i].directName);//�����ļ���
					count = -1;
					break;
				}
			}
			if (count == -1)
				break;
		}

		inode = inode->parent;//�Ը��ڵ��ظ�����
	}

	if (pwd.empty())//��ǰΪroot
		cout << "/";
	else
	{
		while (!pwd.empty())
		{
			cout << "/";
			cout.write(pwd.top(), strlen(pwd.top()));//�����������Ŀ��ַ�
			pwd.pop();//ջ����ȡ����Ӧ���ֲ����
		}
	}
	cout << endl;
	return 1;
}

//Ŀ¼��ת���޸�current
inode* cd(char* path,inode* inode)
{
	int start;
	int more;
	char path1[directname] = { 0 };//�����һ����ַ������

	if (path[0] == '/'&&strlen(path) == 1)//����/����ת�ظ�Ŀ¼
	{
		current = root;
		return NULL;
	}

	if (path[0] == '/')//���Ե�ַ
		start = 1;
	else
		start = 0;
	
	more = findpos(path,1,'/');//Ѱ���Ƿ��к�����ַ
	//cout << more << endl;
	substring(path,path1,start,more);//�зֵ�ǰ��ַ������path1
	//cout << path1 << endl;

	char type = inode->finode.power[0];//��ȡ�ļ�����

	if (type == 'd')//Ŀ¼
	{
		if ((!strcmp(path, ".")==0) && (!strcmp(path, "..")==0))
		{
			int count = inode->finode.size / sizeof(struct direct);//��ַ����Ŀ
			int addr = count / dirnum;
			if (count%dirnum >= 1)//��ƫ����
				addr++;
			if (addr > 4)
				addr = 4;

			dir *dir = (struct dir*)calloc(1, sizeof(struct dir));
			bool find = false;//�ж��ļ��Ƿ����

			for (int i = 0; i < addr; i++)
			{
				bread(dir, inode->finode.arr[i], 0, sizeof(struct dir));//�ڸ�����ַ���н��в���
				for (int j = 0; j < dir->dirNum; j++)//��������Ŀ¼�����в���
				{
					if (strcmp(dir->direct[j].directName, path1) == 0)//�Ƚ�������ȷ
					{
						find = true;
						struct inode* temple = iget(dir->direct[j].inodeID);//ȡ����һ���ļ��Ľڵ�
						if (temple->finode.power[0] == '-')//�¸��ļ���ַ��Ӧ����ͨ�ļ�
						{
							cout << "�޷��ƶ�����ͨ�ļ�" << endl;
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
				cout << "δ����Ԥ��תĿ¼" << endl;
				return NULL;
			}
		}
		else if (strcmp(path, ".")==0)//��ת����
		{
			inode = inode;
		}
		else if (strcmp(path, "..")==0)//��ת�ϲ�
		{
			inode = inode->parent;
		}

		if (more != -1 && inode != NULL)//�к�����ַ
		{
			substring(path, path1, more + 1, -1);
			return cd(path1, inode);//�ݹ����
		}
		else
			return inode;
	}
}

//��ʾ��ǰĿ¼���ļ�����Ŀ¼
int ls()
{
	char type = current->finode.power[0];//����Ȩ���ж��Ƿ�ΪĿ¼
	if (type != 'd')
	{
		cout << "����Ŀ¼�ļ�" << endl;
		return 0;
	}
	int count = current->finode.size/ sizeof(struct direct);
	dir * dir = (struct dir*)calloc(1, sizeof(struct dir));

	int addrnum = count / dirnum + (count%dirnum >= 1 ? 1 : 0);
	addrnum>4 ? addrnum = 4 : NULL;
	for (int addr = 0; addr<addrnum; addr++)
	{
		bread(dir, current->finode.arr[addr], 0, sizeof(struct dir));
		for (int i = 0; i<dir->dirNum; i++)//�ļ�����
			cout<<dir->direct[i].directName<<"  ";//����ļ���
		cout << endl;
	}
	return 1;
}

//��������������Ҳ��ʾ����
int lsl()
{
	char type = current->finode.power[0];//����Ȩ���ж��Ƿ�ΪĿ¼
	if (type != 'd')
	{
		cout << "����Ŀ¼�ļ�" << endl;
		return 0;
	}
	cout << "�ļ���" << "      " << "������" << "      " << "������" <<"       "<<"Ȩ��"<< "       " << "�ļ���С" << endl;
	int count = current->finode.size / sizeof(struct direct);
	dir * dir = (struct dir*)calloc(1, sizeof(struct dir));

	int addrnum = count / dirnum + (count%dirnum >= 1 ? 1 : 0);
	addrnum>4 ? addrnum = 4 : NULL;
	for (int addr = 0; addr<addrnum; addr++)
	{
		bread(dir, current->finode.arr[addr], 0, sizeof(struct dir));
		for (int i = 0; i < dir->dirNum; i++)//�ļ�����
		{
			inode* temple = iget(dir->direct[i].inodeID);
			cout << dir->direct[i].directName <<"     "<<temple->finode.owner<<"     "<<temple->finode.group << "     " <<temple->finode.power << "     " <<temple->finode.size<<endl;//����ļ���ϸ��Ϣ
		}
	}
	return 1;
}

//����Ŀ¼
int mkdir(const char*dirname)
{
	if (current->finode.power[0] != 'd')
	{
		cout << "��ǰλ��Ϊ����Ŀ¼" << endl;
		return -1;
	}
	
	dir * dir = (struct dir*)calloc(1, sizeof(struct dir));

	int count = current->finode.size/ sizeof(struct direct);//��ǰ�ļ�����Ŀ
	if (count >= dirnum-1)
	{
		cout << "��ǰĿ¼�ļ�̫�࣬����������" << endl;
		return -1;
	}

	int addrnum = count / dirnum + (count%dirnum >= 1 ? 1 : 0);
	addrnum>4 ? addrnum = 4 : NULL;
	for (int addr = 0; addr < addrnum; addr++)//�鿴��ǰĿ¼�Ƿ��Ѿ�����
	{
		bread(dir, current->finode.arr[addr], 0, sizeof(struct dir));
		for (int i = 0; i<dir->dirNum; i++)
			if (strcmp(dir->direct[i].directName, dirname) == 0)
			{
				cout.write(dirname, strlen(dirname));
				cout << "�ļ��Ѵ���" << endl;
				return -1;
			}
	}
	cout << "��ʼ����" << endl;
	current->finode.size += sizeof(struct direct);//��ǰĿ¼��һ��Ŀ¼��

	int addrnum1 = count / dirnum;
	bread(dir, current->finode.arr[addrnum1], 0, sizeof(struct dir));//��ȡ��Ŀ¼���һ����

	strcpy_s(dir->direct[dir->dirNum].directName, dirname);//���һ��Ŀ¼��Ϊ��Ŀ¼


	struct inode * newdocument = ialloc();//����Ŀ¼������нڵ�
	newdocument->finode.arr[0] = balloc();//�����µĿռ�

	//newdocument->finode.power[0] = 'd';//����Ȩ��
	strcpy_s(newdocument->finode.power, "drw-rw---");//����Ȩ��
	newdocument->finode.power[9] = '-';
	strcpy_s(newdocument->finode.owner, currentuser.userName);//������
	strcpy_s(newdocument->finode.group, currentuser.userGroup);//������
	finodein(newdocument);

	dir->direct[dir->dirNum].inodeID = newdocument->inodeID;//д��������
	dir->dirNum += 1;//Ŀ¼����+1
	bwrite(dir, current->finode.arr[addrnum1], 0, sizeof(struct dir));//ԭĿ¼�ļ��޲�д��

	finodein(current);//д��Ŀ¼��Ϣ

	return 0;
}

//ɾ����Ŀ¼
int rmdir(char*dirname)
{
	struct inode* temple=NULL;//����ҵ����ļ�inode
	dir * dir = (struct dir*)calloc(1, sizeof(struct dir));

	int count = current->finode.size / sizeof(struct direct);//��ǰ�ļ�����Ŀ

	int addrnum = count / dirnum + (count%dirnum >= 1 ? 1 : 0);
	addrnum>4 ? addrnum = 4 : NULL;
	bool find = false;
	for (int addr = 0; addr < addrnum; addr++)//�鿴��ǰĿ¼�Ƿ��Ѿ�����
	{
		bread(dir, current->finode.arr[addr], 0, sizeof(struct dir));
		for (int i = 0; i<dir->dirNum; i++)
			if (strcmp(dir->direct[i].directName, dirname) == 0)//�ҵ�Ŀ¼
			{
				temple= iget(dir->direct[i].inodeID);//��ȡҪɾ��Ŀ¼��������
				if (temple->finode.power[0] != 'd')
				{
					cout << "���ļ���ΪĿ¼" << endl;
					return -1;
				}
				else
				{
					if (temple->finode.size != 0)//�ǿ�
					{
						cout << "��ǰҪɾ��Ŀ¼��Ϊ�գ�����ɾ����Ŀ¼���ļ�" << endl;
						return -1;
					}
					else//�ҵ���Ŀ¼
					{
						find = true;
						for (int j = i; j < dir->dirNum; j++)
						{
							dir->direct[j] = dir->direct[j + 1];//ǰ�ƣ�ɾ����Ŀ¼��
						}
						dir->dirNum--;//Ŀ¼��-1
						//cout << temple->inodeID<< endl;
						//cout << dir->dirNum << endl;
						//cout << "ȷʵ������ɾ��"<<endl;
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
		cout << "��ǰĿ¼��û�и��ļ�" << endl;
	}
	else
	{
		cout << "��ʼɾ��" << endl;
		bfree(temple->finode.arr[0]);
		temple->finode.filelink--;//�ļ����Ӽ���-1
		finodein(temple);
		if(temple->finode.filelink==0)//û���ļ�������ɾ��
			ifree(temple); 

		current->finode.size -= sizeof(struct direct);//��ǰĿ¼��ȥһ��Ŀ¼��
		finodein(current);
	}
	return 0;
}

//�����ļ������� chown ownername filename
int chown(char* command)//�����������������ļ���
{
	char filename[directname] = { 0 };
	char ownername[maxname] = {0};
	int place = findpos(command,0,' ');//�ҵ����ַ�λ��
	substring(command,ownername,0,place);//��ȡ��������
	substring(command,filename,place+1,-1);//��ȡ�ļ���
	//cout << filename << endl;
	//cout << ownername << endl;

	struct inode* temple = NULL;//����ҵ����ļ�inode
	dir * dir = (struct dir*)calloc(1, sizeof(struct dir));

	int count = current->finode.size / sizeof(struct direct);//��ǰ�ļ�����Ŀ

	int addrnum = count / dirnum + (count%dirnum >= 1 ? 1 : 0);
	addrnum>4 ? addrnum = 4 : NULL;
	bool find = false;
	for (int addr = 0; addr < addrnum; addr++)//�鿴��ǰĿ¼�Ƿ��Ѿ�����
	{
		bread(dir, current->finode.arr[addr], 0, sizeof(struct dir));
		for (int i = 0; i < dir->dirNum; i++)
			if (strcmp(dir->direct[i].directName, filename) == 0)//�ҵ��ļ�
			{
				temple = iget(dir->direct[i].inodeID);//��ȡҪ�޸��ļ���������
				strcpy_s(temple->finode.owner, ownername);//�޸�������
				finodein(temple);
				find = true;
				break;
			}
		if (find)
			break;
	}
	if (!find)
	{
		cout << "δ�ڸ�Ŀ¼���ҵ��ļ�" << filename << endl;
		return -1;
	}
	else 
	{
		cout << "�ļ�" << filename << "ӵ�����޸�Ϊ" << ownername << endl;
		return 0;
	}	
	return 0;
}

//�����ļ������� chgrp groupname filename
int chgrp(char* command)//�����������������ļ���
{
	char filename[directname] = { 0 };
	char groupname[maxname] = { 0 };
	int place = findpos(command, 0, ' ');//�ҵ����ַ�λ��
	substring(command,groupname , 0, place);//��ȡ��������
	substring(command, filename, place + 1, -1);//��ȡ�ļ���
	//cout << filename << endl;
	//cout << groupname << endl;

	struct inode* temple = NULL;//����ҵ����ļ�inode
	dir * dir = (struct dir*)calloc(1, sizeof(struct dir));

	int count = current->finode.size / sizeof(struct direct);//��ǰ�ļ�����Ŀ

	int addrnum = count / dirnum + (count%dirnum >= 1 ? 1 : 0);
	addrnum>4 ? addrnum = 4 : NULL;
	bool find = false;
	for (int addr = 0; addr < addrnum; addr++)//�鿴��ǰĿ¼�Ƿ��Ѿ�����
	{
		bread(dir, current->finode.arr[addr], 0, sizeof(struct dir));
		for (int i = 0; i < dir->dirNum; i++)
			if (strcmp(dir->direct[i].directName, filename) == 0)//�ҵ��ļ�
			{
				temple = iget(dir->direct[i].inodeID);//��ȡҪ�޸��ļ���������
				strcpy_s(temple->finode.group, groupname);//�޸�������
				finodein(temple);
				find = true;
				break;
			}
		if (find)
			break;
	}
	if (!find)
	{
		cout << "δ�ڸ�Ŀ¼���ҵ��ļ�" << filename << endl;
		return -1;
	}
	else
	{
		cout << "�ļ�" << filename << "ӵ�����޸�Ϊ" << groupname << endl;
		return 0;
	}
	return 0;
}

//�޸��ļ�Ȩ��
int chmod(char*command)//ȫ����Ȩ��
{
	char filename[directname] = { 0 };
	char power[20] = { 0 };//�������ֹ����
	int place = findpos(command, 0, ' ');//�ҵ����ַ�λ��
	substring(command, power, 0, place);//��ȡȨ��
	substring(command, filename, place + 1, -1);//��ȡ�ļ���
	if (strlen(power) != 9)
	{
		cout << "����������ļ�������Ȩ��"<<endl;
		return -1;
	}
	for (int j = 0; j < strlen(power); j++)
	{
		if (power[j] != 'r' && power[j] != 'w' && power[j] != 'x'&&power[j]!='-')
		{
			cout << "�벻Ҫ�����޹��ַ�" << endl;
			return -1;
		}
	}

	struct inode* temple = NULL;//����ҵ����ļ�inode
	dir * dir = (struct dir*)calloc(1, sizeof(struct dir));

	int count = current->finode.size / sizeof(struct direct);//��ǰ�ļ�����Ŀ

	int addrnum = count / dirnum + (count%dirnum >= 1 ? 1 : 0);
	addrnum>4 ? addrnum = 4 : NULL;
	bool find = false;
	for (int addr = 0; addr < addrnum; addr++)//�鿴��ǰĿ¼�Ƿ��Ѿ�����
	{
		bread(dir, current->finode.arr[addr], 0, sizeof(struct dir));
		for (int i = 0; i < dir->dirNum; i++)
			if (strcmp(dir->direct[i].directName, filename) == 0)//�ҵ��ļ�
			{
				temple = iget(dir->direct[i].inodeID);//��ȡҪ�޸��ļ���������
				for (int i = 1; i <= 9; i++)
				{
					temple->finode.power[i] = power[i-1];	
				}
				finodein(temple);//д��
				find = true;
					break;		
			}
		if (find)
			break;
	}
	if (!find)
	{
		cout << "δ�ڸ�Ŀ¼���ҵ��ļ�" << filename << endl;
		return -1;
	}
	else
	{
		cout << "�ļ�" << filename << "Ȩ���޸�Ϊ" << power << endl;
		return 0;
	}
	return 0;
}

//�ı��ļ��� mv oldname newname
int mv(char* command)
{
	char oldname[directname] = { 0 };
	char newname[directname] = { 0 };//�������ֹ����
	int place = findpos(command, 0, ' ');//�ҵ����ַ�λ��
	substring(command, oldname, 0, place);//��ȡ���ļ���
	substring(command, newname, place + 1, -1);//��ȡ���ļ���
	//cout << oldname << endl;
	//cout << newname << endl;

	dir * dir = (struct dir*)calloc(1, sizeof(struct dir));

	int count = current->finode.size / sizeof(struct direct);//��ǰ�ļ�����Ŀ

	int addrnum = count / dirnum + (count%dirnum >= 1 ? 1 : 0);
	addrnum>4 ? addrnum = 4 : NULL;
	bool find = false;
	for (int addr = 0; addr < addrnum; addr++)//�鿴��ǰĿ¼�Ƿ��Ѿ�����
	{
		bread(dir, current->finode.arr[addr], 0, sizeof(struct dir));
		for (int i = 0; i < dir->dirNum; i++)
			if (strcmp(dir->direct[i].directName, oldname) == 0)//�ҵ��ļ�
			{
				strcpy_s(dir->direct[i].directName,newname);
				bwrite(dir,current->finode.arr[addr],0,sizeof(struct dir));//д��Ŀ¼�̿�
				find = true;
				break;
			}
		if (find == true)
			break;
	}
	if (!find)
	{
		cout << "δ�ڸ�Ŀ¼���ҵ��ļ�" << oldname << endl;
		return -1;
	}
	else
	{
		cout << "�ļ�" << oldname << "�����޸�Ϊ" << newname << endl;
		return 0;
	}
	return 0;
}

//�����ļ�(��)����Ѱ˼�����������������ļ����ݣ�
int touch(char* dirname)//�ļ��� ��֮�����ļ����ݣ�
{
	dir * dir = (struct dir*)calloc(1, sizeof(struct dir));

	int count = current->finode.size / sizeof(struct direct);//��ǰ�ļ�����Ŀ
	if (count >= dirnum - 1)
	{
		cout << "��ǰĿ¼�ļ�̫�࣬����������" << endl;
		return -1;
	}

	int addrnum = count / dirnum + (count%dirnum >= 1 ? 1 : 0);
	addrnum>4 ? addrnum = 4 : NULL;
	for (int addr = 0; addr < addrnum; addr++)//�鿴��ǰĿ¼�Ƿ��Ѿ�����
	{
		bread(dir, current->finode.arr[addr], 0, sizeof(struct dir));
		for (int i = 0; i<dir->dirNum; i++)
			if (strcmp(dir->direct[i].directName, dirname) == 0)
			{
				cout.write(dirname, strlen(dirname));
				cout << "�ļ��Ѵ���" << endl;
				return -1;
			}
	}

	cout << "��ʼ������ͨ���ļ�" << endl;
	current->finode.size += sizeof(struct direct);//��ǰĿ¼��һ��Ŀ¼��

	int addrnum1 = count / dirnum;
	bread(dir, current->finode.arr[addrnum1], 0, sizeof(struct dir));//��ȡ��Ŀ¼���һ����

	strcpy_s(dir->direct[dir->dirNum].directName, dirname);//���һ��Ŀ¼��Ϊ��Ŀ¼


	struct inode * newdocument = ialloc();//����Ŀ¼������нڵ�
	newdocument->finode.arr[0] = balloc();//�����µĿռ�

	//��������������ź��̿�
	cout << newdocument->inodeID << endl;
	cout << newdocument->finode.arr[0]<< endl;

	strcpy_s(newdocument->finode.power, "-rw-rw---");//����Ȩ��
	newdocument->finode.power[9] = '-';
	strcpy_s(newdocument->finode.owner, currentuser.userName);//������
	strcpy_s(newdocument->finode.group, currentuser.userGroup);//������
	finodein(newdocument);

	dir->direct[dir->dirNum].inodeID = newdocument->inodeID;//д��������
	dir->dirNum += 1;//Ŀ¼����+1
	bwrite(dir, current->finode.arr[addrnum1], 0, sizeof(struct dir));//ԭĿ¼�ļ��޲�д��

	finodein(current);//д��Ŀ¼��Ϣ

	return 0;

}

//ɾ���ļ�
int rm(char* dirname)
{
	struct inode* temple = NULL;//����ҵ����ļ�inode
	dir * dir = (struct dir*)calloc(1, sizeof(struct dir));

	int count = current->finode.size / sizeof(struct direct);//��ǰ�ļ�����Ŀ
	
	int addrnum = count / dirnum + (count%dirnum >= 1 ? 1 : 0);
	addrnum>4 ? addrnum = 4 : NULL;
	bool find = false;
	for (int addr = 0; addr < addrnum; addr++)//�鿴��ǰĿ¼�Ƿ��Ѿ�����
	{
		bread(dir, current->finode.arr[addr], 0, sizeof(struct dir));
		for (int i = 0; i<dir->dirNum; i++)
			if (strcmp(dir->direct[i].directName, dirname) == 0)//�ҵ�Ŀ¼
			{
				temple = iget(dir->direct[i].inodeID);//��ȡҪɾ���ļ���������
				if (temple->finode.power[0] != '-')
				{
					cout << "���ļ���Ϊ��ͨ�ļ�" << endl;
					return -1;
				}
				else
				{
					find = true;
					for (int j = i; j < dir->dirNum; j++)
					{
						dir->direct[j] = dir->direct[j + 1];//ǰ�ƣ�ɾ����Ŀ¼��
					}
					dir->dirNum--;//Ŀ¼��-1;
					bwrite(dir, current->finode.arr[addr], 0, sizeof(struct dir));//����Ŀ¼�ļ��������
					count = -1;
					break;
				}
			}
		if (find)
			break;
	}
	if (!find)
	{
		cout << "��ǰĿ¼��û�и��ļ�" << endl;
	}
	else//�ҵ����ļ�
	{
		cout << "��ʼɾ��" << endl;//��Ҫ����ж����ַ��Ӧ�̿�Ҫɾ��

		temple->finode.filelink--;
		finodein(temple);
		if (temple->finode.filelink == 0)
		{
			ifree(temple);
			//�������̿鶼free��
			//temple->finode.size

			int bcount = temple->finode.size / blocksize + (temple->finode.size%blocksize > 0 ? 1 : 0);//�����ļ�����ռ�˶����̿�
			if (bcount == 0)
				bcount = bcount + 1;
			if (bcount <= 4)//ֻ��ֱ��Ѱַ
			{
				for (int adrr = 0; adrr < bcount; adrr++)
				{
					bfree(temple->finode.arr[adrr]);//bfree����ֱ��Ѱַ���̿�
				}
			}
			else
			{
				if (bcount <= 4 + maxbno)//һ��Ѱַ��4 + blocksize/sizeof(int)
				{
					for (int i = 0; i < 4; i++)
					{
						bfree(temple->finode.arr[i]);//bfree����ֱ��Ѱַ���̿�
					}
					//���ʣ��һ��Ѱַ�̿���
					int other = bcount - 4;

					//�����̿�ſռ䣬bread�̿�ţ�ȫ��free��֮��free���̿�ŵ��Ǹ��̿�
					struct bnoblock* bno = (struct bnoblock*)calloc(1, sizeof(struct bnoblock));
					int place = temple->finode.arr[4] * blocksize;//��Ӧ�����ַ
					fseek(disk, place, SEEK_SET);
					int count = fread(bno, sizeof(int), maxbno, disk);//��ȡ�̿�ţ����128��

					for (int j = 0; j < other; j++)
					{
						bfree(bno->bno[j]);//�����ͷ��̿�	
					}
				}
				else if (bcount > (4 + maxbno))//����Ѱַ���ظ�����һ��Ѱַ
				{
					for (int i = 0; i < 4; i++)
					{
						bfree(temple->finode.arr[i]);//bfree����ֱ��Ѱַ���̿�
					}
					//���ʣ��һ��Ѱַ�̿���
					int other = bcount - 4;

					//���ͷ�һ��Ѱַ�е��̿�
					struct bnoblock* bno = (struct bnoblock*)calloc(1, sizeof(struct bnoblock));
					int place = temple->finode.arr[4] * blocksize;//��Ӧ�����ַ
					fseek(disk, place, SEEK_SET);
					int count = fread(bno, sizeof(int), maxbno, disk);//��ȡ�̿�ţ����128��
					for (int j = 0; j < maxbno; j++)
					{
						bfree(bno->bno[j]);//�����ͷ��̿�	
					}

					//�ͷŶ���Ѱַ�е��̿�
					other = other - maxbno;//ʣ�µ��̿���
					//int sum = 0;
					int bcount2 = other / maxbno + (other % maxbno > 0 ? 1 : 0);

					struct bnoblock* bno2 = (struct bnoblock*)calloc(1, sizeof(struct bnoblock));//����Ѱַ��ź����̿���̿�
					int place2 = temple->finode.arr[5] * blocksize;//��Ӧ�����ַ
					fseek(disk, place2, SEEK_SET);
					fread(bno2, sizeof(int), maxbno, disk);//��ȡ�̿�ţ����128��

					for (int i = 0; i < bcount2 - 1; i++)//�ȴ���bcount-1���̿�
					{
						struct bnoblock* bno = (struct bnoblock*)calloc(1, sizeof(struct bnoblock));
						int place = bno2->bno[i] * blocksize;//��Ӧ�����ַ
						fseek(disk, place, SEEK_SET);
						int count = fread(bno, sizeof(int), maxbno, disk);//��ȡ�̿�ţ����128��
						for (int j = 0; j < maxbno; j++)
						{
							bfree(bno->bno[j]);//�����ͷ��̿�	
						}
					}

					other -= (bcount2 - 1) * 128;//���һ���̿���ص��̿���
					struct bnoblock* otherbno = (struct bnoblock*)calloc(1, sizeof(struct bnoblock));
					int otherplace = bno2->bno[bcount2 - 1] * blocksize;//��Ӧ�����ַ
					fseek(disk, place, SEEK_SET);
					int othercount = fread(bno, sizeof(int), maxbno, disk);//��ȡ�̿�ţ����128��
					for (int o = 0; o < other; o++)
					{
						bfree(otherbno->bno[o]);
					}
				}
			}
			current->finode.size -= sizeof(struct direct);//��ǰĿ¼��ȥһ��Ŀ¼��
			finodein(current);
			superin();//д�볬����
		}
	}
	return 0;
}

//�����û�
int adduser(const char* filename,user* user,int num)
{
	struct inode* temple = NULL;//����ҵ����ļ�inode
	dir * dir = (struct dir*)calloc(1, sizeof(struct dir));

	int count = current->finode.size / sizeof(struct direct);//��ǰ�ļ�����Ŀ

	int addrnum = count / dirnum + (count%dirnum >= 1 ? 1 : 0);
	addrnum>4 ? addrnum = 4 : NULL;
	bool find = false;
	for (int addr = 0; addr < addrnum; addr++)//�鿴��ǰĿ¼�Ƿ��Ѿ�����
	{
		bread(dir, current->finode.arr[addr], 0, sizeof(struct dir));
		for (int i = 0; i < dir->dirNum; i++)
			if (strcmp(dir->direct[i].directName, filename) == 0)//�ҵ�Ŀ¼
			{
				temple = iget(dir->direct[i].inodeID);//��ȡҪɾ���ļ���������
				if (temple->finode.power[0] != '-')
				{
					cout << "���ļ���Ϊ��ͨ�ļ�" << endl;
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
		cout << "δ�ڸ�Ŀ¼���ҵ��ļ�"<< endl;
		return -1;
	}
	else
	{
		temple->finode.size += sizeof(struct user)*num;//����num���û��ṹ�ĳ���
		int bno=temple->finode.arr[0];
		struct user users[3];
		for (int i = 0; i < 3; i++)
			users[i] = user[i];
		bwrite(users,bno,0,sizeof(users));//д�����
		finodein(temple);
	}
}

//��ʼ���û���Ϣ
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

//�鿴�ļ���Ϣ
int cat(char*filename)//��࣬��д������
{
	struct inode* temple = NULL;//����ҵ����ļ�inode
	dir * dir = (struct dir*)calloc(1, sizeof(struct dir));

	int count = current->finode.size / sizeof(struct direct);//��ǰ�ļ�����Ŀ

	int addrnum = count / dirnum + (count%dirnum >= 1 ? 1 : 0);
	addrnum>4 ? addrnum = 4 : NULL;
	bool find = false;
	for (int addr = 0; addr < addrnum; addr++)//�鿴��ǰĿ¼�Ƿ��Ѿ�����
	{
		bread(dir, current->finode.arr[addr], 0, sizeof(struct dir));
		for (int i = 0; i < dir->dirNum; i++)
			if (strcmp(dir->direct[i].directName, filename) == 0)//�ҵ�Ŀ¼
			{
				temple = iget(dir->direct[i].inodeID);//��ȡҪɾ���ļ���������
				if (temple->finode.power[0] != '-')
				{
					cout << "���ļ���Ϊ��ͨ�ļ�" << endl;
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
		cout << "δ�ڸ�Ŀ¼���ҵ��ļ�" << endl;
		return -1;
	}
	else//
	{
		int bno = temple->finode.arr[0];
		int place = 0;
		char contain[blocksize] = { 0 };//��ʱ�������
		char* content = (char*)calloc(temple->finode.size,sizeof(char));//������е�����
		
		int bcount = temple->finode.size / blocksize + (temple->finode.size%blocksize > 0 ? 1 : 0);//�����ļ�����ռ�˶����̿�
		if (bcount <= 4&&(bcount != 0))//ֻ��ֱ��Ѱַ
		{
			for (int adrr = 0; adrr < bcount-1; adrr++)//ǰ���������ȡ����
			{
				bread(contain,temple->finode.arr[adrr],0,blocksize,1);
				for (int i = 0; i < blocksize; i++)
				{
					content[place] = contain[i];//��contain�����е����ݸ��Ƶ�content�У�����֮�����
					place++;
				}
			}
			bread(contain,temple->finode.arr[bcount-1],0,temple->finode.size-(bcount-1)*blocksize);//��ȡ����������̿�����
			for (int k = 0; k < temple->finode.size - (bcount - 1)*blocksize; k++)
			{
				content[place]=contain[k];
				place++;
			}
			//���
			for (int i = 0; i < temple->finode.size; i++)
			{
				if ((int)content[i] < 0)//��Ϊ-2��ռλ���������
					cout << " ";
				else
					_putch(content[i]);
			}
			cout << endl;
		}
		else if(bcount > 4 && (bcount != 0))//һ��Ѱַ
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
			//��ûд��
			bcount = bcount - 4;//�����̿�����

			struct bnoblock* bno = (struct bnoblock*)calloc(1, sizeof(struct bnoblock));
			int place1 = temple->finode.arr[4] * blocksize;//��Ӧ�����ַ
			fseek(disk, place1, SEEK_SET);
			
			int count = fread(bno, sizeof(int), maxbno, disk);//��ȡ�̿�ţ����128��
			for (int b = 0; b < bcount-1; b++)//�������һ������512�ֽ�
			{
				bread(contain, bno->bno[b], 0, blocksize, 1);
				for (int i = 0; i < blocksize; i++)
				{
					content[place] = contain[i];
					place++;
				}
			}
			int other = temple->finode.size - (bcount - 1)*blocksize-4 * blocksize;//ͬʱ��ȥֱ��Ѱַ�̿�
			bread(contain, bno->bno[bcount-1], 0, other, 1);
			for (int i = 0; i < other; i++)//��ȡ���һ���̿���Ϣ
			{
				content[place] = contain[i];
				place++;
			}
			//���
			for (int i = 0; i < temple->finode.size; i++)
			{
				if ((int)content[i] < 0)//��Ϊ-2��ռλ���������
					cout << " ";
				else
					_putch(content[i]);
			}
			cout << endl;
		}
	}
	return 0;
}


//�����ļ���һ���̿�ӵ�����
int addcontent(char* command)
{
	char filename[directname] = { 0 };
	char content[200] = { 0 };//�������ֹ����
	int place = findpos(command, 0, ' ');//�ҵ����ַ�λ��
	substring(command, filename, 0, place);//��ȡ�ļ���
	substring(command, content, place + 1, -1);//��ȡ��������
	cout << strlen(content) << endl;
	cout << filename << endl;
	cout << content << endl;

	struct inode* temple = NULL;//����ҵ����ļ�inode
	dir * dir = (struct dir*)calloc(1, sizeof(struct dir));

	int count = current->finode.size / sizeof(struct direct);//��ǰ�ļ�����Ŀ

	int addrnum = count / dirnum + (count%dirnum >= 1 ? 1 : 0);
	addrnum>4 ? addrnum = 4 : NULL;
	bool find = false;
	for (int addr = 0; addr < addrnum; addr++)//�鿴��ǰĿ¼�Ƿ��Ѿ�����
	{
		bread(dir, current->finode.arr[addr], 0, sizeof(struct dir));
		for (int i = 0; i < dir->dirNum; i++)
			if (strcmp(dir->direct[i].directName, filename) == 0)//�ҵ��ļ�
			{
				temple = iget(dir->direct[i].inodeID);//��ȡҪɾ���ļ���������
				if (temple->finode.power[0] != '-')
				{
					cout << "���ļ���Ϊ��ͨ�ļ�" << endl;
					return -1;
				}
				else
				{
					temple = iget(dir->direct[i].inodeID);//��ȡҪ�޸��ļ���������
					temple->finode.size = strlen(content);//�޸��ļ���С
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
		cout << "δ�ڸ�Ŀ¼���ҵ��ļ�" << filename << endl;
		return -1;
	}
	else
	{
		bwrite(content, temple->finode.arr[0], 0, strlen(content));//д���̿�
		cout << "д���ļ����" << endl;
	}
	return 0;
}

//���ļ������ڲ���
int bigfile()
{
	//�ڵ��9 �ǵð�size����
	inode* temple = iget(9);//���ڵ��µ�bigfile�ļ�
	for (int i = 1; i < 4; i++)//��ֱ���̿�ȫ����
	{
		temple->finode.arr[i] = balloc();//�־�������
		cout << temple->finode.arr[i] << endl;
	}
	char content[blocksize];
	for(int i=0;i<blocksize;i++)
		content[i] ='a';//һ���̿��a
	for(int k=0;k<4;k++)
		bwrite(content, temple->finode.arr[k], 0, blocksize);

	//��һ������������
	temple->finode.arr[4]=balloc();
	int bno = balloc();//�ŵ�һ��������Ӧ�̿���
	bwrite(&bno,temple->finode.arr[4],0,sizeof(int));
	bwrite(content, bno, 0, blocksize);//������̿�����

	temple->finode.size = blocksize * 5;
	finodein(temple);
	return 0;
}

//Ӳ����
int ln(char* command)//ԭ�ļ� Ŀ���ļ�
{
	char filename[directname] = {0};//ԭ�ļ���
	char target[200] = {0};//Ŀ��λ�ú����ļ���
	int place = findpos(command, 0, ' ');//�ҵ����ַ�λ��
	substring(command, filename, 0, place);//��ȡ���ļ���
	substring(command, target, place + 1, -1);//��ȡ���ļ����͵�ַ
		//cout <<filename<<endl;
		//cout << target << endl;

	struct inode* temple = NULL;//����ҵ����ļ�inode
	dir * dir = (struct dir*)calloc(1, sizeof(struct dir));

	int count = current->finode.size / sizeof(struct direct);//��ǰ�ļ�����Ŀ

	int addrnum = count / dirnum + (count%dirnum >= 1 ? 1 : 0);
	addrnum>4 ? addrnum = 4 : NULL;
	bool find = false;
	for (int addr = 0; addr < addrnum; addr++)//�鿴��ǰĿ¼�Ƿ��Ѿ�����
	{
		bread(dir, current->finode.arr[addr], 0, sizeof(struct dir));
		for (int i = 0; i < dir->dirNum; i++)
			if (strcmp(dir->direct[i].directName, filename) == 0)//�ҵ�Ŀ¼
			{
				temple = iget(dir->direct[i].inodeID);//��ȡҪ�����ļ���������
				if (temple->finode.power[0] != '-')
				{
					cout << "���ļ���Ϊ��ͨ�ļ�" << endl;
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
		cout << "δ�ڸ�Ŀ¼���ҵ��ļ�" << endl;
		return -1;
	}
	else
	{
		//��ʼ����
		//��cd����λ�ã�ͬʱ�ж��Ƿ��Ѿ�����
		//���������ļ�Ŀ¼��
		inode* targetinode = NULL;
		root->parent = root;
		if (target[0] == '/')//���Ե�ַ
			targetinode = root;
		else
			targetinode = current;
		int lastplace;
		for (int i = strlen(target); i > 0; i--)//Ѱ�����һ��/��λ�����ָ��ļ���
		{
			if (target[i] == '/')
			{
				lastplace = i;
				break;
			}
		}
		char newfilename[directname] = { 0 };
		substring(target,newfilename,lastplace+1,-1);//���ļ���
		substring(target,target,0,lastplace);//��ת��ַ
			//cout << newfilename << endl;
			//cout << target<<endl;
		targetinode = cd(target,targetinode);//��ת
		if (targetinode == NULL)
		{
			cout << "����ʧ��" << endl;
			return -1;
		}
		else//��ʼ����Ŀ¼��
		{
			struct dir * targetdir = (struct dir*)calloc(1, sizeof(struct dir));
			int targetcount = targetinode->finode.size / sizeof(struct direct);//Ŀ��Ŀ¼�ļ�����Ŀ
			int targetaddrnum = targetcount / dirnum + (count%dirnum >= 1 ? 1 : 0);
			targetaddrnum>4 ? targetaddrnum = 4 : NULL;
			for (int addr = 0; addr <targetaddrnum; addr++)//�鿴��ǰĿ¼�Ƿ��Ѿ�����
			{
				bread(targetdir, targetinode->finode.arr[addr], 0, sizeof(struct dir));
				for (int i = 0; i<targetdir->dirNum; i++)
					if (strcmp(targetdir->direct[i].directName,newfilename) == 0)
					{
						cout << newfilename << endl;
						cout << "�ļ��Ѵ���" << endl;
						return -1;
					}
			}
			int addrnum1 = count / dirnum;
			bread(targetdir, targetinode->finode.arr[addrnum1], 0, sizeof(struct dir));//��ȡĿ��Ŀ¼

			targetinode->finode.size += sizeof(struct direct);//Ŀ��Ŀ¼��һ��Ŀ¼��
			strcpy_s(targetdir->direct[targetdir->dirNum].directName, newfilename);//���һ��Ŀ¼��
			targetdir->direct[targetdir->dirNum].inodeID = temple->inodeID;//����������
			targetdir->dirNum += 1;
			bwrite(targetdir,targetinode->finode.arr[addrnum1],0,sizeof(struct dir));//д�����һ���̿�

			temple->finode.filelink++;
			finodein(temple);//���������ڵ���Ϣ
		}
	}
	cout << "�ļ�" << filename << "�������ļ�" << target << "���ӳɹ�" << endl;
	return 0;
}

//�ļ�����
int cp(char* command)
{
	char filename[directname] = { 0 };//ԭ�ļ���
	char target[200] = { 0 };//Ŀ��λ�ú����ļ���
	int place = findpos(command, 0, ' ');//�ҵ����ַ�λ��
	substring(command, filename, 0, place);//��ȡ���ļ���
	substring(command, target, place + 1, -1);//��ȡ��ַ
											  //cout <<filename<<endl;
											  //cout << target << endl;

	struct inode* temple = NULL;//����ҵ����ļ�inode
	dir * dir = (struct dir*)calloc(1, sizeof(struct dir));

	int count = current->finode.size / sizeof(struct direct);//��ǰ�ļ�����Ŀ

	int addrnum = count / dirnum + (count%dirnum >= 1 ? 1 : 0);
	addrnum>4 ? addrnum = 4 : NULL;
	bool find = false;
	for (int addr = 0; addr < addrnum; addr++)//�鿴��ǰĿ¼�Ƿ��Ѿ�����
	{
		bread(dir, current->finode.arr[addr], 0, sizeof(struct dir));
		for (int i = 0; i < dir->dirNum; i++)
			if (strcmp(dir->direct[i].directName, filename) == 0)//�ҵ�Ŀ¼
			{
				temple = iget(dir->direct[i].inodeID);//��ȡҪɾ���ļ���������
				if (temple->finode.power[0] != '-')
				{
					cout << "���ļ���Ϊ��ͨ�ļ�" << endl;
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
		cout << "δ�ڸ�Ŀ¼���ҵ��ļ�" << endl;
		return -1;
	}
	else//��ʼ����
	{
		cout << filename << endl;
		cout << target << endl;

		inode* targetinode = NULL;//Ŀ��Ŀ¼inode
		root->parent = root;
		if (target[0] == '/')//���Ե�ַ
			targetinode = root;
		else
			targetinode = current;

		targetinode = cd(target, targetinode);//��ת
		if (targetinode == NULL)
		{
			cout << "����Ŀ¼ʧ��" << endl;
			return -1;
		}
		else//��ʼ����Ŀ¼��
		{
			struct dir * targetdir = (struct dir*)calloc(1, sizeof(struct dir));
			int targetcount = targetinode->finode.size / sizeof(struct direct);//Ŀ��Ŀ¼�ļ�����Ŀ
			int targetaddrnum = targetcount / dirnum + (count%dirnum >= 1 ? 1 : 0);
			targetaddrnum > 4 ? targetaddrnum = 4 : NULL;
			for (int addr = 0; addr < targetaddrnum; addr++)//�鿴��ǰĿ¼�Ƿ��Ѿ�����
			{
				bread(targetdir, targetinode->finode.arr[addr], 0, sizeof(struct dir));
				for (int i = 0; i < targetdir->dirNum; i++)
					if (strcmp(targetdir->direct[i].directName, filename) == 0)
					{
						cout << filename << endl;
						cout << "�ļ��Ѵ���" << endl;
						return -1;
					}
			}
			int addrnum1 = count / dirnum;
			bread(targetdir, targetinode->finode.arr[addrnum1], 0, sizeof(struct dir));//��ȡĿ��Ŀ¼

			targetinode->finode.size += sizeof(struct direct);//Ŀ��Ŀ¼��һ��Ŀ¼��
			strcpy_s(targetdir->direct[targetdir->dirNum].directName, filename);//���һ��Ŀ¼��
			struct inode* newinode = ialloc();//���ļ�inode
			targetdir->direct[targetdir->dirNum].inodeID =newinode->inodeID;//����������
			targetdir->dirNum += 1;
			bwrite(targetdir, targetinode->finode.arr[addrnum1], 0, sizeof(struct dir));//д�����һ���̿�

			//���������ڵ�����
			newinode->finode.size = temple->finode.size;
			for (int i = 0; i < 10; i++)//�����ļ�Ȩ��
			{
				newinode->finode.power[i] = temple->finode.power[i];
			}
			for (int i = 0; i < maxname; i++)//����ӵ���ߺ�������
			{
				newinode->finode.owner[i] = temple->finode.owner[i];
				newinode->finode.group[i] = temple->finode.group[i];
			}

			//�����̿�����
			int bcount = temple->finode.size / blocksize + (temple->finode.size%blocksize > 0 ? 1 : 0);//�����ļ�����ռ�˶����̿�
			if (bcount == 0)
				bcount = bcount + 1;
			if (bcount <= 4)//ֻ��ֱ��Ѱַ
			{
				for (int adrr = 0; adrr < bcount; adrr++)
				{
					newinode->finode.arr[adrr] = balloc();//�������̿�
					char contain[blocksize] = { 0 };//��ʱ�������
					bread(contain,temple->finode.arr[adrr],0,blocksize);//ԭ����
					bwrite(contain,newinode->finode.arr[adrr],0,blocksize);//���̿�д��
				}
			}
			else if (bcount <= 4 + maxbno)//һ��Ѱַ��4 + blocksize/sizeof(int)
			{
				for (int adrr = 0; adrr < 4; adrr++)//�Ȱ�ֱ�ӵĶ�������
				{
					newinode->finode.arr[adrr] = balloc();//�������̿�
					char contain[blocksize] = { 0 };//��ʱ�������
					bread(contain, temple->finode.arr[adrr], 0, blocksize);//ԭ����
					bwrite(contain, newinode->finode.arr[adrr], 0, blocksize);//���̿�д��
				}
				//���ʣ��һ��Ѱַ�̿���
				int other = bcount - 4;

				//�����̿�ſռ�
				struct bnoblock* bno = (struct bnoblock*)calloc(1, sizeof(struct bnoblock));//һ���̿��
				int place = temple->finode.arr[4] * blocksize;//��Ӧ�����ַ
				fseek(disk, place, SEEK_SET);
				int count = fread(bno, sizeof(int), maxbno, disk);//��ȡ�̿�ţ����128��

				newinode->finode.arr[4] = balloc();//һ��Ѱַ
				
				struct bnoblock* newbno = (struct bnoblock*)calloc(1, sizeof(struct bnoblock));//һ���̿��
				for (int i = 0; i < other; i++)
				{
					newbno->bno[i] = balloc();
				}
				bwrite(newbno, newinode->finode.arr[4], 0, sizeof(struct bnoblock));//һ��Ѱַ�̿�д��
				for (int i = 0; i < other; i++)
				{
					char contain[blocksize] = { 0 };//��ʱ�������
					bread(contain, bno->bno[i], 0, blocksize);//ԭ����
					bwrite(contain,newbno->bno[i],0,blocksize);//д�����̿�
				}
			}
			finodein(newinode);
		}
	}
	cout << "�ļ�" << filename << "����Ϊ�ļ�" << target <<"/"<<filename<< endl;
	return 0;
}

//������
void help(char*command)
{
	//cout << "û�²�Ҫhelp"<<endl;
	cout << "ls: ��ʾ��ǰĿ¼���ļ��� ����ls-l��ʾ��ϸ��Ϣ"<<endl;
	cout << "chmod: �޸�Ȩ�ޣ�����Ϊ�Ÿ�Ȩ���ַ��͵�ǰĿ¼���ļ���" << endl;
	cout << "chown: �޸������ߣ�����Ϊ�����������͵�ǰĿ¼���ļ���" << endl;
	cout << "chgrp: �޸������飬����Ϊ�����������͵�ǰĿ¼���ļ���" << endl;
	cout << "pwd: ��ʾ��ǰ����Ŀ¼���Ե�ַ" << endl;
	cout << "cd: ������Ҫ��ת��Ŀ¼��ַ��������������ԭ��" << endl;
	cout << "mkdir: ������Ҫ������Ŀ¼�����ڵ�ǰĿ¼�´�����Ӧ��Ŀ¼" << endl;
	cout << "rmdir: ������Ҫɾ���Ŀ�Ŀ¼������ǰĿ¼�´�����Ϊ����ɾ��" << endl;
	cout << "mv: ������Ҫ������ԭ�ļ������ļ������ɸ���" << endl;
	cout << "cp: �����븴�Ƶ�ԭ�ļ���Ŀ��Ŀ¼������Ŀ��Ŀ¼�������ļ�" << endl;
	cout << "rm: ɾ����ǰĿ¼����ͨ�ļ��������ļ�������" << endl;
	cout << "ln: Ӳ���ӣ�����ԭ�ļ�����Ŀ�����ļ��������ӣ���������Ŀ¼�ļ�" << endl;
	cout << "cat: ��ʾ�ļ����ݣ����뵱ǰĿ¼���ļ�������" << endl;
	cout << "passwd: �޸ĵ�ǰ�û�����" << endl;
	cout<<"touch: �������ļ�"<<endl;
}

//��ȡ����
int getcommand()
{
	cout << "[ ";
	cout << currentuser.userName << "  @loaclhost :$";
	char command[2000] = {0};//ǰ������
	char enter[2000] = {0};//������
	char **com = new char*[5];
	char ch;
	int i = 0;

	ch = getchar();//��ȡ����
	if (ch == 10)//Ϊ�س��򷵻�
		return 0;
	while (ch != 10)//��Ϊ�س�
	{
		enter[i] = ch;//������ȡ
		ch = getchar();
		i++;
	}
	strcpy_s(command,enter);
	strtok_s(command," ",com);//��һ���ո�����ж�
	//cout << strlen(enter)<<endl;
	//cout << strlen(command)<<endl;

	if (strcmp(command, "cd") == 0)
	{
		root->parent = root;
		inode* temple;
		strCpy(command,enter,strlen(command)+1);//��ú�������command
		if (command[0] == '/')
			temple = root;//�Ӹ�Ŀ¼��ת
		else
			temple = current;//�ӵ�ǰĿ¼��ת
		temple = cd(command, temple);
		if (temple != NULL)
			current =temple;
		cout << "��ǰλ��Ϊ:";
		pwd();
	}
	/*else if (strcmp(command,"inituser") == 0)//������ʼ���û�
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
		strCpy(command, enter, strlen(command) + 1);//��ú�������command
		if (strlen(command) <= 13)
			mkdir(command);
		else
			cout << "Ŀ¼������" << endl;
	}
	else if (strcmp(command,"rmdir")==0)
	{
		strCpy(command, enter, strlen(command) + 1);//��ú�������command
		if (strlen(command) <= 13)
			rmdir(command);
		else
			cout << "Ŀ¼������" << endl;
	}
	else if (strcmp(command, "touch") == 0)
	{
		strCpy(command, enter, strlen(command) + 1);//��ú�������command
		if (strlen(command) <= 13)
			touch(command);
		else
			cout << "Ŀ¼������" << endl;
	}
	else if (strcmp(command, "cat") == 0)
	{
		strCpy(command, enter, strlen(command) + 1);//��ú�������command
		if (strlen(command) <= 13)
			cat(command);
		else
			cout << "Ŀ¼������" << endl;
	}
	else if (strcmp(command,"chown")==0)
	{
		strCpy(command, enter, strlen(command) + 1);//��ú�������command
		chown(command);
	}
	else if (strcmp(command, "chgrp") == 0)
	{
		strCpy(command, enter, strlen(command) + 1);//��ú�������command
		chgrp(command);
	}
	else if (strcmp(command, "chmod") == 0)
	{
		strCpy(command, enter, strlen(command) + 1);//��ú�������command
		chmod(command);
	}
	else if (strcmp(command, "ln") == 0)
	{
		strCpy(command, enter, strlen(command) + 1);//��ú�������command
		ln(command);
	}
	else if (strcmp(command, "cp") == 0)
	{
		strCpy(command, enter, strlen(command) + 1);//��ú�������command
		cp(command);
	}
	else if (strcmp(command, "mv") == 0)
	{
		strCpy(command, enter, strlen(command) + 1);//��ú�������command
		mv(command);
	}
	else if (strcmp(command, "add") == 0)
	{
		strCpy(command,enter,strlen(command)+1 );
		addcontent(command);
	}
	else if (strcmp(command, "rm") == 0)
	{
		strCpy(command, enter, strlen(command) + 1);//��ú�������command
		if (strlen(command) <= 13)
			rm(command);
		else
			cout << "Ŀ¼������" << endl;
	}
	else if (strcmp(command, "exit") == 0)
	{
		commandover = true;
	}
	else if (strcmp(command, "help") == 0)
	{
		if (strlen(enter) != 4)//�к���	
		{
			strCpy(command, enter, strlen(command) + 1);//��ú�������command
			help(command);
		}
		else
		{
			help(command);
		}
	}
	return 0;
}