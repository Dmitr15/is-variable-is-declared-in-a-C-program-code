#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#define ArrSz(x)    (sizeof(x)/sizeof(x[0]))

using namespace std;

int SkipComments(std::vector<std::string>& vec, int& row, int pos)
{
	if (vec[row].at(pos) == '/' && vec[row].at(pos + 1) == '*')//���� ������� ������ / � ��������� * 
	{
		pos = vec[row].find("*/", pos + 1, 2);//���� ��������� */
		if (pos != std::string::npos)//���� �� �������
			return pos + 2;
		do
		{
			row++;
			pos = vec[row].find("*/", pos + 1, 2);
		} while (pos == std::string::npos && row < vec.size());//���� �� ������� ������� */ � �� ��������� ������
		return pos + 2;
	}
	else if (vec[row].at(pos) == '/' && vec[row].at(pos + 1) == '/')//���� ������� ������ / � ��������� /
	{
		while (row < vec.size() && vec[row].at(vec[row].size() - 1) == '\\')//������ � ���������
			row++;
		pos = vec[row].size();//��������� � ����� ������		
		return pos;
	}
	else if (vec[row].at(pos) == '\"')//���� ������� ������ "
	{
		int bscount = 0, qpos = pos;
		while (qpos >= 0 && vec[row].at(qpos--) == '\\')// �������� �� ������� ������� ���-�� ������� '\' ����� ����������� ", ���� �� ����� �� ������, �� ������� ��� ��������� ������ \ ���������� "
			bscount++;
		if (!(bscount & 1))// ���� " �� ������������ 
		{
			do
			{
				pos = vec[row].find('\"', pos + 1);//���� " ������� �� ���������� �������
				if (pos == std::string::npos)//���� �� ������  ������ " 
				{
					while (row < vec.size() && vec[row].at(vec[row].size() - 1) == '\\')//���������� ������ ���� � ����� �������� ����
						row++;
					pos = vec[row].find('\"', pos + 1);//���� " ������� � ������ ������
				}
				bscount = 0, qpos = pos - 1;
				while (qpos >= 0 && vec[row].at(qpos--) == '\\')// �������� �� ������� ������� ���-�� ������� '\' ����� ����������� ", ���� �� ����� �� ������, �� ������� ��� ��������� ������ \ ���������� "
					bscount++;
			} while ((bscount & 1));//���� �� ������ ����� '\'	
		}
		pos++;
	}
	return pos;
}


int  CheckType(std::vector<std::string>& vec, int& row, int pos, int& valuedType)
{
	int oldpos = pos;
	struct STYPE { const char* Type; int SzType; };
	STYPE STypes1[] = { {"struct", sizeof("struct") - 1},{"enum", sizeof("enum") - 1}, {"union", sizeof("union") - 1} };//������ �������� � ����������������� ������ ������
	STYPE STypes[] = { {"char", sizeof("char") - 1},{"int", sizeof("int") - 1}, {"float", sizeof("float") - 1},{"double", sizeof("double") - 1},{"void", sizeof("void") - 1}, {"DWORD", sizeof("DWORD") - 1},{"bool", sizeof("bool") - 1},{"BYTE", sizeof("BYTE") - 1},{"short", sizeof("short") - 1},{"long", sizeof("long") - 1},{"unsigned", sizeof("unsigned") - 1},{"signed", sizeof("signed") - 1} };//������ �������� � ������ ������
	for (int i = 0; i < ArrSz(STypes) && !valuedType; i++)
		if (vec[row].compare(pos, STypes[i].SzType, STypes[i].Type) == 0)//���������� ����� �� ������� ������� � ����� �� ����������� ����� ������  
		{
			valuedType = 1;
			pos += STypes[i].SzType;
		}
	if (!valuedType)// ���� ��� �� ������
	{
		for (int i = 0; i < ArrSz(STypes1) && !valuedType; i++) //���������� ����� �� ������� ������� � ����� �� ���������������� ����� ������
			if (vec[row].compare(pos, STypes1[i].SzType, STypes1[i].Type) == 0)
			{
				valuedType = 1;
				pos += STypes1[i].SzType;
			}
		if (valuedType)//���� ���������������� ��� ������
		{
			while (pos < vec[row].size() && vec[row].at(pos) == ' ')//���������� �������
				pos++;
			while (pos < vec[row].size() && (isalnum(vec[row].at(pos)) || vec[row].at(pos) == '_'))//���������� ��� ���������
				pos++;
			int oldrow = row;
			int start_brackets = 0;
			int current_pos = pos;
			pos = vec[row].find('{', pos);//���� { �� ������� ������
			if (pos == std::string::npos && vec[row].find(';', current_pos) == std::string::npos)//���� { �� ������� ������ �� �������
			{
				do
				{
					row++;
					if (row < vec.size())
						pos = vec[row].find("{", pos + 1, 1);
				} while (pos == std::string::npos && row < vec.size() - 1);//���� { ��������� ���� �� ������ � �� ������ �� �������� ������ 
			}
			if (pos != std::string::npos) {//���� { �� ������� ������ �������
				pos = vec[row].find('}', pos);//���� } �� ������� ������
				start_brackets = 1;//������ ���� ���������� }
			}
			if (pos != std::string::npos)//���� } �� ������� ������ �������
			{
				pos++;
				while (vec[row].at(pos) == ' ' && pos < vec[row].size())// ���������� �������
					pos++;
				if (vec[row].at(pos) == ';')// ���� �� ������� ������� ��������� ;
					pos++;
			}
			else if (start_brackets)// ���� } �� �������
			{
				do
				{
					row++;
					pos = vec[row].find("}", pos + 1, 1);// ���� } �� ������� ������
				} while (pos == std::string::npos && row < vec.size());// ���� } ��������� ���� �� ������ � �� ������ �� �������� ������
			}
			if (!start_brackets)// ���� } �������
			{
				pos = current_pos;
				row = oldrow;
			}
		}
	}
	if (oldpos == pos)// ���� ������� �� ����������
		pos++;
	return pos;
}

int FindVar(std::vector<std::string>& vec, const std::string& var)
{
	int find = 0, pos = 0, row = 0, allrows = 0, type = 0, typepos = 0, typerow;
	do
	{
		if (vec[row].size() == 0) {//������� ������ ������
			row++;
			if (row == vec.size()) { break; }
			else { continue; }
		}
		if (pos <= (vec[row].size() - 2) && vec[row].at(pos) == '/' && vec[row].at(pos + 1) == '*')// ��������� �� ������������� �����������      
			pos = SkipComments(vec, row, pos);
		else
			if (pos <= (vec[row].size() - 2) && vec[row].at(pos) == '/' && vec[row].at(pos + 1) == '/')// ��������� �� ������������ �����������        
				pos = SkipComments(vec, row, pos);
			else if (vec[row].at(pos) == '\"')// ��������� �� ������� ������� �������              
				pos = SkipComments(vec, row, pos);
			else if (vec[row].at(pos) == ';' || vec[row].at(pos) == '(' || vec[row].at(pos) == ')')// ���� ������� ������ ; ��� ( ��� )
			{
				type = 0;//���������� ��������� ���
				pos++;//��������� � ���������� �������
				typepos = 0;
				typerow = 0;
			}
			else if (!type && isalpha(vec[row].at(pos)))//���� ��� �� ������ � ������� ������ �����
			{
				pos = CheckType(vec, row, pos, type);
				if (type) {//���� ��� ������
					typepos = pos;
					typerow = row;
				}
			}
			else if (type && (isalpha(vec[row].at(pos)) || vec[row].at(pos) == '_'))//���� ��� ������ � ������� ������ ����� ��� ����� ��� _
			{
				if (vec[row].compare(pos, var.size(), var) == 0)//���� ��������� ���������� ��������� � ������� 
				{
					int tpos = pos;
					pos += var.size();//���������� �����      					
					if (pos < vec[row].size() && !isalnum(vec[row].at(pos)) && vec[row].at(pos) != '_')//���� �� ��������� ������ ������ � ������� ������ �� ����� � �� ����� � �� _
					{
						while (pos < vec[row].size() && vec[row].at(pos) == ' ')// ���������� �������
							pos++;
						if (vec[row].at(pos) != '(')//���� ������� ������ �� ����� (
							find = 1;//������ ������� ����������� ����������
					}
					else if (pos == vec[row].size())//���� �������� ��� ������� � ������
						find = 1;  //������ ������� ����������� ����������      
					if (find)
					{
						if (tpos)
							tpos--;
						while (tpos && vec[row].at(tpos) == ' ')
							tpos--;
						if (vec[row].at(tpos) == '=' || vec[row].at(tpos) == '+' || vec[row].at(tpos) == '-')
							find = 0;
						if (vec[row].at(tpos) == '*')
						{
							tpos--;
							while (tpos && vec[row].at(tpos) == ' ')
								tpos--;
							if (typerow == row && tpos > typepos && vec[row].at(tpos) != ',')
								find = 0;
						}
					}
				}
				else
					while (pos < vec[row].size() && (isalnum(vec[row].at(pos)) || vec[row].at(pos) == '_'))//���� ������� ������ ����� ��� ����� ��� _ � ��� ������� ������ �� ��������
						pos++;
			}
			else
				pos++;
		if (pos == vec[row].size() && row == vec.size() - 1)//���� �������� ��� ������� ������ � ��� ������
			allrows = 1;
		else
			if (row < vec.size() - 1 && pos == vec[row].size())//���� �������� ��� ������� ������ �� �� ��� ������
			{
				pos = 0;
				row++;
			}
	} while (!allrows && !find);
	return find;//���������� ������� ����������� ����������
}

