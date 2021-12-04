#include <iostream>
#define WINVER 0x0502
#include <windows.h>
#include <stdio.h>
#include <string>
#include <cstring>
#include <sddl.h>

using namespace std;

static PSECURITY_DESCRIPTOR create_security_descriptor()
{
const char* sddl ="D:(A;OICI;GRGW;;;AU)(A;OICI;GA;;;BA)";
PSECURITY_DESCRIPTOR security_descriptor = NULL;
ConvertStringSecurityDescriptorToSecurityDescriptor(sddl, SDDL_REVISION_1, &security_descriptor, NULL);
return security_descriptor;
}

static SECURITY_ATTRIBUTES create_security_attributes()
{
SECURITY_ATTRIBUTES attributes;
attributes.nLength = sizeof(attributes);
attributes.lpSecurityDescriptor = create_security_descriptor();
attributes.bInheritHandle = FALSE;
return attributes;
}

BOOL WINAPI Create_MailSlot(LPCTSTR lpSlotName, HANDLE& hslot, bool& isserver)
{
auto attributes = create_security_attributes();
hslot = CreateMailslot(lpSlotName,NULL,MAILSLOT_WAIT_FOREVER,&attributes);

if (hslot == INVALID_HANDLE_VALUE)
{
DWORD err = GetLastError();

if (err == ERROR_ALREADY_EXISTS || err ==ERROR_INVALID_NAME)
{
isserver = FALSE;
hslot = CreateFile(lpSlotName,GENERIC_WRITE,FILE_SHARE_READ,(LPSECURITY_ATTRIBUTES)NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,(HANDLE)NULL);

if (hslot == INVALID_HANDLE_VALUE)
{
DWORD err = GetLastError();
printf("Ошибка при выполнении функции CreateFile(): %d\n", GetLastError());
return FALSE;
}
}
else
{
printf("Ошибка при выполнении функции CreateFile(): %d\n", GetLastError());
return FALSE;
}
}
else
{
isserver = TRUE;
}
return TRUE;
}

BOOL Get_Messages(HANDLE hSlot, DWORD* cbMessage, DWORD* cMessage)
{
DWORD cbMessageL, cMessageL;
BOOL hmessage;

hmessage = GetMailslotInfo(hSlot,(LPDWORD) NULL,&cbMessageL,&cMessageL,(LPDWORD) NULL);

if (hmessage)
{
printf("Найдено %i сообщений на почтовом ящике\n", cMessageL);
if (cbMessage != NULL)*cbMessage = cbMessageL;
if (cMessage != NULL)*cMessage = cMessageL;
return TRUE;
}
else
{
printf("Ошибка при выполнении функции GetMailslotInfo(): %d\n", GetLastError());
return FALSE;
}
}
BOOL Write_Message(HANDLE hSlot, LPCTSTR lpszMessage)
{
BOOL hwrite;
DWORD cbWritten;

hwrite = WriteFile(hFile,lpszMessage,(lstrlen(lpszMessage) + 1) * sizeof(TCHAR),&cbWritten,(LPOVERLAPPED)NULL);

if (hwrite ==0)
{
printf("Ошибка при выполнении функции WriteFile(): %d\n", GetLastError());
return FALSE;
}
else
{
printf("Сообщение успешно отправлено.\n");
return TRUE;
}
}

BOOL Read_Message(HANDLE hfSlot)
{
DWORD cbrMessage, crMessage, cbrRead;
BOOL hread;
cbrMessage = crMessage = cbrRead = 0;
hread = GetMailslotInfo(hfSlot,NULL,&cbrMessage,&crMessage,NULL);

if (cbrMessage == MAILSLOT_NO_MESSAGE)
{
printf("Сообщения в почтовом ящике отсутствуют.\n");
return TRUE;
}

if (hread==0)
{
printf("Ошибка при выполнении функции GetMailslotInfo: %d\n", GetLastError());
return FALSE;
}

string Message(cbrMessage,'\0');

hread = ReadFile(hfSlot,&Message[0], Message.size(),&cbrRead,NULL);

if (hread == 0)
{
printf("Ошибка при попытке прочитать содержимое файла: %d\n", GetLastError());
return FALSE;
}
cout « Message;
return TRUE;
}

int main()
{
DWORD* cbMessage;
DWORD* cMessage;
cbMessage = cMessage = 0;
string InputName;

setlocale(LC_ALL, "Russian");

cout « "Введите почтовый ящик:";
cin » InputName;

LPCTSTR SlotName = InputName.c_str();

HANDLE hSlot;
bool isserver;

bool res=Create_MailSlot(SlotName, hSlot, isserver);
if (res==FALSE)
return 0;

printf("Запущен процесс-");
printf((isserver) ? "СЕРВЕР\n" : "КЛИЕНТ\n");
printf("Список команд:\n[check] - получение информации о количестве сообщений\n");
printf((isserver) ? "[read] - чтение полученных сообщений\n" : "[write] - печать сообщений\n");
printf("[quit] - завершение работы программы\n");

string command;
while (1)
{
cout « ">";
cin » command;
if (command == "check")
{
Get_Messages(hSlot, cbMessage, cMessage);
}
else if (command == "quit")
{
CloseHandle(hSlot);
return 0;
}
else if ((command == "read") && (isserver))
{
Read_Message(hSlot);
}
else if ((command == "write") && (!isserver))
{
printf("Введите текст сообщения.Ввод продолжится, пока строка не пустая.\n");

string message, str;

getline(cin, str);
do
{
getline(cin, str);
message += str + '\n';
} while (!str.empty());

Write_Message(hSlot, (LPCTSTR)message.c_str());
}
else if ((command != "write") || (command != "check") || (command != "read") || (command == "quit"))
{
printf("Вы некорректно ввели команду.Пожалуйста, введите одну из команд, предложенных в списке:\n[check]-получение информации о количестве сообщений\n");
printf((isserver) ? "[read]-чтение полученных сообщений\n" : "[write]-печать сообщений\n");
printf("[quit]-завершение работы программы\n");
}
}
return 0;
}


