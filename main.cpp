#define _CRT_SECURE_NO_WARNINGS
#include<SFML/Graphics.hpp>
#include<string.h>
#include<iostream>
#include<fstream>
#include<Windows.h>
#include<direct.h>

sf::Vector2f deskSize;
sf::Vector2i cursorPos, cursorTxtPos, cursorOffset = sf::Vector2i(5, 2), scroll = sf::Vector2i(0, 0), maxTextView;
sf::Font font0;
sf::Text sfText;
sf::Color def_colors[15] = { sf::Color(0, 51, 51, 255), sf::Color(255, 255, 255, 255), sf::Color(255, 255, 255, 255), sf::Color(255, 155, 155, 255), sf::Color(255, 255, 0, 255), sf::Color(0, 150, 0, 255), sf::Color(255, 128, 0, 255), sf::Color(0, 255, 255, 255), sf::Color(0, 125, 175, 255), sf::Color(0, 255, 100, 255), sf::Color(125, 0, 255, 255), sf::Color(125, 125, 125, 225), sf::Color(100, 100, 150, 225) }, colors[15] = { sf::Color(0, 51, 51, 255), sf::Color(255, 255, 255, 255), sf::Color(255, 255, 255, 255), sf::Color(255, 155, 155, 255), sf::Color(255, 255, 0, 255), sf::Color(0, 150, 0, 255), sf::Color(255, 128, 0, 255), sf::Color(0, 255, 255, 255), sf::Color(0, 125, 175, 255), sf::Color(0, 255, 100, 255), sf::Color(125, 0, 255, 255), sf::Color(125, 125, 125, 225), sf::Color(100, 100, 150, 225) };
sf::RectangleShape textBg(sf::Vector2f(11.0f, 20.0f)), generalBg;
sf::Texture tx, icon_tx;
sf::Sprite sp, icon_sp;

PROCESS_INFORMATION codeProcess;

struct Area
{
	sf::Vector2i start, end;
}codeSelection;
struct ActionList
{
	int line, addL;
	char before[1001], after[1001];
	bool forward = false;
} undoList[100];

bool b_title = true, b_file = false, b_edit = false, b_settingsEdt = false, b_settingsEnv = false, b_settingsOth = false, b_newFile = false, b_openFile = false, b_evalCMD = true, b_cursor = false, b_code = false, b_saveAs = false, b_saved = false, b_running = false, b_saveBefore = false, b_error0 = false, b_warning0 = false, b_select1 = false, b_select2 = false, b_verticalCursor = false, b_blinkerCursor = true, b_rtUpdate = false, b_blueFilter = false, b_find = false, b_findCase = false;
char userCode[1000][1001], codeName[256], codeLocation[256], projectName[256], afritLocation[256], recentFiles[10][256];
int codeLines, recentCnt, saveBefOut = -1, saveBefInh = 0, warn0Inh = 0, undoCnt = 0, undoPos = 0, errorList[1000];

void my_tok(char s[], char dlm[], char aux[])
{
	static int _i;
	int i = 0;
	static char delim[100];
	if (dlm != NULL) // first call
		strcpy(delim, dlm), _i = 0;
	if (s[_i] == 0 || s + _i == NULL) // null parameter or end of array
		aux[0] = 0;
	else
	{
		while (s[_i + i] && !strchr(delim, s[_i + i])) i++;
		strcpy(aux, s + _i);
		if (i == 0) i++;
		aux[i] = 0;
		_i += i;
	}
}
void print(char text[], int X, int Y, sf::Color bgColor, sf::Color fgColor, sf::RenderWindow& window) //handling multiline text
{
	textBg.setFillColor(bgColor);
	sfText.setPosition(sf::Vector2f(X * 11, Y * 22 - 4));
	sfText.setFillColor(fgColor);

	char sText[500], *p;
	strcpy(sText, text);
	p = strtok(sText, "\n");
	int i = 0;
	while (p != NULL)
	{
		textBg.setSize(sf::Vector2f((strlen(p)) * 11, 22));
		textBg.setPosition(sf::Vector2f(X * 11, (Y + i) * 22));
		window.draw(textBg);
		p = strtok(NULL, "\n");
		i++;
	}
	std::string str; str += text; sfText.setString(str); //setting the string

	window.draw(sfText);
}
void w_Print(char text[], int X, int Y, sf::Color bgColor, sf::Color fgColor, sf::RenderWindow& window) //handling monoline text
{
	if (bgColor != colors[0])
	{
		textBg.setFillColor(bgColor);
		textBg.setSize(sf::Vector2f(strlen(text) * 11, 20));
		textBg.setPosition(sf::Vector2f(X * 11, Y * 20));
		window.draw(textBg);
	}
	sfText.setPosition(sf::Vector2f(X * 11 + 1, Y * 20 - 2));
	sfText.setFillColor(fgColor);
	std::string str; str += text; sfText.setString(str);
	window.draw(sfText);
}
void clearSelect()
{
	codeSelection.start.x = -1;
	codeSelection.start.y = -1;
	codeSelection.end.x = -1;
	codeSelection.end.y = -1;
}
bool fLoadCode(char filename[])
{
	std::ifstream fileInput(filename);
	if (!fileInput.is_open()) return false;
	codeLines = 0;
	while (fileInput.getline(userCode[codeLines], 1000)) codeLines++;
	fileInput.close();
	return true;
}
void addRecentFile(char filenamet[])
{
	int i;
	if (filenamet[0] != 0)
	{
		bool ok = true;
		for (i = 0; i < recentCnt && ok; i++)
			if (!strcmp(filenamet, recentFiles[i]))
				ok = false;

		if (ok)
		{
			recentCnt++;
			for (i = recentCnt - 1; i > 0; i--)
				strcpy(recentFiles[i], recentFiles[i - 1]);
			strcpy(recentFiles[0], filenamet);
		}
	}
	int j;
	for (i = 0; i < recentCnt; i++) // empy lines get inserted SOMEHOW, this is the only way that solves the mf
		if (recentFiles[i][0] == 0)
		{
			for (j = i; i < recentCnt; i++)
				strcpy(recentFiles[i], recentFiles[i + 1]);
			recentCnt--;
		}
}
void initCode(char filename[])
{
	b_code = true, b_openFile = false, b_file = false, b_title = false, b_saved = true, b_cursor = true;
	if (userCode[0][0])
		cursorTxtPos = sf::Vector2i(strlen(userCode[codeLines - 1]), codeLines - 1);
	else
		cursorTxtPos = sf::Vector2i(0, 0), codeLines = 1;
	char *p = strrchr(filename, '\\');
	if (p)
	{
		strncpy(codeLocation, filename, strlen(filename) - strlen(p) + 1);
		strcpy(codeName, p + 1);
	}
	else
	{
		strcpy(codeLocation, afritLocation);
		strcpy(codeName, filename);
	}

	strcpy(filename, codeName);
	strcpy(projectName, strtok(filename, "."));
	memset(filename, 0, 101);

	char aux1[256];
	strcpy(aux1, codeLocation);
	if (aux1[strlen(aux1) - 1] != '\\') strcat(aux1, "\\");
	strcat(aux1, codeName);
	addRecentFile(aux1);
}
void fSaveCode()
{
	_chdir(codeLocation);
	std::ofstream fileOutput(codeName);
	if (fileOutput.is_open())
	{
		for (int i = 0; i < codeLines; i++)
			fileOutput << userCode[i] << '\n';
		b_saved = true;
	}
	_chdir(afritLocation);
	fileOutput.close();
}
void buildCode()
{
	_chdir(codeLocation);
	char aux[256] = { 0 };
	strcpy(aux, projectName);
	strcat(aux, ".bat");
	std::ofstream fileOutput(aux);
	if (fileOutput.is_open())
	{
		for (int i = 0; i < codeLines; i++)
			fileOutput << userCode[i] << '\n';
	}
	_chdir(afritLocation);
	fileOutput.close();
}
void runCode()
{
	_chdir(codeLocation);
	char aux0[256];
	strcpy(aux0, "/c");
	strcat(aux0, projectName);
	STARTUPINFO info = { sizeof(info) };
	char *aux; size_t len;
	_dupenv_s(&aux, &len, "COMSPEC");
	CreateProcessA(aux, aux0, NULL, NULL, TRUE, CREATE_NEW_CONSOLE, NULL, NULL, &info, &codeProcess);
	b_running = true;
	_chdir(afritLocation);
}
void stopCode()
{
	TerminateProcess(codeProcess.hProcess, NULL);
	CloseHandle(codeProcess.hProcess);
	CloseHandle(codeProcess.hThread);
	b_running = false;
}
void evalCode(char s[], int line)
{
	char aux0[1001], delims[50] = { " -+/*=:%!.@(),[]\">\^\\" };
	if (s[0] == ' ' || s[0] == 0) //blank lines
	{
		bool ok = true;
		for (int i = 0; i < strlen(s); i++)
			if (s[i] != ' ' && s[0] != 0)
				ok = false;
		if (ok)
			errorList[line] = 0;
		else
			errorList[line] = 1;
	}
	else if (s[0] == '.' || s[0] == '@')
	{
		my_tok(s, delims, aux0);
		my_tok(s, NULL, aux0);
		if (_strcmpi(aux0, "echo"))
			errorList[line] = 1;
		else
			errorList[line] = 0;
	}
	else if (s[0] == ':' && s[1] != 0)
		errorList[line] = 0; //comm
	else
	{
		my_tok(s, delims, aux0);
		if (_strcmpi(aux0, "echo") && _strcmpi(aux0, "cd") && _strcmpi(aux0, "goto") && _strcmpi(aux0, "rem") && _strcmpi(aux0, "for") && _strcmpi(aux0, "set") && _strcmpi(aux0, "if") && _strcmpi(aux0, "choice") && _strcmpi(aux0, "cls") && _strcmpi(aux0, "mode") && _strcmpi(aux0, "color") && _strcmpi(aux0, "date") && _strcmpi(aux0, "exit") && _strcmpi(aux0, "title") && _strcmpi(aux0, "setlocal") && _strcmpi(aux0, "endlocal") && _strcmpi(aux0, "time") && _strcmpi(aux0, "pause") && _strcmpi(aux0, "timeout") && _strcmpi(aux0, "help") && _strcmpi(aux0, "assoc") && _strcmpi(aux0, "attrib") && _strcmpi(aux0, "assoc") && _strcmpi(aux0, "call") && _strcmpi(aux0, "cmd") && _strcmpi(aux0, "copy") && _strcmpi(aux0, "del") && _strcmpi(aux0, "dir") && _strcmpi(aux0, "erase") && _strcmpi(aux0, "fc") && _strcmpi(aux0, "mkdir") && _strcmpi(aux0, "move") && _strcmpi(aux0, "path") && _strcmpi(aux0, "print") && _strcmpi(aux0, "rd") && _strcmpi(aux0, "ren") && _strcmpi(aux0, "rename") && _strcmpi(aux0, "replace") && _strcmpi(aux0, "shutdown") && _strcmpi(aux0, "start") && _strcmpi(aux0, "tasklist") && _strcmpi(aux0, "taskkill") && _strcmpi(aux0, "tree") && _strcmpi(aux0, "type") && _strcmpi(aux0, "ver") && _strcmpi(aux0, "vol") && _strcmpi(aux0, "xcopy") && strcmp(aux0, "cmdwiz") && strcmp(aux0, "batbox") && strcmp(aux0, ")"))
		{
			errorList[line] = 1; //unknown syntax
		}
		else
			errorList[line] = 0; //no error
	}
}
void evalDrawLine(char s[], int line, sf::Vector2i offset, sf::RenderWindow& window)
{
	if (b_find && codeSelection.end.x != -1 && codeSelection.start.y == codeSelection.end.y)
	{
		char aux[1001];
		memset(aux, 0, 1001);
		strncpy(aux, userCode[codeSelection.start.y] + codeSelection.start.x, codeSelection.end.x - codeSelection.start.x + 1);
		aux[strlen(aux)] = 0;
		if(strstr(s, aux))
		{
			ptrdiff_t i = strstr(s, aux) - s;
			sf::RectangleShape shape0(sf::Vector2f(strlen(aux) * 11, 20.f));
			shape0.setPosition(sf::Vector2f((i + offset.x) * 11, (line + offset.y) * 20 + 2.f));
			shape0.setFillColor(sf::Color(255, 150, 0, 150));
			window.draw(shape0);
		}
	}

	int k = 0, cnt;
	char aux0[1001], aux1[1001], delims[50] = { " -+/*=:%!.@(),[]\">\^\\" }, varDelims[3] = { "%" };
	bool eol = false, isVar, quote, first = true, inFile;

	my_tok(s, delims, aux0);
	while (aux0[0] && !eol)
	{
		eol = false;

		if (aux0[0] == '/') // modes
		{
			w_Print(aux0, k + offset.x - scroll.x, line + offset.y, colors[0], colors[3], window), k++;
			my_tok(s, NULL, aux0);
			if (!_strcmpi(aux0, "A") || !_strcmpi(aux0, "P") || !_strcmpi(aux0, "R") || !_strcmpi(aux0, "D") || !_strcmpi(aux0, "L") || !_strcmpi(aux0, "F") || !_strcmpi(aux0, "C") || !_strcmpi(aux0, "N") || !_strcmpi(aux0, "T") || !_strcmpi(aux0, "M") || !_strcmpi(aux0, "B"))
				w_Print(aux0, k + offset.x - scroll.x, line + offset.y, colors[0], colors[4], window), k++, my_tok(s, NULL, aux0);
			else if (!_strcmpi(aux0, "CS"))
				w_Print(aux0, k + offset.x - scroll.x, line + offset.y, colors[0], colors[4], window), k += 2, my_tok(s, NULL, aux0);
			else if (!_strcmpi(aux0, "nobreak"))
				w_Print(aux0, k + offset.x - scroll.x, line + offset.y, colors[0], colors[4], window), k += 7, my_tok(s, NULL, aux0);
		}

		if (!_strcmpi(aux0, "for") || !_strcmpi(aux0, "set") || !_strcmpi(aux0, "if") || !_strcmpi(aux0, "not") || !_strcmpi(aux0, "in") || !_strcmpi(aux0, "do") || !_strcmpi(aux0, "choice"))
			w_Print(aux0, k + offset.x - scroll.x, line + offset.y, colors[0], colors[4], window), k += strlen(aux0); //basic commands

		else if (!_strcmpi(aux0, "cls") || !_strcmpi(aux0, "mode") || !_strcmpi(aux0, "con") || !_strcmpi(aux0, "color") || !_strcmpi(aux0, "date") || !_strcmpi(aux0, "exit") || !_strcmpi(aux0, "title") || !_strcmpi(aux0, "setlocal") || !_strcmpi(aux0, "endlocal") || !_strcmpi(aux0, "time") || !_strcmpi(aux0, "pause") || !_strcmpi(aux0, "timeout") || !_strcmpi(aux0, "help") || !_strcmpi(aux0, "?"))
			w_Print(aux0, k + offset.x - scroll.x, line + offset.y, colors[0], colors[5], window), k += strlen(aux0); //console commands

		else if (!_strcmpi(aux0, "cd") || !_strcmpi(aux0, "assoc") || !_strcmpi(aux0, "attrib") || !_strcmpi(aux0, "assoc") || !_strcmpi(aux0, "call") || !_strcmpi(aux0, "cmd") || !_strcmpi(aux0, "copy") || !_strcmpi(aux0, "del") || !_strcmpi(aux0, "dir") || !_strcmpi(aux0, "assoc") || !_strcmpi(aux0, "erase") || !_strcmpi(aux0, "fc") || !_strcmpi(aux0, "mkdir") || !_strcmpi(aux0, "move") || !_strcmpi(aux0, "assoc") || !_strcmpi(aux0, "path") || !_strcmpi(aux0, "print") || !_strcmpi(aux0, "rd") || !_strcmpi(aux0, "ren") || !_strcmpi(aux0, "rename") || !_strcmpi(aux0, "replace") || !_strcmpi(aux0, "shutdown") || !_strcmpi(aux0, "start") || !_strcmpi(aux0, "tasklist") || !_strcmpi(aux0, "taskkill") || !_strcmpi(aux0, "tree") || !_strcmpi(aux0, "type") || !_strcmpi(aux0, "ver") || !_strcmpi(aux0, "vol") || !_strcmpi(aux0, "xcopy"))
			w_Print(aux0, k + offset.x - scroll.x, line + offset.y, colors[0], colors[6], window), k += strlen(aux0); //system commands

		else if (!strcmp(aux0, "cmdwiz") || !strcmp(aux0, "batbox"))
			w_Print(aux0, k + offset.x - scroll.x, line + offset.y, colors[0], colors[10] , window), k += strlen(aux0); //external functions

		else if (!_strcmpi(aux0, "echo"))
		{
			w_Print(aux0, k + offset.x - scroll.x, line + offset.y, colors[0], colors[4], window), k += 4;
			quote = false; inFile = false;
			my_tok(s, NULL, aux0);
			while (aux0[0])
			{
				if (aux0[0] == '"' && !quote) quote = true;
				else if (aux0[0] == '"' && quote) quote = false;
				if (aux0[0] == '>' && !quote) w_Print(aux0, k + offset.x - scroll.x, line + offset.y, colors[0], colors[3], window), k++, inFile = true;
				else if (aux0[0] == '%')
				{
					cnt = 0;
					w_Print(aux0, k + offset.x - scroll.x, line + offset.y, colors[0], colors[7], window), k++;
					my_tok(s, NULL, aux0);
					while (aux0[0] && aux0[0] != '%')
					{
						w_Print(aux0, k + offset.x - scroll.x, line + offset.y, colors[0], colors[7], window), k += strlen(aux0);
						my_tok(s, NULL, aux0);
						cnt++;
					}
					w_Print(aux0, k + offset.x - scroll.x, line + offset.y, colors[0], colors[7], window), k++;
					if (cnt == 0) // for variables
					{
						my_tok(s, NULL, aux0);
						w_Print(aux0, k + offset.x - scroll.x, line + offset.y, colors[0], colors[7], window), k += strlen(aux0);
					}
				}
				else if (aux0[0] == '!')
				{
					w_Print(aux0, k + offset.x - scroll.x, line + offset.y, colors[0], colors[8], window), k++;
					my_tok(s, NULL, aux0);
					while (aux0[0] && aux0[0] != '!')
					{
						if (aux0[0] == '%')
						{
							cnt = 0;
							w_Print(aux0, k + offset.x - scroll.x, line + offset.y, colors[0], colors[7], window), k++;
							my_tok(s, NULL, aux0);
							while (aux0[0] && aux0[0] != '%')
							{
								w_Print(aux0, k + offset.x - scroll.x, line + offset.y, colors[0], colors[7], window), k += strlen(aux0);
								my_tok(s, NULL, aux0);
								cnt++;
							}
							w_Print(aux0, k + offset.x - scroll.x, line + offset.y, colors[0], colors[7], window), k++;
							if (cnt == 0) //for variables
							{
								my_tok(s, NULL, aux0);
								w_Print(aux0, k + offset.x - scroll.x, line + offset.y, colors[0], colors[7], window), k += strlen(aux0);
							}
							my_tok(s, NULL, aux0);
						}
						else
						{
							w_Print(aux0, k + offset.x - scroll.x, line + offset.y, colors[0], colors[8], window), k += strlen(aux0);
							my_tok(s, NULL, aux0);
						}
					}
					w_Print(aux0, k + offset.x - scroll.x, line + offset.y, colors[0], colors[8], window), k++;
				}
				else if (inFile && aux0[0] == '.') w_Print(aux0, k + offset.x - scroll.x, line + offset.y, colors[0], colors[3], window), k++;
				else if (aux0[0] == '\^')
				{
					w_Print(aux0, k + offset.x - scroll.x, line + offset.y, colors[0], colors[3], window), k++;
					my_tok(s, NULL, aux0);
					w_Print(aux0, k + offset.x - scroll.x, line + offset.y, colors[0], colors[2], window), k += strlen(aux0);//error
				}
				else w_Print(aux0, k + offset.x - scroll.x, line + offset.y, colors[0], colors[2], window), k += strlen(aux0);
				my_tok(s, NULL, aux0);
			}
		}

		else if (!_strcmpi(aux0, "rem"))
		{
			w_Print(aux0, k + offset.x - scroll.x, line + offset.y, colors[0], colors[11], window), k += 3;
			w_Print(s + k, k + offset.x - scroll.x, line + offset.y, colors[0], colors[11], window), eol = true;
		}

		else if (strchr("-+*=>.@(),\"\^\\", aux0[0])) // operators
			w_Print(aux0, k + offset.x - scroll.x, line + offset.y, colors[0], colors[3], window), k++;

		else if (!_strcmpi(aux0, "equ") || !_strcmpi(aux0, "neq") || !_strcmpi(aux0, "lss") || !_strcmpi(aux0, "leq") || !_strcmpi(aux0, "gtr") || !_strcmpi(aux0, "geq"))
			w_Print(aux0, k + offset.x - scroll.x, line + offset.y, colors[0], colors[3], window), k += 3; // str operators

		else if (!_strcmpi(aux0, "goto"))
		{
			w_Print(aux0, k + offset.x - scroll.x, line + offset.y, colors[0], colors[4], window), k += strlen(aux0);
			w_Print(s + k, k + offset.x - scroll.x, line + offset.y, colors[0], colors[9], window), eol = true;
		}

		else if (aux0[0] == ':')
		{
			aux1[0] = ':', aux1[1] = 0;
			my_tok(s, NULL, aux0);
			if (aux0[0] != ':')
			{
				w_Print(aux1, k + offset.x - scroll.x, line + offset.y, colors[0], colors[4], window), k++;
				w_Print(s + k, k + offset.x - scroll.x, line + offset.y, colors[0], colors[9], window), eol = true;
			}
			else
			{
				aux1[1] = ':', aux1[2] = 0;
				w_Print(aux1, k + offset.x - scroll.x, line + offset.y, colors[0], colors[12], window), k += 2;
				w_Print(s + k, k + offset.x - scroll.x, line + offset.y, colors[0], colors[12], window), eol = true;
			}
		}

		else if (aux0[0] == '%') //variables
		{
			cnt = 0;
			w_Print(aux0, k + offset.x - scroll.x, line + offset.y, colors[0], colors[7], window), k++;
			my_tok(s, NULL, aux0);
			while (aux0[0] && aux0[0] != '%')
			{
				w_Print(aux0, k + offset.x - scroll.x, line + offset.y, colors[0], colors[7], window), k += strlen(aux0);
				my_tok(s, NULL, aux0);
				cnt++;
			}
			w_Print(aux0, k + offset.x - scroll.x, line + offset.y, colors[0], colors[7], window), k++;
			if (cnt == 0) // for variables
			{
				my_tok(s, NULL, aux0);
				w_Print(aux0, k + offset.x - scroll.x, line + offset.y, colors[0], colors[7], window), k += strlen(aux0);
			}
		}

		else if (aux0[0] == '!')
		{
			w_Print(aux0, k + offset.x - scroll.x, line + offset.y, colors[0], colors[8], window), k++;
			my_tok(s, NULL, aux0);
			while (aux0[0] && aux0[0] != '!')
			{
				if (aux0[0] == '%')
				{
					cnt = 0;
					w_Print(aux0, k + offset.x - scroll.x, line + offset.y, colors[0], colors[7], window), k++;
					my_tok(s, NULL, aux0);
					while (aux0[0] && aux0[0] != '%')
					{
						w_Print(aux0, k + offset.x - scroll.x, line + offset.y, colors[0], colors[7], window), k += strlen(aux0);
						my_tok(s, NULL, aux0);
						cnt++;
					}
					w_Print(aux0, k + offset.x - scroll.x, line + offset.y, colors[0], colors[7], window), k++;
					if (cnt == 0) //for variables
					{
						my_tok(s, NULL, aux0);
						w_Print(aux0, k + offset.x - scroll.x, line + offset.y, colors[0], colors[7], window), k += strlen(aux0);
					}
					my_tok(s, NULL, aux0);
				}
				else
				{
					w_Print(aux0, k + offset.x - scroll.x, line + offset.y, colors[0], colors[8], window), k += strlen(aux0);
					my_tok(s, NULL, aux0);
				}
			}
			w_Print(aux0, k + offset.x - scroll.x, line + offset.y, colors[0], colors[8], window), k++;
		} // extended variables

		else
			w_Print(aux0, k + offset.x - scroll.x, line + offset.y, colors[0], colors[2], window), k += strlen(aux0);
		my_tok(s, NULL, aux0);
		first = false;
	}
}
void onExit(sf::RenderWindow& window)
{
	if (b_saved || !b_code) //recent projects
	{
		std::ofstream exitOut("data0.dat");
		int i;
		for (i = 0; i < recentCnt && recentFiles[i][0]; i++)
			exitOut << recentFiles[i] << '\n';
		exitOut.close();
		window.close();
	}
	else if (!b_saveBefore) b_saveBefore = true, saveBefInh = 1;
	else if (saveBefOut != -1)
	{
		b_saveBefore = false;
		if (saveBefOut == 1) fSaveCode();
		if (saveBefOut != 0)
		{
			std::ofstream exitOut("data0.dat");
			int i;
			for (i = 0; i < recentCnt && recentFiles[i][0]; i++)
				exitOut << recentFiles[i] << '\n';
			exitOut.close();
			window.close();
		}
		saveBefOut = -1;
		saveBefInh = 0;
	}

	std::ofstream exitPref("pref.txt");
	int i, r, g, b;
	for (i = 0; i <= 12; i++)
	{
		r = colors[i].r;
		g = colors[i].g;
		b = colors[i].b;
		exitPref << r << ' ' << g << ' ' << b << '\n';
	}
	exitPref << b_verticalCursor << ' ' << b_blinkerCursor << ' ' << b_rtUpdate << ' ' << b_blueFilter << '\n';
	exitPref.close();
}
void returnMainMenu()
{
	if (b_saved) b_code = false, b_file = false, b_title = true, clearSelect();
	else if (!b_saveBefore) b_saveBefore = true, saveBefInh = 2;
	else if (saveBefOut != -1)
	{
		b_saveBefore = false;
		if (saveBefOut == 1) fSaveCode();
		if (saveBefOut != 0) b_code = false, b_file = false, b_title = true, clearSelect();
		saveBefOut = -1;
		saveBefInh = 0;
	}
}
void selectAll()
{
	codeSelection.start.x = 0;
	codeSelection.start.y = 0;
	codeSelection.end.x = strlen(userCode[codeLines - 1]) - 1;
	codeSelection.end.y = codeLines - 1;
}
void copyToClipboard()
{
	if (codeSelection.end.x != -1)
	{
		char aux[100001], aux0[1001]={0};
		int i,j;
		if(codeSelection.end.y == codeSelection.start.y) //one line
			strncpy(aux, userCode[codeSelection.start.y] + codeSelection.start.x, codeSelection.end.x - codeSelection.start.x + 1);
		else //multiline
		{
			strcpy(aux, userCode[codeSelection.start.y] + codeSelection.start.x);
			for (i = codeSelection.start.y + 1; i < codeSelection.end.y; i++)
				strcat(aux, "\n"), strcat_s(aux, 100000, userCode[i]);
			strcat(aux, "\n");
			strncpy(aux0, userCode[codeSelection.end.y], codeSelection.end.x + 1);
			strcat(aux, aux0);
		}
			
		const size_t len = strlen(aux) + 1;
		HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, len);
		memcpy(GlobalLock(hMem), aux, len);
		GlobalUnlock(hMem);
		OpenClipboard(0);
		EmptyClipboard();
		SetClipboardData(CF_TEXT, hMem);
		CloseClipboard();
	}
}
void cutCode()
{
	if (codeSelection.end.x != -1)
	{
		char aux[1001];
		int a = codeSelection.end.y - codeSelection.start.y;
		if (a == 0) // same line
		{
			strcpy(aux, userCode[codeSelection.start.y] + codeSelection.end.x + 1);
			strcpy(userCode[codeSelection.start.y] + codeSelection.start.x, aux);
			if (cursorTxtPos.y == codeSelection.start.y && cursorTxtPos.x > codeSelection.start.x)
				cursorTxtPos.x = strlen(userCode[codeSelection.start.y]);
		}
		else //multi line
		{
			strcpy(userCode[codeSelection.start.y] + codeSelection.start.x, userCode[codeSelection.end.y] + codeSelection.end.x + 1);
			for (int i = codeSelection.start.y + 1; i < codeLines - a; i++)
				strcpy(userCode[i], userCode[i + a]);
			codeLines -= a;
			if (cursorTxtPos.y >= codeSelection.start.y && cursorTxtPos.y <= codeSelection.end.y)
				cursorTxtPos = codeSelection.start;
			else if (cursorTxtPos.y > codeLines - 1) cursorTxtPos.y -= a, cursorTxtPos.x = strlen(userCode[cursorTxtPos.y]);
		}
		b_saved = false;
		clearSelect();
	}
}
void pasteCode()
{
	OpenClipboard(0);
	HANDLE hData = GetClipboardData(CF_TEXT);
	char *p = static_cast<char*>(GlobalLock(hData));
	GlobalUnlock(hData);
	CloseClipboard();
	char aux[100001], aux0[1001];

	if (codeSelection.end.x != -1)
		cursorTxtPos = codeSelection.start;


	strcpy_s(aux, 100000, p);
	p = strtok(aux, "\n");
	strcpy(aux0, userCode[cursorTxtPos.y] + cursorTxtPos.x);
	memset(userCode[cursorTxtPos.y] + cursorTxtPos.x, 0, 1000);
	strcat_s(userCode[cursorTxtPos.y] + cursorTxtPos.x, 1000, p);
	int lines = 0;
	p = strtok(NULL, "\n");
	while (p)
	{
		lines++;
		codeLines++;
		for (int i = codeLines; i > cursorTxtPos.y + lines + 1; i--)
			strcpy(userCode[i], userCode[i - 1]);
		strcpy(userCode[cursorTxtPos.y + lines], p);
		p = strtok(NULL, "\n");
	}
	strcat_s(userCode[cursorTxtPos.y + lines] + strlen(userCode[cursorTxtPos.y + lines]), 1000, aux0);
}
void addUndo(int l, char b[], char a[], bool f)
{
	if (undoCnt == 99)
	{
		for (int i = 0; i < 98; i++)
			undoList[i] = undoList[i + 1];
	}
	undoList[undoCnt].line = l;
	strcpy(undoList[undoCnt].before, b);
	strcpy(undoList[undoCnt].after, a);
	undoList[undoCnt].forward = f;
	undoPos = undoCnt;
	if (undoCnt < 99) undoCnt++;
}
void undo()
{
	if (undoPos >= 0)
	{
		do
		{
			/*
			if (undoList[undoPos].addL == 1) //remove a line
			{
				for (int i = undoList[undoPos].line; i < codeLines; i++)
					strcpy(userCode[i], userCode[i + 1]);
				codeLines--;
			}
			else if (undoList[undoPos].addL == -1) //add a line
			{ 
				codeLines++;
				for (int i = codeLines; i > undoList[undoPos].line + 1; i--)
					strcpy(userCode[i], userCode[i - 1]);
			}
			*/
			strcpy(userCode[undoList[undoPos].line], undoList[undoPos].before);
			undoPos--;
		} while (undoList[undoPos].forward && undoPos >= 0);

		if (cursorTxtPos.x > strlen(userCode[cursorTxtPos.y])) cursorTxtPos.x = strlen(userCode[cursorTxtPos.y]);
	}
}
void redo()
{
	if (undoPos < undoCnt - 1)
	{
		do
		{
			undoPos++;
			/*
			if (undoList[undoPos].addL == 1) //reverse remove a line
			{
				codeLines++;
				for (int i = codeLines; i > undoList[undoPos].line + 1; i--)
					strcpy(userCode[i], userCode[i - 1]);
			}
			else if (undoList[undoPos].addL == -1) //reverse add a line
			{
				for (int i = undoList[undoPos].line; i < codeLines; i++)
					strcpy(userCode[i], userCode[i + 1]);
				codeLines--;
			}
			*/
			strcpy(userCode[undoList[undoPos].line], undoList[undoPos].after);
		} while (undoList[undoPos].forward && undoPos < undoCnt - 1);

		if (cursorTxtPos.x > strlen(userCode[cursorTxtPos.y])) cursorTxtPos.x = strlen(userCode[cursorTxtPos.y]);
	}
}

void drawBlueFilter(sf::RenderWindow &window)
{
	sf::RectangleShape shape0(deskSize);
	shape0.setFillColor(sf::Color(150, 50, 0, 50));
	window.draw(shape0);
}
void drawCursor(sf::Vector2i cursor, sf::RenderWindow &window)
{
	static int c;
	if (c == 20) c = 0;
	else c += 1;
	if (c < 10 || !b_blinkerCursor)
	{
		sf::RectangleShape shape0;
		if (b_verticalCursor) shape0.setSize(sf::Vector2f(2.f, 20.f)), shape0.setPosition(sf::Vector2f(cursor.x * 11, cursor.y * 20));
		else shape0.setSize(sf::Vector2f(11.f, 5.f)), shape0.setPosition(sf::Vector2f(cursor.x * 11, cursor.y * 20 + 15));
		shape0.setFillColor(colors[1]);
		window.draw(shape0);
	}
}
void drawError0(sf::RenderWindow &window)
{
	sf::RectangleShape shape0(sf::Vector2f(385.f, 80.f));
	shape0.setPosition(sf::Vector2f(330.f, 100.f));
	shape0.setFillColor(sf::Color(240, 240, 240, 255));
	window.draw(shape0);

	shape0.setSize(sf::Vector2f(11.f, 60.f));    //v shader
	shape0.setPosition(sf::Vector2f(715.f, 120.f));
	shape0.setFillColor(sf::Color(0, 0, 0, 150));
	window.draw(shape0);

	shape0.setSize(sf::Vector2f(385.f, 20.f));    //h shader
	shape0.setPosition(sf::Vector2f(341.f, 180.f));
	window.draw(shape0);

	sfText.setString(L"╔═════════════════════════════════╗\n║   Error: File cannot be opened! ║\n║                                 ║\n╚═════════════════════════════════╝");
	sfText.setPosition(sf::Vector2f(330.f, 100.f));
	sfText.setFillColor(sf::Color::Black);
	window.draw(sfText);

	sf::Sprite sp;
	sp.setTexture(icon_tx);
	sp.setTextureRect(sf::IntRect(23, 21, 22, 20));
	sp.setPosition(sf::Vector2f(345.f, 123.f));
	window.draw(sp);

	char aux[51];
	sf::Vector2i mPos(sf::Mouse::getPosition(window));
	mPos.x /= 11;
	mPos.y /= 20;
	strcpy(aux, "             Close             ");
	if (mPos.y == 7 && mPos.x >= 32 && mPos.x <= 62) w_Print(aux, 32, 7, sf::Color(0, 102, 0, 255), sf::Color(255, 255, 0, 255), window);
	else w_Print(aux, 32, 7, colors[0], sf::Color::Black, window);
}
void drawWarning0(sf::RenderWindow &window)
{
	sf::RectangleShape shape0(sf::Vector2f(407.f, 120.f));
	shape0.setPosition(sf::Vector2f(330.f, 100.f));
	shape0.setFillColor(sf::Color(240, 240, 240, 255));
	window.draw(shape0);

	shape0.setSize(sf::Vector2f(11.f, 100.f));    //v shader
	shape0.setPosition(sf::Vector2f(737.f, 120.f));
	shape0.setFillColor(sf::Color(0, 0, 0, 150));
	window.draw(shape0);

	shape0.setSize(sf::Vector2f(407.f, 20.f));    //h shader
	shape0.setPosition(sf::Vector2f(341.f, 220.f));
	window.draw(shape0);

	sfText.setString(L"╔═══════════════════════════════════╗\n║   Your code returned some errors! ║\n║   Build anyway?                   ║\n║                                   ║\n║                                   ║\n╚═══════════════════════════════════╝");
	sfText.setPosition(sf::Vector2f(330.f, 100.f));
	sfText.setFillColor(sf::Color::Black);
	window.draw(sfText);

	char aux[51];
	sf::Vector2i mPos(sf::Mouse::getPosition(window));
	mPos.x /= 11;
	mPos.y /= 20;

	strcpy(aux, " Yes ");
	if (mPos.y == 9 && mPos.x >= 39 && mPos.x <= 43) w_Print(aux, 39, 9, sf::Color(255, 0, 0, 255), sf::Color(255, 255, 0, 255), window);
	else w_Print(aux, 39, 9, colors[0], sf::Color::Black, window);
	strcpy(aux, " No ");
	if (mPos.y == 9 && mPos.x >= 54 && mPos.x <= 57) w_Print(aux, 54, 9, sf::Color(0, 102, 0, 255), sf::Color(255, 255, 0, 255), window);
	else w_Print(aux, 54, 9, colors[0], sf::Color::Black, window);

	sf::Sprite sp;
	sp.setTexture(icon_tx);
	sp.setTextureRect(sf::IntRect(0, 21, 22, 20));
	sp.setPosition(sf::Vector2f(345.f, 122.f));
	window.draw(sp);
}
void drawSaveBefore(sf::RenderWindow& window)
{
	sf::RectangleShape shape0(sf::Vector2f(363.f, 100.f));
	shape0.setPosition(sf::Vector2f(330.f, 100.f));
	shape0.setFillColor(sf::Color(240, 240, 240, 255));
	window.draw(shape0);

	shape0.setSize(sf::Vector2f(11.f, 80.f));    //v shader
	shape0.setPosition(sf::Vector2f(693.f, 120.f));
	shape0.setFillColor(sf::Color(0, 0, 0, 150));
	window.draw(shape0);

	shape0.setSize(sf::Vector2f(363.f, 20.f));    //h shader
	shape0.setPosition(sf::Vector2f(341.f, 200.f));
	window.draw(shape0);

	sfText.setString(L"╔═══════════════════════════════╗\n║   Save file before you leave? ║\n║                               ║\n║                               ║\n╚═══════════════════════════════╝");
	sfText.setPosition(sf::Vector2f(330.f, 100.f));
	sfText.setFillColor(sf::Color::Black);
	window.draw(sfText);

	char aux[51];
	sf::Vector2i mPos(sf::Mouse::getPosition(window));
	mPos.x /= 11;
	mPos.y /= 20;

	strcpy(aux, " Yes ");
	if (mPos.y == 8 && mPos.x >= 36 && mPos.x <= 40) w_Print(aux, 36, 8, sf::Color(0, 102, 0, 255), sf::Color(255, 255, 0, 255), window);
	else w_Print(aux, 36, 8, colors[0], sf::Color::Black, window);
	strcpy(aux, " No ");
	if (mPos.y == 8 && mPos.x >= 42 && mPos.x <= 45) w_Print(aux, 42, 8, sf::Color(255, 0, 0, 255), sf::Color(255, 255, 0, 255), window);
	else w_Print(aux, 42, 8, colors[0], sf::Color::Black, window);
	strcpy(aux, " Cancel ");
	if (mPos.y == 8 && mPos.x >= 47 && mPos.x <= 54) w_Print(aux, 47, 8, sf::Color(0, 102, 0, 255), sf::Color(255, 255, 0, 255), window);
	else w_Print(aux, 47, 8, colors[0], sf::Color::Black, window);

	sf::Sprite sp;
	sp.setTexture(icon_tx);
	sp.setTextureRect(sf::IntRect(0, 21, 22, 20));
	sp.setPosition(sf::Vector2f(345.f, 122.f));
	window.draw(sp);
}
void drawEditorHood(sf::RenderWindow& window)
{
	//--------Menu Bar------------
	sf::RectangleShape shape0(sf::Vector2f(deskSize.x, 20.f));
	shape0.setFillColor(sf::Color(220, 220, 220, 255));
	window.draw(shape0);
	char aux[41];
	sf::Vector2i mPos(sf::Mouse::getPosition(window));
	mPos.x /= 11;
	mPos.y /= 20;

	strcpy(aux, "|        |            |        |");
	w_Print(aux, 8, 0, colors[0], sf::Color::Black, window);

	strcpy(aux, "  File  ");
	if ((mPos.y == 0 && mPos.x >= 0 && mPos.x <= 7) || b_file) w_Print(aux, 0, 0, sf::Color(0, 102, 0, 255), sf::Color(255, 255, 0, 255), window);
	else w_Print(aux, 0, 0, colors[0], sf::Color::Black, window);

	strcpy(aux, "  Edit  ");
	if ((mPos.y == 0 && mPos.x >= 9 && mPos.x <= 16) || b_edit) w_Print(aux, 9, 0, sf::Color(0, 102, 0, 255), sf::Color(255, 255, 0, 255), window);
	else w_Print(aux, 9, 0, colors[0], sf::Color::Black, window);

	strcpy(aux, "  Settings  ");
	if ((mPos.y == 0 && mPos.x >= 18 && mPos.x <= 29) || b_settingsEdt || b_settingsEnv || b_settingsOth) w_Print(aux, 18, 0, sf::Color(0, 102, 0, 255), sf::Color(255, 255, 0, 255), window);
	else w_Print(aux, 18, 0, colors[0], sf::Color::Black, window);

	strcpy(aux, "  Help  ");
	if (mPos.y == 0 && mPos.x >= 31 && mPos.x <= 38) w_Print(aux, 31, 0, sf::Color(0, 102, 0, 255), sf::Color(255, 255, 0, 255), window);
	else w_Print(aux, 31, 0, colors[0], sf::Color::Black, window);
}
void drawCodeScreen(sf::RenderWindow& window)
{
	sf::RectangleShape shape0(sf::Vector2f(deskSize.x, deskSize.y));
	//text background
	shape0.setFillColor(sf::Color::Black);
	shape0.setPosition(sf::Vector2f(33.0f, 20.0f));
	window.draw(shape0);
	int i; //draw the code
	if (b_evalCMD)
		for (i = scroll.y; i < codeLines; i++)
			evalDrawLine(userCode[i], i - scroll.y, cursorOffset, window); //must be done before the other for hScroll
	else
		for (i = scroll.y; i < codeLines; i++)
			w_Print(userCode[i], cursorOffset.x - scroll.x, i + cursorOffset.y, colors[0], sf::Color::White, window);

	if (codeSelection.start.x != -1)
	{
		if (codeSelection.end.x != -1)
		{
			if (codeSelection.start.y == codeSelection.end.y)
			{
				shape0.setSize(sf::Vector2f((codeSelection.end.x - codeSelection.start.x + 1) * 11.f, 20.f));
				shape0.setPosition(sf::Vector2f((codeSelection.start.x + cursorOffset.x - scroll.x) * 11.f, (codeSelection.start.y + cursorOffset.y - scroll.y) * 20.f + 2.f));
				shape0.setFillColor(sf::Color(100, 100, 255, 150));
				window.draw(shape0);
			}
			else
			{
				shape0.setFillColor(sf::Color(100, 100, 255, 150));
				shape0.setSize(sf::Vector2f((strlen(userCode[codeSelection.start.y] + codeSelection.start.x)) * 11.f, 20.f));
				shape0.setPosition(sf::Vector2f((codeSelection.start.x + cursorOffset.x - scroll.x) * 11.f, (codeSelection.start.y + cursorOffset.y - scroll.y) * 20.f + 2.f));
				window.draw(shape0);
				for (i = codeSelection.start.y + 1; i < codeSelection.end.y; i++)
				{
					shape0.setSize(sf::Vector2f(strlen(userCode[i]) * 11.f, 20.f));
					shape0.setPosition(sf::Vector2f((cursorOffset.x - scroll.x) * 11.f, (i + cursorOffset.y - scroll.y) * 20.f + 2.f));
					window.draw(shape0);
				}
				shape0.setSize(sf::Vector2f((codeSelection.end.x + 1) * 11.f, 20.f));
				shape0.setPosition(sf::Vector2f((cursorOffset.x - scroll.x) * 11.f, (codeSelection.end.y + cursorOffset.y - scroll.y) * 20.f + 2.f));
				window.draw(shape0);
			}
		}
		else
		{
			shape0.setSize(sf::Vector2f(11.f, 20.f));
			shape0.setPosition(sf::Vector2f((codeSelection.start.x + cursorOffset.x - scroll.x) * 11.f, (codeSelection.start.y + cursorOffset.y - scroll.y) * 20.f + 2.f));
			shape0.setFillColor(sf::Color(100, 175, 255, 150));
			window.draw(shape0);
		}
	}

	char aux[51];
	shape0.setSize(sf::Vector2f(deskSize.x, 20.f)); //build bar
	shape0.setPosition(sf::Vector2f(0.f, 20.f));
	shape0.setFillColor(sf::Color(220, 220, 220, 255));
	window.draw(shape0);

	sf::Vector2i mPos(sf::Mouse::getPosition(window));
	mPos.x /= 11;
	mPos.y /= 20;

	strcpy(aux, "|                   |                   |");
	w_Print(aux, 12, 1, colors[0], sf::Color::Black, window);
	strcpy(aux, "  Build     ");
	if (!b_file && !b_edit && !b_settingsEdt && !b_settingsEnv && !b_settingsOth && mPos.y == 1 && mPos.x >= 0 && mPos.x <= 11) w_Print(aux, 0, 1, sf::Color(0, 0, 255, 255), sf::Color(255, 255, 0, 255), window);
	else w_Print(aux, 0, 1, colors[0], sf::Color::Black, window);
	strcpy(aux, "  Build & Run      ");
	if (!b_running && !b_file && !b_edit && !b_settingsEdt && !b_settingsEnv && !b_settingsOth && (mPos.y == 1 && mPos.x >= 13 && mPos.x <= 31)) w_Print(aux, 13, 1, sf::Color(0, 0, 255, 255), sf::Color(255, 255, 0, 255), window);
	else if (!b_running) w_Print(aux, 13, 1, colors[0], sf::Color::Black, window);
	else w_Print(aux, 13, 1, colors[0], sf::Color(100, 100, 100, 255), window);
	strcpy(aux, "  Force Stop       ");
	if (b_running && !b_file && !b_edit && !b_settingsEdt && !b_settingsEnv && !b_settingsOth && (mPos.y == 1 && mPos.x >= 33 && mPos.x <= 51)) w_Print(aux, 33, 1, sf::Color(255, 0, 0, 255), sf::Color(255, 255, 0, 255), window);
	else if (b_running) w_Print(aux, 33, 1, colors[0], sf::Color::Black, window);
	else w_Print(aux, 33, 1, colors[0], sf::Color(100, 100, 100, 255), window);
	strcpy(aux, "F8                  F9                 F10");
	w_Print(aux, 9, 1, colors[0], sf::Color(100, 100, 100, 255), window);


	//left counter bar
	shape0.setFillColor(sf::Color(175, 175, 175, 255));
	shape0.setSize(sf::Vector2f(55.f, deskSize.y));
	shape0.setPosition(sf::Vector2f(0.f, 40.f));
	window.draw(shape0);
	std::string str;
	shape0.setSize(sf::Vector2f(11.f, 20.f));
	shape0.setFillColor(sf::Color(255, 0, 0, 200));
	for (i = scroll.y; i < codeLines; i++)
	{
		if (errorList[i] != 0)
		{
			shape0.setPosition(sf::Vector2f(0.f, (i-scroll.y + 2)*20));
			window.draw(shape0);
		}

		if (i + 1 <= 9) str = "   " + std::to_string(i + 1);
		else if (i + 1 <= 99) str = "  " + std::to_string(i + 1);
		else if (i + 1 <= 999) str = " " + std::to_string(i + 1);
		else if (i + 1 <= 9999) str = std::to_string(i + 1);
		sfText.setString(str);
		sfText.setFillColor(sf::Color(75, 75, 75, 255));
		sfText.setPosition(sf::Vector2f(0.f, 20 * (i - scroll.y + 2)));
		window.draw(sfText);
	}

	shape0.setSize(sf::Vector2f(22.f, deskSize.y - 60));           //v scroll
	shape0.setPosition(sf::Vector2f(deskSize.x - 22, 40));
	shape0.setFillColor(sf::Color(175, 175, 175, 255));
	window.draw(shape0);
	sf::Sprite sp; sp.setTexture(icon_tx);
	sp.setTextureRect(sf::IntRect(0, 0, 22, 20));
	sp.setPosition(sf::Vector2f(deskSize.x - 22, 40.f));
	sp.setColor(sf::Color(0, 100, 255, 255));
	window.draw(sp);
	sp.setTextureRect(sf::IntRect(23, 0, 22, 20));
	sp.setPosition(sf::Vector2f(deskSize.x - 22, deskSize.y - 60));
	window.draw(sp);

	shape0.setSize(sf::Vector2f(deskSize.x, 20.f));           //h scroll
	shape0.setPosition(sf::Vector2f(0.f, deskSize.y - 40));
	shape0.setFillColor(sf::Color(175, 175, 175, 255));
	window.draw(shape0);
	sp.setTextureRect(sf::IntRect(46, 0, 22, 20));
	sp.setPosition(sf::Vector2f(55.f, deskSize.y - 40));
	window.draw(sp);
	sp.setTextureRect(sf::IntRect(69, 0, 22, 20));
	sp.setPosition(sf::Vector2f(deskSize.x - 44, deskSize.y - 40));
	window.draw(sp);
	sp.setColor(sf::Color(255, 255, 255, 255));


	shape0.setSize(sf::Vector2f(deskSize.x, 20.f));           //bottom bar
	shape0.setPosition(sf::Vector2f(0.f, deskSize.y - 20));
	shape0.setFillColor(sf::Color(220, 220, 220, 255));
	window.draw(shape0);
	strcpy(aux, "Project: ");
	w_Print(aux, 5, deskSize.y / 20 - 1, colors[0], sf::Color::Black, window);
	memset(aux, 0, 10);
	if (strlen(projectName) > 50)
		strncpy(aux, projectName, 50), strcat(aux, "...");
	else
		strcpy(aux, projectName);
	w_Print(aux, 14, deskSize.y / 20 - 1, colors[0], sf::Color::Blue, window);

	if (b_running) strcpy(aux, "Running"), w_Print(aux, 120, deskSize.y / 20 - 1, colors[0], sf::Color(0, 0, 255, 255), window); // Running

	if (b_saved)
	{
		strcpy(aux, "Saved"), w_Print(aux, 130, deskSize.y / 20 - 1, colors[0], sf::Color(0, 150, 0, 255), window); // Saved
		sp.setTextureRect(sf::IntRect(46, 21, 22, 20));
		sp.setPosition(sf::Vector2f(1408.f, deskSize.y - 20));
		window.draw(sp);
	}

	str = "Line: " + std::to_string(cursorTxtPos.y) + " | Column: " + std::to_string(cursorTxtPos.x); //Cursor Coord
	sfText.setString(str);
	sfText.setFillColor(sf::Color::Black);
	sfText.setPosition(sf::Vector2f(1540.f, deskSize.y - 22));
	window.draw(sfText);
	if (b_find)
	{
		sp.setTextureRect(sf::IntRect(69, 21, 22, 20));
		sp.setPosition(sf::Vector2f(1364.f, deskSize.y - 20));
		window.draw(sp);
	}

	if (mPos.x >= 0 && mPos.x <= 4 && mPos.y >= 2 && errorList[mPos.y - 2] != 0)
	{
		if (errorList[mPos.y - 2] == 1)
		{
			shape0.setSize(sf::Vector2f(341.f, 60.f)); 
			shape0.setPosition(sf::Vector2f(44.f, mPos.y * 20 - 20));
			shape0.setFillColor(sf::Color(220, 220, 220, 255));
			window.draw(shape0);

			sfText.setString(L"╔═════════════════════════════╗\n║                             ║\n╚═════════════════════════════╝");
			sfText.setPosition(sf::Vector2f(44.f, mPos.y*20 - 20));
			sfText.setFillColor(sf::Color::Black);
			window.draw(sfText);
			sfText.setString("Error #001: Unknown syntax!");
			sfText.setPosition(sf::Vector2f(66.f, mPos.y * 20));
			sfText.setFillColor(sf::Color::Red);
			window.draw(sfText);
		}
	}
}
void drawTitleScreen(sf::RenderWindow& window)
{
	sfText.setString(L"                       ░░░▒▒▓▓▓▓▓▓▓▓▓▒░          ░░▒▒▒▒▒░               ░▓▓▒░\n                      ░▓███████████████▒       ░▒▓██████▓              ▒█████░\n                      ░░░░   ░▓█████████░     ░▓████▒▓██▓              ░████▓░     ░░░░\n                            ▒███▒▒██████▒     ▒████▓░░░▒░ ░░░░   ░░░░░  ░▒▒░░░    ▓▓▓██░░░\n                          ░▒██▓░  ██████▓    ░▓████████▓▒ ▓█▓▓▓▒▒████░░░░░▒▓▓▒   ░██████▓▓░\n                         ░▓██▓░   ▓██████   ░▓███████▓▓▓  ▓████▓████▓░▓▓█████▓  ░▓█████▓▓▓\n          ░░░░░▒▒▒▒░░░░░▒██▓░     ░██████░  ░░▓█████      ▓█████▓░▒▓░░░░▓████▓  ░░████▓\n      ░░▒▓▓████████████████▓▓▓▓▒▒░░██████▓    ▓█████      ▓█████        ▓████▓   ▒████▓\n    ▒▓██████▓▓▓▓▓▓▓▓▓████▓▓██████████████▓    ▒█████      ▓█████▓▒░     ▓████▓   ▒█████▓▒░\n  ▒▓████░░░░░░░    ░▓██▒░          ▒██████░   ▒█████      ▒███████░     ▓████▓   ░▓█████▓░\n░▓█████▒         ░▓█▓▒░            ▒██████▒   ▒█████░      ░▒▓▓▓▓░      ▓█▓▓▓▓    ░▒▓▓█▒░\n▒██████▓░      ░▓█▓▒░              ░███████   ▒█▓▓▓▒░                   ░░░          ░░\n░████████▓▓▓▓███▓░                 ░████▓▓▒    ░░\n ░▒▓█████████▒░░                    ▒░░\n    ░░░░░░░\n");
	sfText.setPosition(sf::Vector2f(44.f, 40.f));
	sfText.setFillColor(sf::Color(255, 200, 0, 255));
	window.draw(sfText);

	int i;
	if (recentCnt)
	{
		char aux[55];
		sf::Vector2i mPos(sf::Mouse::getPosition(window));
		mPos.x /= 11;
		mPos.y /= 20;
		sf::RectangleShape shape0(sf::Vector2f(605.f, 20.f));
		shape0.setPosition(sf::Vector2f(55.f, 380.f));
		shape0.setFillColor(sf::Color(240, 240, 240, 255));
		window.draw(shape0);

		shape0.setSize(sf::Vector2f(11.f, recentCnt*20));    //v shader
		shape0.setPosition(sf::Vector2f(660.f, 400.f));
		shape0.setFillColor(sf::Color(0, 0, 0, 150));
		window.draw(shape0);

		shape0.setSize(sf::Vector2f(605.f, 20.f));    //h shader
		shape0.setPosition(sf::Vector2f(66.f, recentCnt * 20 + 400));
		window.draw(shape0);

		int j, k;
		for (i = 0; i < recentCnt; i++)
		{
			k = strlen(recentFiles[i]);
			shape0.setPosition(sf::Vector2f(55.f, i * 20 + 400));
			if (k > 50)
			{
				for (j = 0; j < 50; j++)
					aux[j] = recentFiles[i][j + k - 50];
				aux[0] = '.', aux[1] = '.', aux[2] = '.', aux[50] = 0;
			}
			else
				strcpy(aux, recentFiles[i]);

			if (!b_file && !b_edit && !b_settingsEdt && !b_settingsEnv && !b_settingsOth && mPos.y == i + 20 && mPos.x >= 5 && mPos.x <= 59)
			{
				shape0.setFillColor(sf::Color(0, 102, 0, 255));
				window.draw(shape0);
				w_Print(aux, 7, i + 20, colors[0], sf::Color(255, 255, 0, 255), window);
				strcpy(aux, "X");
				w_Print(aux, 59, i + 20, colors[0], sf::Color(355, 0, 0, 255), window);
			}
			else
			{
				shape0.setFillColor(sf::Color(200, 200, 200, 255));
				window.draw(shape0);
				w_Print(aux, 7, i + 20, colors[0], sf::Color::Black, window);
			}
		}
		strcpy(aux, "Recent projects:");
		w_Print(aux, 5, 19, colors[0], sf::Color::Black, window);
	}
	
	//version notice
	sf::RectangleShape shape0(sf::Vector2f(330.f, 280.f));
	shape0.setPosition(sf::Vector2f(1100.f, 60.f));
	shape0.setFillColor(sf::Color(150, 150, 150, 255));
	window.draw(shape0);
	shape0.setSize(sf::Vector2f(11.f, 260.f));    //v shader
	shape0.setPosition(sf::Vector2f(1430.f, 80.f));
	shape0.setFillColor(sf::Color(0, 0, 0, 150));
	window.draw(shape0);
	shape0.setSize(sf::Vector2f(330.f, 20.f));    //h shader
	shape0.setPosition(sf::Vector2f(1111.f, 340.f));
	window.draw(shape0);

	sfText.setString(L"╔════════════════════════════╗\n║ Afrit version 0.8 (alpha)  ║\n║                            ║\n╟────────────────────────────╢\n║ Lucrare de atestat         ║\n║ INFORMATICA, anul 2021     ║\n║                            ║\n║ Autor:                     ║\n║ FILIP ANDREI-DAN           ║\n║                            ║\n║ Profesori coordonatori:    ║\n║ - PRECUP LUMINITA-DORINA   ║\n║ - ROSU OVIDIU              ║\n╚════════════════════════════╝\n");
	sfText.setPosition(sf::Vector2f(1100.f, 60.f));
	sfText.setFillColor(sf::Color::Black);
	window.draw(sfText);

}
void drawFileMenu(sf::RenderWindow &window)
{
	sf::RectangleShape shape0(sf::Vector2f(275.f, 140.f));
	shape0.setPosition(sf::Vector2f(0.f, 20.f));
	shape0.setFillColor(sf::Color(240, 240, 240, 255));
	window.draw(shape0);

	shape0.setSize(sf::Vector2f(11.f, 120.f));    //v shader
	shape0.setPosition(sf::Vector2f(275.f, 40.f));
	shape0.setFillColor(sf::Color(0, 0, 0, 150));
	window.draw(shape0);

	shape0.setSize(sf::Vector2f(275.f, 20.f));    //h shader
	shape0.setPosition(sf::Vector2f(11.f, 160.f));
	window.draw(shape0);

	sfText.setString(L"╔═══════════════════════╗\n║                       ║\n║                       ║\n║                       ║\n║                       ║\n║                       ║\n╚═══════════════════════╝");
	sfText.setPosition(sf::Vector2f(0.f, 20.f));
	sfText.setFillColor(sf::Color::Black);
	window.draw(sfText);

	char aux[51];
	sf::Vector2i mPos(sf::Mouse::getPosition(window));
	mPos.x /= 11;
	mPos.y /= 20;

	strcpy(aux, " New                   ");
	if (!b_openFile && !b_code && ((mPos.y == 2 && mPos.x >= 1 && mPos.x <= 23) || b_newFile)) w_Print(aux, 1, 2, sf::Color(0, 102, 0, 255), sf::Color(255, 255, 0, 255), window);
	else if(!b_code) w_Print(aux, 1, 2, colors[0], sf::Color::Black, window);
	else w_Print(aux, 1, 2, colors[0], sf::Color(100, 100, 100, 255), window);
	strcpy(aux, " Open                  ");
	if (!b_newFile && !b_code &&((mPos.y == 3 && mPos.x >= 1 && mPos.x <= 23) || b_openFile)) w_Print(aux, 1, 3, sf::Color(0, 102, 0, 255), sf::Color(255, 255, 0, 255), window);
	else if(!b_code) w_Print(aux, 1, 3, colors[0], sf::Color::Black, window);
	else w_Print(aux, 1, 3, colors[0], sf::Color(100, 100, 100, 255), window);
	strcpy(aux, " Save File             ");
	if (b_code && !b_newFile && !b_openFile && (mPos.y == 4 && mPos.x >= 1 && mPos.x <= 23)) w_Print(aux, 1, 4, sf::Color(0, 102, 0, 255), sf::Color(255, 255, 0, 255), window);
	else if (b_code) w_Print(aux, 1, 4, colors[0], sf::Color::Black, window);
	else w_Print(aux, 1, 4, colors[0], sf::Color(100, 100, 100, 255), window);
	strcpy(aux, " Main Menu             ");
	if (b_code && !b_newFile && !b_openFile && (mPos.y == 6 && mPos.x >= 1 && mPos.x <= 23)) w_Print(aux, 1, 6, sf::Color(0, 102, 0, 255), sf::Color(255, 255, 0, 255), window);
	else if (b_code) w_Print(aux, 1, 6, colors[0], sf::Color::Black, window);
	else w_Print(aux, 1, 6, colors[0], sf::Color(100, 100, 100, 255), window);

	strcpy(aux, "Ctrl+S");
	w_Print(aux, 17, 4, colors[0], sf::Color(100, 100, 100, 255), window);
}
void drawEditMenu(sf::RenderWindow &window)
{
	sf::RectangleShape shape0(sf::Vector2f(297.f, 220.f));
	shape0.setPosition(sf::Vector2f(99.f, 20.f));
	shape0.setFillColor(sf::Color(240, 240, 240, 255));
	window.draw(shape0);

	shape0.setSize(sf::Vector2f(11.f, 200.f));    //v shader
	shape0.setPosition(sf::Vector2f(396.f, 40.f));
	shape0.setFillColor(sf::Color(0, 0, 0, 150));
	window.draw(shape0);

	shape0.setSize(sf::Vector2f(297.f, 20.f));    //h shader
	shape0.setPosition(sf::Vector2f(110.f, 240.f));
	window.draw(shape0);

	sfText.setString(L"╔═════════════════════════╗\n║                         ║\n║                         ║\n║                         ║\n║                         ║\n║                         ║\n║                         ║\n║                         ║\n║                         ║\n║                         ║\n╚═════════════════════════╝");
	sfText.setPosition(sf::Vector2f(99.f, 20.f));
	sfText.setFillColor(sf::Color::Black);
	window.draw(sfText);

	char aux[51];
	sf::Vector2i mPos(sf::Mouse::getPosition(window));
	mPos.x /= 11;
	mPos.y /= 20;

	strcpy(aux, " Undo                    ");
	if (b_code && (mPos.y == 2 && mPos.x >= 10 && mPos.x <= 34)) w_Print(aux, 10, 2, sf::Color(0, 102, 0, 255), sf::Color(255, 255, 0, 255), window);
	else if (b_code) w_Print(aux, 10, 2, colors[0], sf::Color::Black, window);
	else w_Print(aux, 10, 2, colors[0], sf::Color(100, 100, 100, 255), window);
	strcpy(aux, " Redo                    ");
	if (b_code && (mPos.y == 3 && mPos.x >= 10 && mPos.x <= 34)) w_Print(aux, 10, 3, sf::Color(0, 102, 0, 255), sf::Color(255, 255, 0, 255), window);
	else if (b_code) w_Print(aux, 10, 3, colors[0], sf::Color::Black, window);
	else w_Print(aux, 10, 3, colors[0], sf::Color(100, 100, 100, 255), window);
	strcpy(aux, " Copy                    ");
	if (b_code && (mPos.y == 5 && mPos.x >= 10 && mPos.x <= 34)) w_Print(aux, 10, 5, sf::Color(0, 102, 0, 255), sf::Color(255, 255, 0, 255), window);
	else if (b_code) w_Print(aux, 10, 5, colors[0], sf::Color::Black, window);
	else w_Print(aux, 10, 5, colors[0], sf::Color(100, 100, 100, 255), window);
	strcpy(aux, " Cut                     ");
	if (b_code && (mPos.y == 6 && mPos.x >= 10 && mPos.x <= 34)) w_Print(aux, 10, 6, sf::Color(0, 102, 0, 255), sf::Color(255, 255, 0, 255), window);
	else if (b_code) w_Print(aux, 10, 6, colors[0], sf::Color::Black, window);
	else w_Print(aux, 10, 6, colors[0], sf::Color(100, 100, 100, 255), window);
	strcpy(aux, " Paste                   ");
	if (b_code && (mPos.y == 7 && mPos.x >= 10 && mPos.x <= 34)) w_Print(aux, 10, 7, sf::Color(0, 102, 0, 255), sf::Color(255, 255, 0, 255), window);
	else if (b_code) w_Print(aux, 10, 7, colors[0], sf::Color::Black, window);
	else w_Print(aux, 10, 7, colors[0], sf::Color(100, 100, 100, 255), window);
	strcpy(aux, " Find                    ");
	if (b_code && (mPos.y == 9 && mPos.x >= 10 && mPos.x <= 34)) w_Print(aux, 10, 9, sf::Color(0, 102, 0, 255), sf::Color(255, 255, 0, 255), window);
	else if (b_code) w_Print(aux, 10, 9, colors[0], sf::Color::Black, window);
	else w_Print(aux, 10, 9, colors[0], sf::Color(100, 100, 100, 255), window);
	strcpy(aux, " Select All              ");
	if (b_code && (mPos.y == 10 && mPos.x >= 10 && mPos.x <= 34)) w_Print(aux, 10, 10, sf::Color(0, 102, 0, 255), sf::Color(255, 255, 0, 255), window);
	else if (b_code) w_Print(aux, 10, 10, colors[0], sf::Color::Black, window);
	else w_Print(aux, 10, 10, colors[0], sf::Color(100, 100, 100, 255), window);

	strcpy(aux, "Ctrl+Z");
	w_Print(aux, 28, 2, colors[0], sf::Color(100, 100, 100, 255), window);
	strcpy(aux, "Ctrl+Alt+Z");
	w_Print(aux, 24, 3, colors[0], sf::Color(100, 100, 100, 255), window);
	strcpy(aux, "Ctrl+C");
	w_Print(aux, 28, 5, colors[0], sf::Color(100, 100, 100, 255), window);
	strcpy(aux, "Ctrl+X");
	w_Print(aux, 28, 6, colors[0], sf::Color(100, 100, 100, 255), window);
	strcpy(aux, "Ctrl+V");
	w_Print(aux, 28, 7, colors[0], sf::Color(100, 100, 100, 255), window);
	strcpy(aux, "Ctrl+F");
	w_Print(aux, 28, 9, colors[0], sf::Color(100, 100, 100, 255), window);
	strcpy(aux, "Ctrl+A");
	w_Print(aux, 28, 10, colors[0], sf::Color(100, 100, 100, 255), window);
}
void drawSettingsMenu(sf::RenderWindow &window)
{
	sf::RectangleShape shape0(sf::Vector2f(803.f, 560.f));
	shape0.setPosition(sf::Vector2f(110.f, 20.f));
	shape0.setFillColor(sf::Color(240, 240, 240, 255));
	window.draw(shape0);

	shape0.setSize(sf::Vector2f(11.f, 540.f));    //v shader
	shape0.setPosition(sf::Vector2f(913.f, 40.f));
	shape0.setFillColor(sf::Color(0, 0, 0, 150));
	window.draw(shape0);

	shape0.setSize(sf::Vector2f(803.f, 20.f));    //h shader
	shape0.setPosition(sf::Vector2f(121.f, 580.f));
	window.draw(shape0);

	sfText.setString(L"╔═════════════╤═══════════════════════════════════════════════════════╤═╗\n                                                                      │ ║\n                                                                      └─╢\n                                                                        ║\n                                                                        ║\n                                                                        ║\n                                                                        ║\n                                                                        ║\n║             │                                                         ║\n║             │                                                         ║\n║             │                                                         ║\n║             │                                                         ║\n║             │                                                         ║\n║             │                                                         ║\n║             │                                                         ║\n║             │                                                         ║\n║             │                                                         ║\n║             │                                                         ║\n║             │                                                         ║\n║             │                                                         ║\n║             │                                                         ║\n║             │                                                         ║\n║             │                                                         ║\n║             │                                                         ║\n║             │                                                         ║\n║             │                                                         ║\n║             │                                                         ║\n╚═════════════╧═════════════════════════════════════════════════════════╝\n");
	sfText.setPosition(sf::Vector2f(110.f, 15.f));
	sfText.setFillColor(sf::Color::Black);
	window.draw(sfText);

	char aux[51];
	sf::Vector2i mPos(sf::Mouse::getPosition(window));
	mPos.x /= 11;
	mPos.y /= 20;

	strcpy(aux, " Editor      ");
	if ((mPos.y == 3 && mPos.x >= 11 && mPos.x <= 24) || b_settingsEdt) w_Print(aux, 11, 3, sf::Color(0, 102, 0, 255), sf::Color(255, 255, 0, 255), window);
	else w_Print(aux, 11, 3, colors[0], sf::Color::Black, window);
	strcpy(aux, " Environment ");
	if ((mPos.y == 5 && mPos.x >= 11 && mPos.x <= 24) || b_settingsEnv) w_Print(aux, 11, 5, sf::Color(0, 102, 0, 255), sf::Color(255, 255, 0, 255), window);
	else w_Print(aux, 11, 5, colors[0], sf::Color::Black, window);
	strcpy(aux, " Other       ");
	if ((mPos.y == 7 && mPos.x >= 11 && mPos.x <= 24) || b_settingsOth) w_Print(aux, 11, 7, sf::Color(0, 102, 0, 255), sf::Color(255, 255, 0, 255), window);
	else w_Print(aux, 11, 7, colors[0], sf::Color::Black, window);
	strcpy(aux, "X");
	if (mPos.y == 2 && mPos.x == 81) w_Print(aux, 81, 2, sf::Color(255, 0, 0, 255), sf::Color::Black, window);
	else w_Print(aux, 81, 2, colors[0], sf::Color::Black, window);

	if (b_settingsEdt)
	{
		sfText.setString(L"╟─────────────┘\n║\n╟─────────────┐\n║             │\n║             │\n║             │\n║             │\n");
		sfText.setPosition(sf::Vector2f(110.f, 35.f));
		sfText.setFillColor(sf::Color::Black);
		window.draw(sfText);

		sfText.setString(L"                      ── Colors ──\n\n                    │ R │ G │ B │\nBackground:         │   │   │   │  ->\nCursor:             │   │   │   │  ->\nStrings:            │   │   │   │  ->\nOperators:          │   │   │   │  ->\nBasic Commands:     │   │   │   │  ->\nConsole Commands:   │   │   │   │  ->\nSystem Commands:    │   │   │   │  ->\nVariables:          │   │   │   │  ->\nExtended Variables: │   │   │   │  ->\nLabels:             │   │   │   │  ->\nExternal Functions: │   │   │   │  ->\nComments 1:         │   │   │   │  ->\nComments 2:         │   │   │   │  ->\n\n\n\n                      ── Cursor ──\n\nStyle: [ ] Vertical    [ ] Horizontal\nBlink: [ ] Yes         [ ] No\n");
		sfText.setPosition(sf::Vector2f(286.f, 55.f));
		sfText.setFillColor(sf::Color::Black);
		window.draw(sfText);

		int i;
		std::string str, r, g, b;
		sfText.setString(L"Abc123█▓▒░\n");
		for (i = 0; i <= 12; i++)
		{
			sfText.setPosition(sf::Vector2f(704.f, i * 20 + 115));
			sfText.setFillColor(colors[i]);
			window.draw(sfText);
		}
		sfText.setFillColor(sf::Color::Black);
		for (i = 0; i <= 12; i++)
		{
			r = std::to_string(colors[i].r);
			if (colors[i].r < 100) r = " " + r;
			if (colors[i].r < 10) r = " " + r;
			g = std::to_string(colors[i].g);
			if (colors[i].g < 100) g = " " + g;
			if (colors[i].g < 10) g = " " + g;
			b = std::to_string(colors[i].b);
			if (colors[i].b < 100) b = " " + b;
			if (colors[i].b < 10) b = " " + b;
			str = r + " " + g + " " + b;
			sfText.setString(str);
			sfText.setPosition(sf::Vector2f(517.f, i * 20 + 115));
			window.draw(sfText);
		}

		strcpy(aux, " Reset to default ");
		if (mPos.y == 20 && mPos.x >= 44 && mPos.x <= 61) w_Print(aux, 44, 20, sf::Color(0, 102, 0, 255), sf::Color(255, 255, 0, 255), window);
		else w_Print(aux, 44, 20, colors[0], sf::Color::Black, window);

		sfText.setString("*");
		sfText.setFillColor(sf::Color::Black);
		if (b_verticalCursor)
			sfText.setPosition(sf::Vector2f(374.f, 475.f)), window.draw(sfText);
		else
			sfText.setPosition(sf::Vector2f(550.f, 475.f)), window.draw(sfText);
		if (b_blinkerCursor)
			sfText.setPosition(sf::Vector2f(374.f, 495.f)), window.draw(sfText);
		else
			sfText.setPosition(sf::Vector2f(550.f, 495.f)), window.draw(sfText);
	}
	if (b_settingsEnv)
	{
		sfText.setString(L"║             │\n║             │\n╟─────────────┘\n║\n╟─────────────┐\n║             │\n║             │\n");
		sfText.setPosition(sf::Vector2f(110.f, 35.f));
		sfText.setFillColor(sf::Color::Black);
		window.draw(sfText);

		sfText.setString(L"               ── Default Directory ──\n\nDir:\n\n\n\n               ── External Functions ──\n\nFunctions:\n");
		sfText.setPosition(sf::Vector2f(286.f, 58.f));
		sfText.setFillColor(sf::Color::Black);
		window.draw(sfText);

		strcpy(aux, " Change ");
		if (mPos.y == 5 && mPos.x >= 70 && mPos.x <= 77) w_Print(aux, 70, 5, sf::Color(0, 102, 0, 255), sf::Color(255, 255, 0, 255), window);
		else w_Print(aux, 70, 5, colors[0], sf::Color::Black, window);
		strcpy(aux, " Add ");
		if (mPos.y == 22 && mPos.x >= 26 && mPos.x <= 30) w_Print(aux, 26, 22, sf::Color(0, 102, 0, 255), sf::Color(255, 255, 0, 255), window);
		else w_Print(aux, 26, 22, colors[0], sf::Color::Black, window);

		int k, i;
		k = strlen(afritLocation);
		if (k > 50)
		{
			for (i = 0; i < 50; i++)
				aux[i] = afritLocation[i + k - 50];
			aux[0] = '.', aux[1] = '.', aux[2] = '.', aux[50] = 0;
		}
		else
			strcpy(aux, afritLocation);
		w_Print(aux, 31, 5, colors[0], sf::Color::Blue, window);

		strcpy(aux, "cmdwiz");
		w_Print(aux, 26, 12, colors[0], sf::Color::Blue, window);
		strcpy(aux, "batbox");
		w_Print(aux, 26, 13, colors[0], sf::Color::Blue, window);
	}
	if (b_settingsOth)
	{
		sfText.setString(L"║             │\n║             │\n║             │\n║             │\n╟─────────────┘\n║\n╟─────────────┐\n");
		sfText.setPosition(sf::Vector2f(110.f, 35.f));
		sfText.setFillColor(sf::Color::Black);
		window.draw(sfText);

		sfText.setString(L"Real-Time Update:        [ ] On       [ ] Off\n");
		sfText.setPosition(sf::Vector2f(286.f, 55.f));
		sfText.setFillColor(sf::Color::Black);
		window.draw(sfText);

		sfText.setString("*");
		sfText.setFillColor(sf::Color::Black);
		if (b_rtUpdate)
			sfText.setPosition(sf::Vector2f(572.f, 55.f)), window.draw(sfText);
		else
			sfText.setPosition(sf::Vector2f(715.f, 55.f)), window.draw(sfText);

		sfText.setString(L"Bluelight Filter:        [ ] On       [ ] Off\n");
		sfText.setPosition(sf::Vector2f(286.f, 75.f));
		sfText.setFillColor(sf::Color::Black);
		window.draw(sfText);

		sfText.setString("*");
		sfText.setFillColor(sf::Color::Black);
		if (b_blueFilter)
			sfText.setPosition(sf::Vector2f(572.f, 75.f)), window.draw(sfText);
		else
			sfText.setPosition(sf::Vector2f(715.f, 75.f)), window.draw(sfText);
	}
}
void drawNewFileMenu(sf::RenderWindow &window, char text[])
{
	sf::RectangleShape shape0(sf::Vector2f(473.f, 220.f));
	shape0.setPosition(sf::Vector2f(253.f, 40.f));
	shape0.setFillColor(sf::Color(230, 230, 230, 255));
	window.draw(shape0);

	shape0.setSize(sf::Vector2f(11.f, 200.f));    //v shader
	shape0.setPosition(sf::Vector2f(726.f, 60.f));
	shape0.setFillColor(sf::Color(0, 0, 0, 150));
	window.draw(shape0);

	shape0.setSize(sf::Vector2f(473.f, 20.f));    //h shader
	shape0.setPosition(sf::Vector2f(264.f, 260.f));
	window.draw(shape0);

	sfText.setString(L"╔═══ NEW FILE ══════════════════════════╤═╗\n║                                       │ ║\n║ Directory & Name:                     └─╢\n║ └>                                      ║\n║                                         ║\n║ Evaluate as:                            ║\n║  - CMD (Batch Script)            [ ]    ║\n║  - Plain Text                    [ ]    ║\n║                                         ║\n║                                         ║\n╚═════════════════════════════════════════╝");
	sfText.setPosition(sf::Vector2f(253.f, 40.f));
	sfText.setFillColor(sf::Color::Black);
	window.draw(sfText);

	char aux[51];
	sf::Vector2i mPos(sf::Mouse::getPosition(window));
	mPos.x /= 11;
	mPos.y /= 20;

	strcpy(aux, "  CREATE->  ");
	if (mPos.y == 11 && mPos.x >= 52 && mPos.x <= 63) w_Print(aux, 52, 11, sf::Color(0, 102, 0, 255), sf::Color(255, 255, 0, 255), window);
	else w_Print(aux, 52, 11, colors[0], sf::Color::Black, window);
	strcpy(aux, "X");
	if (mPos.y == 3 && mPos.x == 64) w_Print(aux, 64, 3, sf::Color(255, 0, 0, 255), sf::Color::Black, window);
	else w_Print(aux, 64, 3, colors[0], sf::Color::Black, window);

	if (text[0]) // type
	{
		if (strlen(text) <= 30)	w_Print(text, 27, 5, colors[0], sf::Color::Blue, window);
		else w_Print(text + strlen(text) - 30, 27, 5, colors[0], sf::Color::Blue, window);
	}

	sfText.setString("*"); //evaluator
	sfText.setFillColor(sf::Color::Black);
	if (b_evalCMD)
		sfText.setPosition(sf::Vector2f(649.f, 160.f)), window.draw(sfText);
	else
		sfText.setPosition(sf::Vector2f(649.f, 180.f)), window.draw(sfText);
}
void drawOpenFileMenu(sf::RenderWindow &window, char text[])
{
	sf::RectangleShape shape0(sf::Vector2f(473.f, 220.f));
	shape0.setPosition(sf::Vector2f(253.f, 60.f));
	shape0.setFillColor(sf::Color(230, 230, 230, 255));
	window.draw(shape0);

	shape0.setSize(sf::Vector2f(11.f, 200.f));    //v shader
	shape0.setPosition(sf::Vector2f(726.f, 80.f));
	shape0.setFillColor(sf::Color(0, 0, 0, 150));
	window.draw(shape0);

	shape0.setSize(sf::Vector2f(473.f, 20.f));    //h shader
	shape0.setPosition(sf::Vector2f(264.f, 280.f));
	window.draw(shape0);

	sfText.setString(L"╔═══ OPEN FILE ═════════════════════════╤═╗\n║                                       │ ║\n║ Directory & Name:                     └─╢\n║ └>                                      ║\n║                                         ║\n║ Evaluate as:                            ║\n║  - CMD (Batch Script)            [ ]    ║\n║  - Plain Text                    [ ]    ║\n║                                         ║\n║                                         ║\n╚═════════════════════════════════════════╝");
	sfText.setPosition(sf::Vector2f(253.f, 60.f));
	sfText.setFillColor(sf::Color::Black);
	window.draw(sfText);

	char aux[51];
	sf::Vector2i mPos(sf::Mouse::getPosition(window));
	mPos.x /= 11;
	mPos.y /= 20;

	strcpy(aux, "  OPEN->  ");
	if (mPos.y == 12 && mPos.x >= 54 && mPos.x <= 63) w_Print(aux, 54, 12, sf::Color(0, 102, 0, 255), sf::Color(255, 255, 0, 255), window);
	else w_Print(aux, 54, 12, colors[0], sf::Color::Black, window);
	strcpy(aux, "X");
	if (mPos.y == 4 && mPos.x == 64) w_Print(aux, 64, 4, sf::Color(255, 0, 0, 255), sf::Color::Black, window);
	else w_Print(aux, 64, 4, colors[0], sf::Color::Black, window);

	if (text[0]) // type
	{
		if (strlen(text) <= 30)	w_Print(text, 27, 6, colors[0], sf::Color::Black, window);
		else w_Print(text + strlen(text) - 30, 27, 6, colors[0], sf::Color::Black, window);
	}

	sfText.setString("*"); //evaluator
	sfText.setFillColor(sf::Color::Black);
	if (b_evalCMD)
		sfText.setPosition(sf::Vector2f(649.f, 180.f)), window.draw(sfText);
	else
		sfText.setPosition(sf::Vector2f(649.f, 200.f)), window.draw(sfText);
}

int main()
{
	//-------------STARTUP---------------//
	ShowWindow(::GetConsoleWindow(), SW_HIDE);
	sf::RenderWindow startupWin(sf::VideoMode(484, 176), "Afrit Startup", sf::Style::None);
	tx.loadFromFile("title.png");
	tx.setSmooth(false);
	sp.setTexture(tx);
	sp.setScale(sf::Vector2f(4.f, 4.f));
	startupWin.draw(sp);
	startupWin.display();

	icon_tx.loadFromFile("iconx.png");
	icon_tx.setSmooth(false);
	icon_sp.setTexture(tx);

	sf::Image icon;
	icon.loadFromFile("icon.png");

	font0.loadFromFile("dosfont.ttf"); //load font
	sfText.setFont(font0);
	sfText.setCharacterSize(20);
	
	std::ifstream dataIn("data0.dat"); //load recent projects
	while (dataIn.getline(recentFiles[recentCnt], 255)) recentCnt++;
	dataIn.close();

	std::ifstream prefIn("pref.txt"); //load preferences
	int l, r, g, b;
	for (l = 0; l <= 12; l++)
	{
		prefIn >> r >> g >> b;
		if (r != -1) colors[l].r = r;
		if (g != -1) colors[l].g = g;
		if (b != -1) colors[l].b = b;
	}
	prefIn >> b_verticalCursor >> b_blinkerCursor >> b_rtUpdate >> b_blueFilter;
	prefIn.close();

	RECT dRes;
	SystemParametersInfo(SPI_GETWORKAREA, 0, &dRes, FALSE);
	maxTextView = sf::Vector2i((dRes.right - dRes.left) / 11 - 5, (dRes.bottom - dRes.top) / 20 - 2);
	deskSize = sf::Vector2f(maxTextView.x * 11, maxTextView.y * 20);
	maxTextView.x -= 6; maxTextView.y -= 4;
	generalBg.setSize(deskSize);
	generalBg.setFillColor(colors[0]);

	_getcwd(afritLocation, 255);

	clearSelect(); //clears selection

	sf::Vector2i mousePos;

	char aux0[256] = { 0 }, aux1[1001];
	long unsigned int eCode;

	//----------END Startup--------------//

	Sleep(1500);
	startupWin.close();
	sf::RenderWindow window(sf::VideoMode(deskSize.x, deskSize.y), "Afrit", sf::Style::Close);
	window.setPosition(sf::Vector2i(0, 0));
	window.setIcon(100, 100, icon.getPixelsPtr());
	window.setFramerateLimit(20);

	int tics = 0;

	while (window.isOpen())
	{
		sf::Event event;
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
				onExit(window);
			else if (event.type == sf::Event::EventType::MouseButtonPressed)
			{
				mousePos = sf::Mouse::getPosition(window);
				if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
				{
					mousePos.x /= 11;
					mousePos.y /= 20;
					//std::cout << mousePos.x << " " << mousePos.y << '\n';

					if (mousePos.y == 0) //menu bar
					{
						if (mousePos.x >= 0 && mousePos.x <= 7)
							b_file = true, b_edit = false, b_settingsEdt = false, b_settingsEnv = false, b_settingsOth = false, b_cursor = false;
						else if (mousePos.x >= 9 && mousePos.x <= 16)
							b_file = false, b_edit = true, b_settingsEdt = false, b_settingsEnv = false, b_settingsOth = false, b_cursor = false;
						else if (mousePos.x >= 18 && mousePos.x <= 29)
							b_file = false, b_edit = false, b_settingsEdt = true, b_settingsEnv = false, b_settingsOth = false, b_cursor = false;
						else if (mousePos.x >= 31 && mousePos.x <= 38)
						{
							b_file = false, b_edit = false, b_settingsEdt = false, b_settingsEnv = false, b_settingsOth = false, b_cursor = false;
							ShellExecute(0, 0, "http://info.tm.edu.ro:8088", 0, 0, SW_SHOW);
						}
					}
					else if (b_saveBefore) //safe save
					{
						if (mousePos.y == 8 && mousePos.x >= 36 && mousePos.x <= 40) saveBefOut = 1;
						else if (mousePos.y == 8 && mousePos.x >= 42 && mousePos.x <= 45) saveBefOut = 2;
						else if (mousePos.y == 8 && mousePos.x >= 47 && mousePos.x <= 54) saveBefOut = 0;
						if (saveBefInh == 1) onExit(window);
						else if (saveBefInh == 2) returnMainMenu();
					}
					else if (b_warning0) //error code warn
					{
						if (mousePos.y == 9 && mousePos.x >= 39 && mousePos.x <= 43)
						{
							if (warn0Inh == 1) buildCode();
							else if (warn0Inh == 2) buildCode(), runCode();
							b_warning0 = false;
						}
						else if (mousePos.y == 9 && mousePos.x >= 54 && mousePos.x <= 57) b_warning0 = false;
					}
					else if (b_error0) //error opening file
					{
						if (mousePos.y == 7 && mousePos.x >= 32 && mousePos.x <= 62) b_error0 = false;
					}
					else if (b_newFile) //newfile menu
					{
						if (b_cursor == false || b_code) //cursor
							b_cursor = true, cursorPos = sf::Vector2i(27, 5);
						if (mousePos.x == 64 && mousePos.y == 3) //X
						{
							b_newFile = false;
							memset(aux0, 0, 101);
							if (!b_code) b_cursor = false;
						}
						else if (mousePos.x == 59 && mousePos.y == 8) //eval on
							b_evalCMD = true;
						else if (mousePos.x == 59 && mousePos.y == 9) //eval off
							b_evalCMD = false;
						else if (mousePos.x >= 55 && mousePos.x <= 62 && mousePos.y == 11) // CREATE
						{
							b_code = true, b_newFile = false, b_file = false, b_title = false, b_saved = false;
							codeLines = 0;
							memset(userCode, 0, sizeof(userCode[0][0]) * 1000000);
							strcpy(userCode[0], "@echo off");
							strcpy(userCode[1], "echo Hello World!");
							strcpy(userCode[2], "pause > nul");
							codeLines = 3;
							cursorTxtPos = sf::Vector2i(strlen(userCode[codeLines - 1]), codeLines - 1);
							strcpy(codeName, aux0);
							strcpy(projectName, strtok(aux0, "."));
							memset(aux0, 0, 101);
						}
					}
					else if (b_openFile) //openfile menu
					{
						if (b_cursor == false || b_code) //cursor
							b_cursor = true, cursorPos = sf::Vector2i(27, 6);
						if (mousePos.x == 64 && mousePos.y == 4) //X
						{
							b_openFile = false;
							memset(aux0, 0, 101);
							if (!b_code) b_cursor = false;
						}
						else if (mousePos.x == 59 && mousePos.y == 9) //eval on
							b_evalCMD = true;
						else if (mousePos.x == 59 && mousePos.y == 10) //eval off
							b_evalCMD = false;
						else if (mousePos.x >= 57 && mousePos.x <= 62 && mousePos.y == 12 && aux0[0]) // OPEN
						{
							codeLines = 0;
							memset(userCode, 0, sizeof(userCode[0][0]) * 1000000);
							if (fLoadCode(aux0))
								initCode(aux0);
							else
								b_error0 = true;
						}
					}
					else if (b_file) //file menu
					{
						if (mousePos.x < 1 || mousePos.x > 23 || mousePos.y < 1 || mousePos.y > 8)
						{
							b_file = false;
							if (b_code) b_cursor = true;
						}
						else if (!b_code && mousePos.x >= 1 && mousePos.x <= 22 && mousePos.y == 2)
							b_newFile = true;
						else if (!b_code && mousePos.x >= 1 && mousePos.x <= 19 && mousePos.y == 3)
							b_openFile = true;
						else if (b_code && mousePos.y == 4 && mousePos.x >= 1 && mousePos.x <= 23)
							fSaveCode();
						else if (b_code && mousePos.y == 6 && mousePos.x >= 1 && mousePos.x <= 23) //main menu
							returnMainMenu();
					}
					else if (b_edit) //edit menu
					{
						if (mousePos.x < 9 || mousePos.x > 35 || mousePos.y < 1 || mousePos.y > 12) //close
						{
							b_edit = false;
							if (b_code) b_cursor = true;
						}
						else if (mousePos.y == 2 && b_code) undo();
						else if (mousePos.y == 3 && b_code) redo();
						else if (mousePos.y == 5 && b_code) copyToClipboard();
						else if (mousePos.y == 6 && b_code) copyToClipboard(), cutCode();
						else if (mousePos.y == 7 && b_code)
						{
							if (codeSelection.end.x != -1)
								cursorTxtPos = codeSelection.start;
							cutCode(), pasteCode(); //paste
						}
						else if (mousePos.x >= 10 && mousePos.x <= 34 && mousePos.y == 9 && b_code)
						{
							if (!b_find)
								b_find = true;
							else
								b_find = false;
						}
						else if (mousePos.x >= 10 && mousePos.x <= 34 && mousePos.y == 10 && b_code)
							selectAll();
					}
					else if (b_settingsEdt || b_settingsEnv || b_settingsOth) //settings
					{
						if (mousePos.x == 81 && mousePos.y == 2)
						{
							b_settingsEdt = false, b_settingsEnv = false, b_settingsOth = false;
							if (b_code) b_cursor = true;
						}
						else if(mousePos.x >= 11 && mousePos.x <= 24 && mousePos.y == 3)
							b_settingsEdt = true, b_settingsEnv = false, b_settingsOth = false;
						else if (mousePos.x >= 11 && mousePos.x <= 24 && mousePos.y == 5)
							b_settingsEdt = false, b_settingsEnv = true, b_settingsOth = false;
						else if (mousePos.x >= 11 && mousePos.x <= 24 && mousePos.y == 7)
							b_settingsEdt = false, b_settingsEnv = false, b_settingsOth = true;

						else if (b_settingsEdt)
						{
							if (mousePos.x == 34 && mousePos.y == 24) b_verticalCursor = true;
							else if (mousePos.x == 50 && mousePos.y == 24) b_verticalCursor = false;
							else if (mousePos.x == 34 && mousePos.y == 25) b_blinkerCursor = true;
							else if (mousePos.x == 50 && mousePos.y == 25) b_blinkerCursor = false;
							else if (mousePos.x >= 44 && mousePos.x <= 61 && mousePos.y == 20)
								for (int i = 0; i <= 12; i++)
									colors[i] = def_colors[i];
						}
						else if (b_settingsOth)
						{
							if (mousePos.x == 52 && mousePos.y == 3) b_rtUpdate = true;
							else if(mousePos.x == 65 && mousePos.y == 3) b_rtUpdate = false;
							else if (mousePos.x == 52 && mousePos.y == 4) b_blueFilter = true;
							else if (mousePos.x == 65 && mousePos.y == 4) b_blueFilter = false;
						}
					}
					else if (b_code)
					{
						if (mousePos.x >= cursorOffset.x && mousePos.x <= 165 && mousePos.y >= cursorOffset.y && mousePos.y <= 46) //cursor set click
						{
							clearSelect(); // clears selection
							cursorTxtPos = mousePos - cursorOffset + scroll;
							if (cursorTxtPos.x > strlen(userCode[cursorTxtPos.y])) cursorTxtPos.x = strlen(userCode[cursorTxtPos.y]);
							if (cursorTxtPos.y >= codeLines) cursorTxtPos.y = codeLines - 1;
						}
						else if (mousePos.y == 1 && mousePos.x >= 0 && mousePos.x <= 11)//build
						{
							bool ok = true;
							for (int i = 0; i < codeLines && ok; i++)
								if (errorList[i] != 0)
									ok = false;
							if (ok)
								buildCode();
							else
								warn0Inh = 1, b_warning0 = true;
						}
						else if (mousePos.y == 1 && mousePos.x >= 13 && mousePos.x <= 31)//build & run
						{
							bool ok = true;
							for (int i = 0; i < codeLines && ok; i++)
								if (errorList[i] != 0)
									ok = false;
							if (ok)
								buildCode(), runCode();
							else
								warn0Inh = 2, b_warning0 = true;
						}
						else if (b_running && (mousePos.y == 1 && mousePos.x >= 33 && mousePos.x <= 51)) stopCode(); //force stop
						else if (mousePos.y == 2 && mousePos.x >= deskSize.x / 11 - 2 && mousePos.x <= deskSize.x / 11 && scroll.y > 0) //vscroll
						{
							scroll.y--;
							if (cursorTxtPos.y > scroll.y) cursorTxtPos.y--;
							if (cursorTxtPos.x > strlen(userCode[cursorTxtPos.y])) cursorTxtPos.x = strlen(userCode[cursorTxtPos.y]);
						}
						else if (mousePos.y == deskSize.y / 20 - 3 && mousePos.x >= deskSize.x / 11 - 2 && mousePos.x <= deskSize.x / 11 && scroll.y < codeLines - maxTextView.y) // vscroll
						{
							scroll.y++;
							if (cursorTxtPos.y < scroll.y) cursorTxtPos.y++;
							if (cursorTxtPos.x > strlen(userCode[cursorTxtPos.y])) cursorTxtPos.x = strlen(userCode[cursorTxtPos.y]);
						}
						else if (mousePos.y == deskSize.y / 20 - 2 && mousePos.x >= 5 && mousePos.x <= 7 && scroll.x > 0) //hscroll
						{
							scroll.x--;
							if (cursorTxtPos.x > scroll.x + maxTextView.x) cursorTxtPos.x--;
						}
						else if (mousePos.y == deskSize.y / 20 - 2 && mousePos.x >= deskSize.x / 11 - 4 && mousePos.x <= deskSize.x / 11 - 2 && scroll.x + maxTextView.x < strlen(userCode[cursorTxtPos.y])) //hscroll
						{
							scroll.x++;
							if (cursorTxtPos.y < scroll.y) cursorTxtPos.y++;
						}
					}
					else if (b_title)
					{
						if(!b_file && !b_edit && !b_settingsEdt && !b_settingsEnv && !b_settingsOth)
						{
							int i;
							for (i = 0; i < recentCnt; i++)
							{
								if (mousePos.y == i + 20 && mousePos.x >= 5 && mousePos.x <= 58) //open
								{
									codeLines = 0;
									memset(userCode, 0, sizeof(userCode[0][0]) * 1000000);
									if (fLoadCode(recentFiles[i])) initCode(recentFiles[i]);
									else //remove
									{
										b_error0 = true;
										int j;
										for (j = i; i < recentCnt; i++)
											strcpy(recentFiles[i], recentFiles[i + 1]);
										recentCnt--;
									}
								}
								else if (mousePos.y == i + 20 && mousePos.x == 59) // remove from list
								{
									int j;
									for (j = i; i < recentCnt; i++)
										strcpy(recentFiles[i], recentFiles[i + 1]);
									recentCnt--;
								}
							}
						}
					}
				}
				else if (sf::Mouse::isButtonPressed(sf::Mouse::Right))
				{
					mousePos.x /= 11;
					mousePos.y /= 20;
					mousePos -= cursorOffset;
					mousePos += scroll;
					if (b_code)
					{
						if (codeSelection.start.x != -1 && codeSelection.end.x != -1)
							clearSelect();
						if (codeSelection.start.x == -1)
						{
							if (mousePos.x > strlen(userCode[mousePos.y]) - 1)
								codeSelection.start.x = strlen(userCode[mousePos.y]) - 1;
							else
								codeSelection.start.x = mousePos.x;
							if (mousePos.y > codeLines - 1)
								codeSelection.start.y = codeLines - 1;
							else
								codeSelection.start.y = mousePos.y;
						}
						else if (codeSelection.end.x == -1)
						{
							if (mousePos.x > strlen(userCode[mousePos.y]) - 1)
								codeSelection.end.x = strlen(userCode[mousePos.y]) - 1;
							else
								codeSelection.end.x = mousePos.x;
							if (mousePos.y > codeLines - 1)
								codeSelection.end.y = codeLines - 1;
							else
								codeSelection.end.y = mousePos.y;
							if (codeSelection.end.y < codeSelection.start.y)
								std::swap(codeSelection.end, codeSelection.start);
							else if (codeSelection.end.y == codeSelection.start.y && codeSelection.end.x < codeSelection.start.x) 
								std::swap(codeSelection.start.x, codeSelection.end.x);
						}
						else clearSelect();
					}
				}
				while (window.pollEvent(event));
			}
			else if (event.type == sf::Event::TextEntered)
			{
				if (b_cursor)
				{
					if (b_code && !b_newFile && !b_openFile)
					{
						if (event.text.unicode >= 32 && event.text.unicode <= 126) //text
						{
							char auxu[1001]; strcpy(auxu, userCode[cursorTxtPos.y]);

							if (codeSelection.end.x != -1)
								cursorTxtPos = codeSelection.start, cutCode();
							strcpy(aux1, userCode[cursorTxtPos.y] + cursorTxtPos.x);
							strcpy(userCode[cursorTxtPos.y] + cursorTxtPos.x + 1, aux1);
							userCode[cursorTxtPos.y][cursorTxtPos.x] = event.text.unicode;
							cursorTxtPos.x++;
							b_saved = false;
			
							addUndo(cursorTxtPos.y, auxu, userCode[cursorTxtPos.y], 0);
						}
						else if (event.text.unicode == 13) //enter
						{
							if (codeSelection.end.x != -1)
								cursorTxtPos = codeSelection.start, cutCode();
							codeLines++;
							for (int i = codeLines; i > cursorTxtPos.y + 1; i--)
								strcpy(userCode[i], userCode[i - 1]);
							strcpy(userCode[cursorTxtPos.y + 1], userCode[cursorTxtPos.y] + cursorTxtPos.x);
							userCode[cursorTxtPos.y][cursorTxtPos.x] = 0;
							cursorTxtPos.x = 0;
							cursorTxtPos.y++;
							b_saved = false;
						}
						else if (event.text.unicode == 8) //backspace
						{
							if (codeSelection.end.x != -1)
								cursorTxtPos = codeSelection.start, cutCode();
							else if (cursorTxtPos.x > 0)
							{
								char auxu[1001]; strcpy(auxu, userCode[cursorTxtPos.y]);
								strcpy(userCode[cursorTxtPos.y] + cursorTxtPos.x - 1, userCode[cursorTxtPos.y] + cursorTxtPos.x), cursorTxtPos.x--;
								addUndo(cursorTxtPos.y, auxu, userCode[cursorTxtPos.y], 0);
							}
							else if (cursorTxtPos.y > 0)
							{
								cursorTxtPos.x = strlen(userCode[cursorTxtPos.y - 1]);
								strcat(userCode[cursorTxtPos.y - 1], userCode[cursorTxtPos.y]);
								for (int i = cursorTxtPos.y; i < codeLines; i++)
									strcpy(userCode[i], userCode[i + 1]);
								codeLines--;
								cursorTxtPos.y--;
							}
							b_saved = false;
						}
					}
					else if (((b_newFile || b_openFile ) && !strchr("<\"/|?*", event.text.unicode)) && event.text.unicode >= 32 && event.text.unicode <= 126 && strlen(aux0) < 255)
					{
						aux0[strlen(aux0)] = event.text.unicode; aux0[strlen(aux0)] = 0;
						if (strlen(aux0) <= 30) cursorPos.x++;
					}
					else if ((b_newFile || b_openFile) && event.text.unicode == 8 && cursorPos.x > 27)
					{
						strcpy(aux0 + strlen(aux0) - 1, aux0 + strlen(aux0));
						if (strlen(aux0) < 30) cursorPos.x--;
					}
					else if (event.text.unicode == 13 && b_newFile)
					{
						b_code = true, b_newFile = false, b_file = false, b_title = false, b_saved = false;
						codeLines = 0;
						memset(userCode, 0, sizeof(userCode[0][0]) * 1000000);
						strcpy(userCode[0], "@echo off");
						strcpy(userCode[1], "echo Hello World!");
						strcpy(userCode[2], "pause > nul");
						codeLines = 3;
						cursorTxtPos = sf::Vector2i(strlen(userCode[codeLines - 1]), codeLines - 1);
						strcpy(codeName, aux0);
						strcpy(projectName, strtok(aux0, "."));
						memset(aux0, 0, 101);
					}
					else if (event.text.unicode == 13 && b_openFile)
					{
						codeLines = 0;
						memset(userCode, 0, sizeof(userCode[0][0]) * 1000000);
						if (fLoadCode(aux0))
							initCode(aux0);
						else
							b_error0 = true;
					}
				}
			}
			else if (event.type == sf::Event::KeyPressed) // arrows and special
			{
				if (cursorPos.y < cursorOffset.y && scroll.y > 0) scroll.y--;
				if (cursorPos.x < cursorOffset.x + 2 && scroll.x > 0) scroll.x--;
				if (cursorPos.y > 40 && scroll.x < codeLines - 40) scroll.y++;
				if (cursorPos.x > 160 && scroll.x < 10000) scroll.x++;
				if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left) && cursorTxtPos.x > 0) cursorTxtPos.x--;
				else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right) && cursorTxtPos.x < strlen(userCode[cursorTxtPos.y])) cursorTxtPos.x++;
				else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up) && cursorTxtPos.y > 0)
				{
					cursorTxtPos.y--;
					if (cursorTxtPos.x > strlen(userCode[cursorTxtPos.y])) cursorTxtPos.x = strlen(userCode[cursorTxtPos.y]);
				}
				else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down) && cursorTxtPos.y < codeLines - 1)
				{
					cursorTxtPos.y++;
					if (cursorTxtPos.x > strlen(userCode[cursorTxtPos.y])) cursorTxtPos.x = strlen(userCode[cursorTxtPos.y]);
				}
				//hotkeys
				else if (b_code)
				{
					if (sf::Keyboard::isKeyPressed(sf::Keyboard::LControl))
					{
						if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) fSaveCode(); //save
						else if (sf::Keyboard::isKeyPressed(sf::Keyboard::C)) copyToClipboard(); //copy
						else if (sf::Keyboard::isKeyPressed(sf::Keyboard::V))
						{
							if (codeSelection.end.x != -1)
								cursorTxtPos = codeSelection.start;
							cutCode(), pasteCode(); //paste
						}
						else if (sf::Keyboard::isKeyPressed(sf::Keyboard::X)) copyToClipboard(), cutCode(); //cut
						else if (sf::Keyboard::isKeyPressed(sf::Keyboard::LAlt) && sf::Keyboard::isKeyPressed(sf::Keyboard::Z)) redo();
						else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Z)) undo();
						else if (sf::Keyboard::isKeyPressed(sf::Keyboard::F))
						{
							if (!b_find)
								b_find = true;
							else
								b_find = false;
						}
						else if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) selectAll(); //select all
					}
					else if (sf::Keyboard::isKeyPressed(sf::Keyboard::F8)) buildCode();
					else if (sf::Keyboard::isKeyPressed(sf::Keyboard::F9)) buildCode(), runCode();
					else if (b_running && sf::Keyboard::isKeyPressed(sf::Keyboard::F10)) stopCode();
					else if (sf::Keyboard::isKeyPressed(sf::Keyboard::LControl) && sf::Keyboard::isKeyPressed(sf::Keyboard::A)) selectAll();
				}
			}
			else if (event.type == sf::Event::EventType::MouseWheelScrolled)
			{
				if (b_code)
				{
					if (event.mouseWheelScroll.delta < 0 && scroll.y < codeLines - maxTextView.y)
					{
						scroll.y++;
						if (scroll.y < codeLines - maxTextView.y - 2) scroll.y += 2;
						if (cursorTxtPos.y < scroll.y) cursorTxtPos.y = scroll.y;
						if (cursorTxtPos.x > strlen(userCode[cursorTxtPos.y])) cursorTxtPos.x = strlen(userCode[cursorTxtPos.y]);
					}
					else if (event.mouseWheelScroll.delta > 0 && scroll.y > 0)
					{
						scroll.y--;
						if (scroll.y > 2) scroll.y -= 2;
						if (cursorTxtPos.y > scroll.y) cursorTxtPos.y = scroll.y;
						if (cursorTxtPos.x > strlen(userCode[cursorTxtPos.y])) cursorTxtPos.x = strlen(userCode[cursorTxtPos.y]);
					}
				}
			}
		}
		window.clear();
		window.draw(generalBg);
		if (b_title) drawTitleScreen(window);
		if (b_code)
		{
			if (b_running)
			{
				GetExitCodeProcess(codeProcess.hProcess, &eCode);
				if (eCode != 259) b_running = false;
			}
			if (tics % 10 == 0) //error check
			{
				if (b_rtUpdate && b_running)
				{
					b_saved = false;
					char aux[256];
					strcpy(aux, codeLocation);
					strcat(aux, projectName);
					strcat(aux, ".bat");
					aux[strlen(aux)] = 0;
					codeLines = 0;
					memset(userCode, 0, sizeof(userCode[0][0]) * 1000000);
					if (fLoadCode(aux))
						initCode(aux);
				}
				if(b_evalCMD)
					for (int i = 0; i < codeLines; i++)
						evalCode(userCode[i], i);
			}
			if (!b_newFile && !b_openFile) cursorPos = cursorTxtPos + cursorOffset - scroll;
			if (cursorPos.y < cursorOffset.y && scroll.y > 0) scroll.y--;
			if (cursorPos.x < cursorOffset.x + 2 && scroll.x > 0) scroll.x--;
			if (cursorPos.y > 40 && scroll.x < codeLines - 40) scroll.y++;
			if (cursorPos.x > 160 && scroll.x < 10000) scroll.x++;
			drawCodeScreen(window);
		}
		drawEditorHood(window);
		if (b_file)
		{
			drawFileMenu(window);
			if (b_newFile) drawNewFileMenu(window, aux0);
			else if (b_openFile) drawOpenFileMenu(window, aux0);
		}
		else if (b_edit) drawEditMenu(window);
		else if (b_settingsEdt || b_settingsEnv || b_settingsOth) drawSettingsMenu(window);
		if (b_cursor) drawCursor(cursorPos, window);
		if (b_error0) drawError0(window);
		if (b_warning0) drawWarning0(window);
		else if (b_saveBefore) drawSaveBefore(window);
		if (b_blueFilter) drawBlueFilter(window);
		window.display();

		if (tics == 1000) tics = 0;
		else tics++;
	}
	return 0;
}