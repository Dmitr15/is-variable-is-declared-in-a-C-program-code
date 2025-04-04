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
	if (vec[row].at(pos) == '/' && vec[row].at(pos + 1) == '*')//если текущий символ / и следующий * 
	{
		pos = vec[row].find("*/", pos + 1, 2);//ищем вхождение */
		if (pos != std::string::npos)//если не найдено
			return pos + 2;
		do
		{
			row++;
			pos = vec[row].find("*/", pos + 1, 2);
		} while (pos == std::string::npos && row < vec.size());//пока не найдены символы */ и не последняя строка
		return pos + 2;
	}
	else if (vec[row].at(pos) == '/' && vec[row].at(pos + 1) == '/')//если текущий символ / и следующий /
	{
		while (row < vec.size() && vec[row].at(vec[row].size() - 1) == '\\')//строка с переносом
			row++;
		pos = vec[row].size();//переходим в конец строки		
		return pos;
	}
	else if (vec[row].at(pos) == '\"')//если текущий символ "
	{
		int bscount = 0, qpos = pos;
		while (qpos >= 0 && vec[row].at(qpos--) == '\\')// проверка на наличие четного кол-ва символа '\' перед открывающей ", если их число не четное, то считаем что последний символ \ экранирует "
			bscount++;
		if (!(bscount & 1))// если " не экранирована 
		{
			do
			{
				pos = vec[row].find('\"', pos + 1);//ищем " начиная со следующего символа
				if (pos == std::string::npos)//если не найден  символ " 
				{
					while (row < vec.size() && vec[row].at(vec[row].size() - 1) == '\\')//пропускаем строки если в конце обратный слеш
						row++;
					pos = vec[row].find('\"', pos + 1);//ищем " начиная с начала строки
				}
				bscount = 0, qpos = pos - 1;
				while (qpos >= 0 && vec[row].at(qpos--) == '\\')// проверка на наличие четного кол-ва символа '\' перед закрывающей ", если их число не четное, то считаем что последний символ \ экранирует "
					bscount++;
			} while ((bscount & 1));//пока не четное число '\'	
		}
		pos++;
	}
	return pos;
}


int  CheckType(std::vector<std::string>& vec, int& row, int pos, int& valuedType)
{
	int oldpos = pos;
	struct STYPE { const char* Type; int SzType; };
	STYPE STypes1[] = { {"struct", sizeof("struct") - 1},{"enum", sizeof("enum") - 1}, {"union", sizeof("union") - 1} };//Массив структур с пользовательскими типами данных
	STYPE STypes[] = { {"char", sizeof("char") - 1},{"int", sizeof("int") - 1}, {"float", sizeof("float") - 1},{"double", sizeof("double") - 1},{"void", sizeof("void") - 1}, {"DWORD", sizeof("DWORD") - 1},{"bool", sizeof("bool") - 1},{"BYTE", sizeof("BYTE") - 1},{"short", sizeof("short") - 1},{"long", sizeof("long") - 1},{"unsigned", sizeof("unsigned") - 1},{"signed", sizeof("signed") - 1} };//Массив структур с типами данных
	for (int i = 0; i < ArrSz(STypes) && !valuedType; i++)
		if (vec[row].compare(pos, STypes[i].SzType, STypes[i].Type) == 0)//сравниваем слово из текущей позиции с одним из стандартных типов данных  
		{
			valuedType = 1;
			pos += STypes[i].SzType;
		}
	if (!valuedType)// если тип не найден
	{
		for (int i = 0; i < ArrSz(STypes1) && !valuedType; i++) //сравниваем слово из текущей позиции с одним из пользовательских типов данных
			if (vec[row].compare(pos, STypes1[i].SzType, STypes1[i].Type) == 0)
			{
				valuedType = 1;
				pos += STypes1[i].SzType;
			}
		if (valuedType)//если пользовательский тип найден
		{
			while (pos < vec[row].size() && vec[row].at(pos) == ' ')//пропускаем пробелы
				pos++;
			while (pos < vec[row].size() && (isalnum(vec[row].at(pos)) || vec[row].at(pos) == '_'))//пропускаем имя структуры
				pos++;
			int oldrow = row;
			int start_brackets = 0;
			int current_pos = pos;
			pos = vec[row].find('{', pos);//ищем { на текущей строке
			if (pos == std::string::npos && vec[row].find(';', current_pos) == std::string::npos)//если { на текущей строке не найдена
			{
				do
				{
					row++;
					if (row < vec.size())
						pos = vec[row].find("{", pos + 1, 1);
				} while (pos == std::string::npos && row < vec.size() - 1);//ищем { построчно пока не найдем и не дойдем до последне строки 
			}
			if (pos != std::string::npos) {//если { на текущей строке найдена
				pos = vec[row].find('}', pos);//ищем } на текущей строке
				start_brackets = 1;//ставим флаг нахождения }
			}
			if (pos != std::string::npos)//если } на текущей строке найдена
			{
				pos++;
				while (vec[row].at(pos) == ' ' && pos < vec[row].size())// пропускаем пробелы
					pos++;
				if (vec[row].at(pos) == ';')// если на текущей позиции находится ;
					pos++;
			}
			else if (start_brackets)// если } не найдена
			{
				do
				{
					row++;
					pos = vec[row].find("}", pos + 1, 1);// ищем } на текущей строке
				} while (pos == std::string::npos && row < vec.size());// ищем } построчно пока не найдем и не дойдем до последне строки
			}
			if (!start_brackets)// если } найдена
			{
				pos = current_pos;
				row = oldrow;
			}
		}
	}
	if (oldpos == pos)// если позиция не изменилась
		pos++;
	return pos;
}

int FindVar(std::vector<std::string>& vec, const std::string& var)
{
	int find = 0, pos = 0, row = 0, allrows = 0, type = 0, typepos = 0, typerow;
	do
	{
		if (vec[row].size() == 0) {//пропуск пустой строки
			row++;
			if (row == vec.size()) { break; }
			else { continue; }
		}
		if (pos <= (vec[row].size() - 2) && vec[row].at(pos) == '/' && vec[row].at(pos + 1) == '*')// проверяем на многострочный комментарий      
			pos = SkipComments(vec, row, pos);
		else
			if (pos <= (vec[row].size() - 2) && vec[row].at(pos) == '/' && vec[row].at(pos + 1) == '/')// проверяем на однострочный комментарий        
				pos = SkipComments(vec, row, pos);
			else if (vec[row].at(pos) == '\"')// проверяем на наличие двойных ковычек              
				pos = SkipComments(vec, row, pos);
			else if (vec[row].at(pos) == ';' || vec[row].at(pos) == '(' || vec[row].at(pos) == ')')// если текущий символ ; или ( или )
			{
				type = 0;//сбрасываем найденный тип
				pos++;//переходим к следующему символу
				typepos = 0;
				typerow = 0;
			}
			else if (!type && isalpha(vec[row].at(pos)))//если тип не найден и текущий символ буква
			{
				pos = CheckType(vec, row, pos, type);
				if (type) {//если тип найден
					typepos = pos;
					typerow = row;
				}
			}
			else if (type && (isalpha(vec[row].at(pos)) || vec[row].at(pos) == '_'))//если тип найден и текущий символ буква или цифра или _
			{
				if (vec[row].compare(pos, var.size(), var) == 0)//если найденная переменная совпадает с искомой 
				{
					int tpos = pos;
					pos += var.size();//пропускаем слово      					
					if (pos < vec[row].size() && !isalnum(vec[row].at(pos)) && vec[row].at(pos) != '_')//если не последний символ строки и текущий символ не буква и не цифра и не _
					{
						while (pos < vec[row].size() && vec[row].at(pos) == ' ')// пропускаем пробелы
							pos++;
						if (vec[row].at(pos) != '(')//если текуций символ не равен (
							find = 1;//ставим признак обнаружения переменной
					}
					else if (pos == vec[row].size())//если пройдены все символы в строке
						find = 1;  //ставим признак обнаружения переменной      
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
					while (pos < vec[row].size() && (isalnum(vec[row].at(pos)) || vec[row].at(pos) == '_'))//если текуций символ буква или цифра или _ и все символы строки не пройдены
						pos++;
			}
			else
				pos++;
		if (pos == vec[row].size() && row == vec.size() - 1)//если пройдены все символы строки и все строки
			allrows = 1;
		else
			if (row < vec.size() - 1 && pos == vec[row].size())//если пройдены все символы строки но не все строки
			{
				pos = 0;
				row++;
			}
	} while (!allrows && !find);
	return find;//возвращаем признак обнаружения переменной
}

