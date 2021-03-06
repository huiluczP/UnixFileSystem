// UnixFileSystem.cpp: 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "filecontrol.h"

int main()
{
	
	//initialize("test.dat");//超级块初始化
	//formatting("test.dat");//成组链接初始化

	/*superload("test.dat");
	makeroot();//初始化root节点
	superInfo();
	inodeinfo(root);*/

	superload("test.dat");
	root = iget(19);//读取root根节点索引
	//inodeinfo(root);

	user u;
	const char *group = "all";
	const char *name = "root";
	const char *pwd1 = "123456";
	strcpy_s(u.userGroup,group);
	strcpy_s(u.userName,name);
	strcpy_s(u.userPwd,pwd1);
	/*cout << u.userGroup << endl;
	cout << u.userName << endl;
	cout << u.userPwd << endl<<endl;*/
	currentuser = u;

	current = root;

	while(login()!=0)
	{
		cout << "用户名或密码错误，重新登录"<<endl;
	}

	while(commandover!=true)
		getcommand();

	system("pause");

    return 0;
}

