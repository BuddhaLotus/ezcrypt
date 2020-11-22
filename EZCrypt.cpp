//---------------------------------------------------------------------------

#include <vcl.h>

BOOL	g_bIsRunAgain=false;		//SDZ added 01181107.
//int MainEntry(int argc, char* argv[]);
#define		EZCryptMutexString		"__EZ_CRYPT_MUTEX_LOCK__"

#pragma hdrstop
//---------------------------------------------------------------------------
USEFORM("MainForm.cpp", FormMain);
USEFORM("AboutForm.cpp", AboutBox);
USEFORM("CreateForm.cpp", FormCreate);
USEFORM("MountForm.cpp", FormMount);
USEFORM("OptionForm.cpp", FormOption);
//---------------------------------------------------------------------------
WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	try
	{


		//SDZ added		200601101649>>
		CreateMutex(NULL,true,EZCryptMutexString);
		if(GetLastError()==ERROR_ALREADY_EXISTS)
		{
			ShowMessage("本程序已经在运行...");
			return -1;
        }
		//SDZ added		200601101649<<

	/*SDZ added	200601101743>>
	char*		cArgv[8];
	char*		CmdLine;
	int			iArgc;

	CmdLine=GetCommandLine();
	for(;;)
	{
		StrChr(CmdLine,' ')
    }
	MainEntry(iArgc,cArgv);
	//ShowMessage(CmdLineArray[1]);
	return 0;
	*/




		Application->Initialize();
		Application->CreateForm(__classid(TFormMain), &FormMain);
		Application->Run();
	}
	catch (Exception &exception)
	{
		Application->ShowException(&exception);
	}
	catch (...)
	{
		try
		{
			throw Exception("");
		}
		catch (Exception &exception)
		{
			Application->ShowException(&exception);
		}
	}


	//SDZ added 01181059>>
	
	if(g_bIsRunAgain)
    {
		AnsiString strPath;
		STARTUPINFO StartInfo;
		PROCESS_INFORMATION procStruct;
		memset(&StartInfo, 0, sizeof(STARTUPINFO));
		StartInfo.cb = sizeof(STARTUPINFO);
		strPath = Application->ExeName;
		//ShowMessage("True");
        if(!::CreateProcess(
                (LPCTSTR) strPath.c_str(),
                NULL,
                NULL,
                NULL,
                FALSE,
                NORMAL_PRIORITY_CLASS,
                NULL,
                NULL,
                &StartInfo,
                &procStruct))
            return 0;
	}

	//SDZ added 01181059<<
	


	return 0;
}
//---------------------------------------------------------------------------
