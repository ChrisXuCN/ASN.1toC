#include<iostream>
#include<fstream>
#include<string>
#include<vector>
#include<iomanip>
#include<stack>
using namespace std;
//语法项结点
struct node {
	int id;				//标识此语法项的类型
	string itemName;	//名
	int nestedCnt;		//嵌套标志，同一嵌套结构中的所有语法项此项相同
	int val;			//0表示类型定义，1表示类型使用
	int ln;				//指出该语法项所在行
	bool moduleIn;		
	struct node *next;	//指向下一个节点
};
//堆栈的数据节点
struct St {
	string s;
	int ln;
	int level;
};
/***************************
*
*第一遍扫描，建立动态链表
*
****************************/
/*
//划分
int Split(const string &str, const string &splitchar, vector <string> &vec){
	string stmp = "";
	string::size_type pos = 0, prev_pos = 0;
	int j = 0;
	vec.clear();
	while ((pos = str.find_first_of(splitchar, pos)) != string::npos)	{
		stmp = str.substr(prev_pos, pos - prev_pos);
		vec.push_back(stmp);
		prev_pos = ++pos;

	}
	stmp = str.substr(prev_pos, pos - prev_pos);
	if (stmp.length() > 0){
		vec.push_back(stmp);
	}
	return 0;
}*/
/*
*2019-4-22
*更新划分函数
*/
void split(const string &str, vector<string> &vec) {
	string stmp = "";
	string::size_type pos = 0, prev_pos = 0, i = 0;
	int cnt = 0;			//1表示由单词或其他东西，0表示空格
	vec.clear();
	while (i < str.length()) {
		if (!isspace(str[i]) && cnt == 0) {	//表示从此开始出现字符
			cnt = 1;
			pos = i;
		}
		else if (isspace(str[i]) && cnt == 1) {	//表示从此开始字符结束
			cnt = 0;
			prev_pos = i;
			stmp = str.substr(pos, prev_pos - pos);
			vec.push_back(stmp);				//将字符置入容器
		}
		++i;
	}
	prev_pos = i;
	stmp = str.substr(pos, prev_pos - pos);
	vec.push_back(stmp);
}


//检查类型
int checkType(const string & s) {
	vector<string> type = { "INTEGER","REAL","ENUMERATED","BIT STRING","OCTET STRING","NULL","CHOCIE",
							"SEQUENCE","SEQUENCE OF" ,"DEFINITIONS" };
	for (int i = 0; i < 10; i++) {
		if (s == type[i])
			return i;
	}
	return -1;
}

node* firstScan() {
	node* L = new node;//头结点
	node* r = L;
	L->next = NULL;
	fstream file;
	file.open("test.asn1", ios::in);
	if (!file) {
		cout << "文件打开失败" << endl;
		exit(0);
	}
	char buffer[256];
	string str;
	int ln = 0;
	bool moduleIn = false;
	while (!file.eof()) {			//判断是否到达文件末尾
		file.getline(buffer, sizeof(buffer));
		ln++;
		str = buffer;
		vector<string> evc;
		split(str, evc);
		for (auto ai : evc) {
			int i = checkType(ai);
			if (i > -1) {		//表明该字符串是一个类型名或者模板名
				node* p = new node;
				p->val = -1;
				p->id = i;
				p->itemName = ai;
				p->nestedCnt = 0;
				p->ln = ln;
				p->moduleIn = moduleIn;
				p->next = NULL;
				r->next = p;			//尾插法  
				r = p;
			}
			else if (ai == "BEGIN")	moduleIn = true;//	表示后面的语法项都在模板内部
			else if (ai == "END") moduleIn = false;// 表示模板结束
			else {
				if (isupper(ai[0])) {	//表示为定义的类型名
					node* p = new node;
					p->val = -1;
					p->id = -1;
					p->itemName = ai;
					p->nestedCnt = 0;
					p->ln = ln;
					p->moduleIn = moduleIn;
					p->next = NULL;
					r->next = p;		//尾插法
					r = p;
				}
				else if (islower(ai[0])) {	//表示为定义的变量名
					node* p = new node;
					p->val = -1;
					p->id = -1;
					p->itemName = ai;
					p->nestedCnt = 0;
					p->ln = ln;
					p->moduleIn = moduleIn;
					p->next = NULL;
					r->next = p;		//尾插法
					r = p;
				}
				else if (ai == "{" || ai == "}") {
					node* p = new node;
					p->val = -1;
					p->id = -1;
					p->itemName = ai;
					p->nestedCnt = 0;
					p->ln = ln;
					p->moduleIn = moduleIn;
					p->next = NULL;
					r->next = p;		//尾插法
					r = p;
				}
			}
		}
	}
	return L;
}

/***************************
*
*第二遍扫描，实现去嵌套
*2019-04-23 问题：无法处理多层嵌套，当出现多层嵌套时，只能处理最里层
****************************/
void secondScan(node*&L){
	node *r, *p,*tail;
	node *nested = new node;			//此节点用来标记嵌套结构
	nested->next = NULL;
	if (L->next != NULL) {			//非空链表
		r = L->next;
		tail = L;
		while (tail->next != NULL) {		//tail指向链表尾节点
			tail = tail->next;
		}
	}
	else
		return;			//若L为空链表直接返回
	stack<St> st;		
	int level = 0, levelcnt = 0;		//嵌套层数
	int pre = 0, post = 0;
	while (r != NULL and r->nestedCnt==0) {	
		if (r->itemName == "{") {
			St c;
			St *m=&c;
			//St *m = new St;
			m->s = r->itemName;
			m->ln = r->ln;
			m->level = level++;
			st.push(*m);		//入栈
		}	
		else if (r->itemName == "}") {		
			p = L;
			St c;		
			c = st.top();		//返回栈顶元素，即与"}"对应的"{"
			st.pop();			//出栈
			St *m = &c;	
			if (m->level > 0) {			//说明有嵌套	
				pre = m->ln;			//嵌套开始所在行
				post = r->ln;			//嵌套结束所在行
				levelcnt++;				
				r = r->next;
				int cnt = 0;			//	当cnt置1时，表示为嵌套结构，将其置入表后
				node*q = NULL, *t = new node;
				while (p->next != NULL) {		//从表头开始扫描节点  
					
					if (p->next->ln == pre) {
						cnt = 1;
						q = p;
						//cout << q->itemName << endl;
					}			
					else if (p->next->ln == post + 1) {
						t->itemName = "Nested";
						t->id = 10;
						//cout << q->moduleIn << endl;
						t->moduleIn = q->moduleIn;
						t->val = -1;
						t->ln = pre;
						t->nestedCnt = levelcnt;
						t->next = q->next;
						q->next = t;
						break;
					}
						
					if (cnt == 1) {
						node *b = p->next;
						if(b->nestedCnt==0)
							b->nestedCnt = levelcnt;
						p->next = b->next;
						b->next = tail->next;				//将其插入到链表后部
						tail->next = b;
						tail = b;
					}
					else {
						p = p->next;
					}
						
				}
				level--;		//处理完一层
				continue;
			}
			
		}
		r = r->next;
	}	
}
/***************************
*第三遍扫描
*将用户自定义的标识符添加到
*链表头部
***************************/
void thridScan(node*&L) {
	node *q, *p,*n;
	string Name;
	if (L->next != NULL)
		p = L->next;
	else
		return;
	while (p->next->ln == 1)
		p = p->next;
	q = p;				//插入到q节点的后面
	n = q;
	int cnt = 0;	
	while (p != NULL) {
		Name = p->itemName;
		if (checkType(Name) >= 0 and checkType(Name) <= 4 ) {			//类型
			//cout << Name << endl;
			cnt = p->ln;
			//cout << cnt << endl;
			while (n->next->ln != cnt) {
				n = n->next;
			}
			//cout << n->next->itemName << endl;
			if (isupper(n->next->itemName[0])) {			//新定义的类型名
				node*r = n->next;
				r->val = 0;
				r->nestedCnt = 0;
				r->next->val = 0;
				r->next->nestedCnt = 0;
				n->next = r->next->next;
				r->next->next = q->next;
				q->next = r;
				q = r->next;
			}
		}
		p = p->next;
	}
}


int main() {
	node *ph[51], *pt[51];
	cout << "第二遍扫描效果" << endl;
	cout << "——————————————————" << endl;
	cout << "itemName    " << "id " << "ln " << "mIn " << "val  " <<"nestedCnt"<< endl;
	node*L = firstScan();
	secondScan(L);
	thridScan(L);
	node*r = L->next;
	while (r != NULL) {
		cout << setw(10) << r->itemName << "  " << r->id << "  " << r->ln << "  " << r->moduleIn << "  " << r->val << "   " << r->nestedCnt << endl;
		r = r->next;
	}
}