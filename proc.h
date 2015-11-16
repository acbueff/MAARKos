// proc.h, 159

#ifndef _PROC_H_
#define _PROC_H_

//void Loader();
void InitProc();
void UserProc();
void PrintDriver();
void UserShell();
void TerminalIn();
void TerminalOut();
void DirStr(attr_t *p, char *str);
void Dir(char *cmd, int TerminalOutPid, int FileSystemPid);
void Cat(char *cmd, int TerminalOutPid, int FileSystemPid);
#endif
