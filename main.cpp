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
	bool read;			//0表示未转换C,1表示已转换C
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

void del(string &str, const char c) {
	int len = str.length();
	if (len >= 1 and str[len - 1] == ',')
		str.erase(len - 1, 1);
}

//检查类型
int checkType(const string & s) {
	vector<string> type = { "INTEGER","BOOLEAN","ENUMERATED","BIT STRING","OCTET STRING","NULL","CHOCIE",
							"SEQUENCE","SEQUENCE OF" ,"DEFINITIONS" };
	for (int i = 0; i < 10; i++) {
		if (s == type[i])
			return i;
	}
	return -1;
}
/***************************
*
*第一遍扫描，建立动态链表
***************************/
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
		int index = 0;
		for (auto ai : evc) {
			del(ai, ',');
			index++;
			if (ai == "BIT" or ai == "OCTET") {
				ai += " STRING";
			}
			else if (ai == "SEQUENCE") {
				if (evc[index] == "OF") {
					ai += " OF";
				}
			}
			else if (ai == "STRING," or ai == "STRING" or ai == "OF") {
				continue;
			}
			int i = checkType(ai);
			if (i > -1) {		//表明该字符串是一个类型名或者模板名
				node* p = new node;
				p->val = -1;
				p->id = i;
				p->itemName = ai;
				p->nestedCnt = 0;
				p->ln = ln;
				p->moduleIn = moduleIn;
				p->read = false;
				p->next = NULL;
				r->next = p;			//尾插法  
				r = p;
			}
			else if (ai == "BEGIN") {
				moduleIn = true;//	表示后面的语法项都在模板内部
			}
			else if (ai == "END") {
				moduleIn = false;// 表示模板结束
				node* p = new node;
				p->val = -1;
				p->id = -1;
				p->itemName = ai;
				p->nestedCnt = 0;
				p->ln = ln;
				p->moduleIn = moduleIn;
				p->read = false;
				p->next = NULL;
				r->next = p;			//尾插法  
				r = p;
			}
			else {
				if (isupper(ai[0])) {	//表示为定义的类型名
					node* p = new node;
					p->val = -1;
					p->id = -2;
					del(ai, ',');
					p->itemName = ai;
					p->nestedCnt = 0;
					p->ln = ln;
					p->moduleIn = moduleIn;
					p->read = false;
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
					p->read = false;
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
					p->read = false;
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
*
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
						t->read = false;
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
			
			if (isupper(n->next->itemName[0])) {			//新定义的类型名
				//cout << n->next->itemName << endl;
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
			n = q;
		}
		p = p->next;
	}
}

int write2file(string message, fstream &file) {
	if (!file) {
		cout << "file error!" << endl;
		return 0;
	}
	else {
		file << message << endl;
		return 1;
	}
}

/**************************
*第四遍扫描
*将动态链表内容转为C语言的数据结构
**************************/
node* fourScan(node*&L, node*&L1,fstream &file, int level) {
	node *p,*q;
	if (L1->next != NULL)
		p = L1->next;
	else
		return NULL;
	string sj = "";
	for (int i = 0; i < level; i++) {
		sj += "	";
	}
	//cout << p->next->id << endl;
	while (p->next != NULL and p->itemName!="END") {
		if (p->next->id == 0 and p->next->read == 0) {		//整型
			if (p->id == -2) {
				string message = sj + "typedef	int	" + p->itemName + ";";
				if (write2file(message, file)) {
					p->read = 1;
					p->next->read = 1;
				}	
			}
			else if (p->id == -1) {
				string message = sj + "int	" + p->itemName + ";";
				if (write2file(message, file)) {
					p->read = 1;
					p->next->read = 1;
				}
			}
		}
		else if (p->next->id == 1 and p->next->read == 0) {		//布尔型
			if (p->id == -2) {
				string message = sj + "typedef bool	" + p->itemName+";";
				if (write2file(message, file)) {
					p->read = 1;
					p->next->read = 1;
				}
			}
			else if (p->id == -1) {
				string message = sj + "bool	" + p->itemName + ";";
				if (write2file(message, file)) {
					p->read = 1;
					p->next->read = 1;
				}
			}
		}
		else if (p->next->id == 3 and p->next->read == 0) {			//比特串
			if (p->id == -2) {
				string message = sj + "typedef	char	*" + p->itemName + "Bit;";
				if (write2file(message, file))
					p->read = 1;
			}
			else if (p->id == -1) {
				string message = sj + "char	*" + p->itemName + "Bit;";
				if (write2file(message, file))
					p->read = 1;
			}
		}
		else if (p->next->id == 4 and p->next->read == 0) {			//字节串
			if (p->id == -2) {										//新定义的类型名
				string message = sj + "typedef	char	*" + p->itemName + "Octet;";
				if (write2file(message, file))
					p->read = 1;
			}
			else if (p->id == -1) {									//使用类型定义变量
				string message = sj + "char	*" + p->itemName + "Octet;";
				if (write2file(message, file))
					p->read = 1;
			}

		}
		else if (p->next->id == -2 and p->id == -1 and p->next->read == 0) {			//新定义的类型名

			string message = sj + p->next->itemName + "	" + p->itemName + ";";
			if (write2file(message, file)) {
				p->next->read = 1;
				p->read = 1;
			}
				
		}
		else if (p->next->id == 10 and p->next->read == 0) {			//嵌套结构
			//cout << "sdfasdf" << endl;
			int nested = p->next->nestedCnt;
			//cout << nested << endl;
			node *q = L;
			while (q->next->nestedCnt != nested or q->next->id == 10)
				q = q->next;
			//cout << q->next->itemName << endl;

			node *r = fourScan(L, q, file, level);

		}
		else if (p->next->id == 7 and p->next->read == 0) {			//结构体
			string message = sj + "typedef	struct	" + p->itemName + p->next->next->itemName;
			if (write2file(message, file)) {
				p->next->read = 1;
				p->read = 1;
			}
			//cout << p->next->next->itemName << endl;
			node *r = fourScan(L, p->next->next, file, level+1);
			
		}
		else if (p->next->itemName == "}" and p->next->read == 0) {
			string sj = "";
			//cout << level << endl;
			for (int i = 0; i < level-1; i++) {
				sj += "	";
			}
			string message = sj + p->next->itemName+";";
			if (write2file(message, file)) {
				p->next->read = 1;
			}
			return p->next->next;
		}
		p = p->next;
	}
	return NULL;
}




int main() {
	node *p,*q;
	cout << "第三遍扫描效果" << endl;
	cout << "——————————————————" << endl;
	cout << "itemName    " << "id " << "ln " << "mIn " << "val  " <<"nestedCnt"<< "   read;"<<endl;
	node*L = firstScan();
	secondScan(L);
	//thridScan(L);
	p = L->next;
	string fileName;
	fstream file;
	if (p->next->id == 9) {		//表明为模块定义
		fileName = p->itemName;
		fileName += ".c";
		ofstream fileOut(fileName, ios::out);
		if (!fileOut) {
			cout << "fileout error!" << endl;
		}
		else {
			fileOut.close();

		}
		file.open(fileName, ios::out);
	}
	q = fourScan(L, L, file, 0);
	//file << "}" << endl;
	file.close();
	node*r = L->next;
	while (r != NULL) {
		cout << setw(10) << r->itemName << "  " << r->id << "  " << r->ln << "  " << r->moduleIn << "  " << r->val << "   " << r->nestedCnt <<"          "<<r->read<< endl;
		r = r->next;
	}
}