#include<iostream>
#include<fstream>
#include<string>
#include<vector>
#include<iomanip>
#include<stack>
using namespace std;
//�﷨����
struct node {
	int id;				//��ʶ���﷨�������
	string itemName;	//��
	int nestedCnt;		//Ƕ�ױ�־��ͬһǶ�׽ṹ�е������﷨�������ͬ
	int val;			//0��ʾ���Ͷ��壬1��ʾ����ʹ��
	int ln;				//ָ�����﷨��������
	bool moduleIn;		
	bool read;			//0��ʾδת��C,1��ʾ��ת��C
	struct node *next;	//ָ����һ���ڵ�
};
//��ջ�����ݽڵ�
struct St {
	string s;
	int ln;
	int level;
};
/***************************
*
*��һ��ɨ�裬������̬����
*
****************************/
/*
//����
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
*���»��ֺ���
*/
void split(const string &str, vector<string> &vec) {
	string stmp = "";
	string::size_type pos = 0, prev_pos = 0, i = 0;
	int cnt = 0;			//1��ʾ�ɵ��ʻ�����������0��ʾ�ո�
	vec.clear();
	while (i < str.length()) {
		if (!isspace(str[i]) && cnt == 0) {	//��ʾ�Ӵ˿�ʼ�����ַ�
			cnt = 1;
			pos = i;
		}
		else if (isspace(str[i]) && cnt == 1) {	//��ʾ�Ӵ˿�ʼ�ַ�����
			cnt = 0;
			prev_pos = i;
			stmp = str.substr(pos, prev_pos - pos);
			vec.push_back(stmp);				//���ַ���������
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

//�������
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
*��һ��ɨ�裬������̬����
***************************/
node* firstScan() {
	node* L = new node;//ͷ���
	node* r = L;
	L->next = NULL;
	fstream file;
	file.open("test.asn1", ios::in);
	if (!file) {
		cout << "�ļ���ʧ��" << endl;
		exit(0);
	}
	char buffer[256];
	string str;
	int ln = 0;
	bool moduleIn = false;
	while (!file.eof()) {			//�ж��Ƿ񵽴��ļ�ĩβ
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
			if (i > -1) {		//�������ַ�����һ������������ģ����
				node* p = new node;
				p->val = -1;
				p->id = i;
				p->itemName = ai;
				p->nestedCnt = 0;
				p->ln = ln;
				p->moduleIn = moduleIn;
				p->read = false;
				p->next = NULL;
				r->next = p;			//β�巨  
				r = p;
			}
			else if (ai == "BEGIN") {
				moduleIn = true;//	��ʾ������﷨���ģ���ڲ�
			}
			else if (ai == "END") {
				moduleIn = false;// ��ʾģ�����
				node* p = new node;
				p->val = -1;
				p->id = -1;
				p->itemName = ai;
				p->nestedCnt = 0;
				p->ln = ln;
				p->moduleIn = moduleIn;
				p->read = false;
				p->next = NULL;
				r->next = p;			//β�巨  
				r = p;
			}
			else {
				if (isupper(ai[0])) {	//��ʾΪ�����������
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
					r->next = p;		//β�巨
					r = p;
				}
				else if (islower(ai[0])) {	//��ʾΪ����ı�����
					node* p = new node;
					p->val = -1;
					p->id = -1;
					p->itemName = ai;
					p->nestedCnt = 0;
					p->ln = ln;
					p->moduleIn = moduleIn;
					p->read = false;
					p->next = NULL;
					r->next = p;		//β�巨
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
					r->next = p;		//β�巨
					r = p;
				}
			}
		}
	}
	return L;
}
/***************************
*
*�ڶ���ɨ�裬ʵ��ȥǶ��
*
****************************/
void secondScan(node*&L){
	node *r, *p,*tail;
	node *nested = new node;			//�˽ڵ��������Ƕ�׽ṹ
	nested->next = NULL;
	if (L->next != NULL) {			//�ǿ�����
		r = L->next;
		tail = L;
		while (tail->next != NULL) {		//tailָ������β�ڵ�
			tail = tail->next;
		}
	}
	else
		return;			//��LΪ������ֱ�ӷ���
	stack<St> st;		
	int level = 0, levelcnt = 0;		//Ƕ�ײ���
	int pre = 0, post = 0;
	while (r != NULL and r->nestedCnt==0) {	
		if (r->itemName == "{") {
			St c;
			St *m=&c;
			//St *m = new St;
			m->s = r->itemName;
			m->ln = r->ln;
			m->level = level++;
			st.push(*m);		//��ջ
		}	
		else if (r->itemName == "}") {		
			p = L;
			St c;		
			c = st.top();		//����ջ��Ԫ�أ�����"}"��Ӧ��"{"
			st.pop();			//��ջ
			St *m = &c;	
			if (m->level > 0) {			//˵����Ƕ��	
				pre = m->ln;			//Ƕ�׿�ʼ������
				post = r->ln;			//Ƕ�׽���������
				levelcnt++;				
				r = r->next;
				int cnt = 0;			//	��cnt��1ʱ����ʾΪǶ�׽ṹ������������
				node*q = NULL, *t = new node;
				while (p->next != NULL) {		//�ӱ�ͷ��ʼɨ��ڵ�  
					
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
						b->next = tail->next;				//������뵽�����
						tail->next = b;
						tail = b;
					}
					else {
						p = p->next;
					}
						
				}
				level--;		//������һ��
				continue;
			}
			
		}
		r = r->next;
	}	
}
/***************************
*������ɨ��
*���û��Զ���ı�ʶ����ӵ�
*����ͷ��
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
	q = p;				//���뵽q�ڵ�ĺ���
	n = q;
	int cnt = 0;	
	while (p != NULL) {
		Name = p->itemName;
		if (checkType(Name) >= 0 and checkType(Name) <= 4 ) {			//����
			//cout << Name << endl;
			cnt = p->ln;
			//cout << cnt << endl;
			while (n->next->ln != cnt) {
				n = n->next;
			}
			
			if (isupper(n->next->itemName[0])) {			//�¶����������
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
*���ı�ɨ��
*����̬��������תΪC���Ե����ݽṹ
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
		if (p->next->id == 0 and p->next->read == 0) {		//����
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
		else if (p->next->id == 1 and p->next->read == 0) {		//������
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
		else if (p->next->id == 3 and p->next->read == 0) {			//���ش�
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
		else if (p->next->id == 4 and p->next->read == 0) {			//�ֽڴ�
			if (p->id == -2) {										//�¶����������
				string message = sj + "typedef	char	*" + p->itemName + "Octet;";
				if (write2file(message, file))
					p->read = 1;
			}
			else if (p->id == -1) {									//ʹ�����Ͷ������
				string message = sj + "char	*" + p->itemName + "Octet;";
				if (write2file(message, file))
					p->read = 1;
			}

		}
		else if (p->next->id == -2 and p->id == -1 and p->next->read == 0) {			//�¶����������

			string message = sj + p->next->itemName + "	" + p->itemName + ";";
			if (write2file(message, file)) {
				p->next->read = 1;
				p->read = 1;
			}
				
		}
		else if (p->next->id == 10 and p->next->read == 0) {			//Ƕ�׽ṹ
			//cout << "sdfasdf" << endl;
			int nested = p->next->nestedCnt;
			//cout << nested << endl;
			node *q = L;
			while (q->next->nestedCnt != nested or q->next->id == 10)
				q = q->next;
			//cout << q->next->itemName << endl;

			node *r = fourScan(L, q, file, level);

		}
		else if (p->next->id == 7 and p->next->read == 0) {			//�ṹ��
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
	cout << "������ɨ��Ч��" << endl;
	cout << "������������������������������������" << endl;
	cout << "itemName    " << "id " << "ln " << "mIn " << "val  " <<"nestedCnt"<< "   read;"<<endl;
	node*L = firstScan();
	secondScan(L);
	//thridScan(L);
	p = L->next;
	string fileName;
	fstream file;
	if (p->next->id == 9) {		//����Ϊģ�鶨��
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