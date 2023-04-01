// dat2.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "dat2.h"
#include "DatFile1.h"
#include "DatFile2.h"
#include "XGetopt.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif	/* _DEBUG */

// Globals
Command g_Command = COMMAND_UNKNOWN;
int g_nCompressMethod = 9;
BOOL g_bRecurseIntoDir = FALSE;
BOOL g_bExtractWithoutPaths = FALSE;
CString g_strOutFolder;
CString g_strDatFileName;
CStringArray g_FileList;
BOOL g_bFallout1Dat = FALSE;
CString g_strTargetDir = "";

// Dat-file
CDatFile1 g_DatFile1;
CDatFile2 g_DatFile2;
CDatFileBase* g_pDatFile = NULL;

// Functions
void PrintUsage(char* lpszFileName);
int ParseCommandLine(int argc, char* argv[]);

// The one and only application object
CWinApp theApp;

int main(int argc, char* argv[])
{
	if (!AfxWinInit(::GetModuleHandle(NULL), NULL, ::GetCommandLine(), 0)) {
		printf("Fatal Error: MFC initialization failed\n");
		return 1;
	}
	else {
		printf("Fallout DAT-files packer/unpacker, version 2.32\n");
		printf("Copyright (C) Anchorite (TeamX), 2004-2006\n");
		printf("anchorite2001@yandex.ru\n");
		printf("\n");

//		for(int i = 0; i < argc; i++) {
//			printf("argv[%d] = %s\n", i, argv[i]);
//		}

		if (argc < 3) {
			PrintUsage(argv[0]);
			return 0;
		}

		int nResult = ParseCommandLine(argc, argv);

		switch(nResult) {
			case -1: {
				printf("Error: Invalid command\n");
				return 1;
			}

			case -2: {
				printf("Error: Name of output directory is omitted\n");
				return 1;
			}

			case -3: {
				printf("Error: Name of dat-file is omitted\n");
				return 1;
			}

			case -4: {
				printf("Error: List of files is omitted\n");
				return 1;
			}

			case -5: {
				printf("Error: Unable open response file\n");
				return 1;
			}
		}

		nResult = 0;
		UINT nOpenMode;

		switch(g_Command) {
			case COMMAND_EXTRACT:
				nOpenMode = CFile::modeRead | CFile::shareDenyWrite;
				break;

			case COMMAND_DELETE:
				nOpenMode = CFile::modeReadWrite | CFile::shareDenyWrite;
				break;

			case COMMAND_LIST:
				nOpenMode = CFile::modeRead | CFile::shareDenyWrite;
				break;

			case COMMAND_SHRINK:
				nOpenMode = CFile::modeReadWrite | CFile::shareDenyWrite;
		}

		if (g_Command == COMMAND_ADD) {
			if (g_bFallout1Dat) {
				g_pDatFile = &g_DatFile1;
			}
			else {
				g_pDatFile = &g_DatFile2;
			}
		}
		else {
			if (g_DatFile2.Open(g_strDatFileName, nOpenMode)) {
				g_pDatFile = &g_DatFile2;
			}
			else if (g_DatFile1.Open(g_strDatFileName, nOpenMode)) {
				g_pDatFile = &g_DatFile1;
			}
			else {
				printf("Error: Unable open file %s\n", LPCTSTR(g_strDatFileName));
				return 1;
			}
		}

		switch(g_Command) {
			case COMMAND_ADD:
				nResult = OnAdd();
				break;

			case COMMAND_EXTRACT:
				nResult = OnExtract();
				break;

			case COMMAND_DELETE:
				nResult = OnDelete();
				break;

			case COMMAND_LIST:
				nResult = OnList();
				break;

			case COMMAND_SHRINK:
				nResult = OnShrink();
		}

		printf("\n");
		printf("Flushing buffers...\n");
		g_pDatFile->Flush();

		return nResult;
	}
}

void PrintUsage(char* lpszFileName)
{
	printf("Usage: %s <command> [options] [-t dir] [-d dir] dat-file [list | @response-file]\n", lpszFileName);
	printf("\n");
	printf("Commands\n");
	printf("  a: Add files to dat-file. Create new if dat-file not exist\n");
	printf("  x: Extract files from dat-file\n");
	printf("  d: Delete files from dat-file (only info about files)\n");
	printf("  l: List files in dat-file\n");
	printf("  k: Shrink dat-file\n");
	printf("\n");
	printf("Options\n");
	printf("  -s: create Fallout 1 dat-file\n");
	printf("  -r: recurse into directories\n");
	printf("  -0..9: Compression method\n");
	printf("         (Fallout1: 0 - store, other numbers - compress (default)\n");
	printf("         (Fallout2: 0 - store, 1 - best speed, 9 - best compression (default)\n");
	printf("  -p: extract without paths\n");
	printf("  -d: extract files into specified directory\n");
	printf("  -t: add files to specified directory of dat-file\n");
	printf("  --: end of options\n");

	printf("\n");
}

int ParseCommandLine(int argc, char* argv[])
{
	// Command 
	if (memcmp(argv[1], "a", lstrlen(argv[1])) == 0) {
		g_Command = COMMAND_ADD;
	}
	else if (memcmp(argv[1], "x", lstrlen(argv[1])) == 0) {
		g_Command = COMMAND_EXTRACT;
	}
	else if (memcmp(argv[1], "d", lstrlen(argv[1])) == 0) {
		g_Command = COMMAND_DELETE;
	}
	else if (memcmp(argv[1], "l", lstrlen(argv[1])) == 0) {
		g_Command = COMMAND_LIST;
	}
	else if (memcmp(argv[1], "k", lstrlen(argv[1])) == 0) {
		g_Command = COMMAND_SHRINK;
	}
	else {
		return -1;
	}

	// Options
	argc--;
	argv++;

	int c;

	while((c = getopt(argc, argv, "r0123456789pd:st:")) != EOF) {
		switch(c) {
			case 'r':
				g_bRecurseIntoDir = TRUE;
				break;

			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				g_nCompressMethod = c - '0';
				break;

			case 'p':
				g_bExtractWithoutPaths = TRUE;
				break;

			case 'd':
				g_strOutFolder = optarg;

				if ((!g_strOutFolder.IsEmpty()) && (g_strOutFolder.Right(1) != "\\")) {
					g_strOutFolder += "\\";
				}

				break;

			case 's':
				g_bFallout1Dat = TRUE;
				break;

			case 't':
				g_strTargetDir = optarg;

				if ((!g_strTargetDir.IsEmpty()) && (g_strTargetDir.Right(1) != "\\")) {
					g_strTargetDir += "\\";
				}

				break;

			case '?':
				return -2;

			default:	// No more options
				break;
		}
	}

	// Dat-file
	if (optind < argc) {
		g_strDatFileName = argv[optind];
	}

	if (g_strDatFileName.IsEmpty()) {
		return -3;
	}

	if ((g_strDatFileName.Right(4).MakeLower() != ".dat") || (g_strDatFileName == ".dat")) {
		g_strDatFileName += ".dat";
	}

	// List of files
	CString strArgument;

	for(int i = optind + 1; i < argc; i++) {
		strArgument = argv[i];

		int nArglength = strArgument.GetLength();

		if ((i == optind + 1) && (nArglength > 1) && (strArgument[0] == '@')) {
			CStdioFile fileResponse;
				
			if (!fileResponse.Open(strArgument.Right(nArglength - 1), CFile::modeRead | CFile::shareDenyWrite | CFile::typeText)) {
				return -5;
			};

			CArchive arResponse(&fileResponse, CArchive::load);

			while(arResponse.ReadString(strArgument)) {
				if (!strArgument.IsEmpty()) {
					g_FileList.Add(strArgument);
				}
			}

			break;
		}
		else {
			g_FileList.Add(strArgument);
		}
	}

	if ((g_FileList.GetSize() == 0)) {
		if (g_Command == COMMAND_EXTRACT) {
			g_FileList.Add("*");
		}
		else if ((g_Command != COMMAND_LIST) && (g_Command != COMMAND_SHRINK)) {
			return -4;
		}
	}

//	for(int i = 0; i < g_FileList.GetSize(); i++) {
//		printf("g_FileList[%d] = %s\n", i, g_FileList[i]);
//	}

	return 0;
}