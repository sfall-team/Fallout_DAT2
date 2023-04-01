#pragma once

#include "DatFileBase.h"
#include "resource.h"

// Commands
enum Command {
	COMMAND_UNKNOWN,
	COMMAND_ADD,
	COMMAND_EXTRACT,
	COMMAND_DELETE,
	COMMAND_LIST,
	COMMAND_SHRINK
};

// Globals
extern Command g_Command;
extern int g_nCompressMethod;
extern BOOL g_bRecurseIntoDir;
extern BOOL g_bExtractWithoutPaths;
extern CString g_strOutFolder;
extern CString g_strDatFileName;
extern CStringArray g_FileList;
extern CDatFileBase* g_pDatFile;

// Functions
int OnAdd();
int OnExtract();
int OnDelete();
int OnList();
int OnShrink();
