//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "MainForm.h"

#include "CreateForm.h"
#include "AboutForm.h"
#include "MountForm.h"
#include "OptionForm.h"

#include "Global.h"


//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"

TFormMain *FormMain;

LPWSTR*		CmdArgv;
int			CmdArgc;
//char		BoxFileName[512];
char		BoxEncType[8];
int			BoxSize;
char		BoxSizeStr[16];
char		EncPasswd[32];
char		CreateCmd[512];
char		MountCmd[512];
char		UmountCmd[512];
int			CmdParaCount;
char*		CmdParaArray[10];
char		EncCombinStr[64];
AnsiString	TempAnsiStr;
BOOLEAN		ForceClose;

extern 	bool 	g_bIsRunAgain;	//SDZ added 01181101

#define		ExitHotKey		MOD_CONTROL|MOD_ALT,VK_F12
#define		OpenBoxHotKey	MOD_CONTROL|MOD_ALT,VK_F1
#define		CloseBoxHotKey	MOD_CONTROL|MOD_ALT,VK_F11
#define		ShowHideHotKey	MOD_CONTROL|MOD_ALT,VK_F10
//|MOD_SHIFT

//---------------------------------------------------------------------------
__fastcall TFormMain::TFormMain(TComponent* Owner)
	: TForm(Owner)
{
}
//---------------------------------------------------------------------------
void
__fastcall TFormMain::ReportError(
//TStatusBar*		StatusBar,
char*			ErrorString,
int				ReportLevel
)
{
	if(ReportLevel<=LogDetailLevel)
		LogEvent(ErrorString);
	StatusBar->Panels->Items[0]->Text=ErrorString;
}

//---------------------------------------------------------------------------
void __fastcall TFormMain::BitBtnExitClick(TObject *Sender)
{
	ForceClose=false;
	Close();
}
//---------------------------------------------------------------------------
void __fastcall TFormMain::BitBtnModeClick(TObject *Sender)
{
#ifndef	   LiteVersion
	if(ExpertMode)
	{
		//StatusBar->Enabled=false;
		//StatusBar->Visible=false;
		StringGridDrvInfo->Enabled=false;
		StringGridDrvInfo->Visible=false;
		this->ClientHeight=BitBtnMode->Height+StatusBar->Height;
		BitBtnMode->Caption="高级";
		ExpertMode=false;
	}
	else
	{
		//StatusBar->Enabled=true;
		//StatusBar->Visible=true;
		StringGridDrvInfo->Enabled=true;
		StringGridDrvInfo->Visible=true;
		this->ClientHeight=BitBtnMode->Height+StringGridDrvInfo->Height+
			StatusBar->Height;
		BitBtnMode->Caption="简易";
		ExpertMode=true;
		ListAllDrivers(StringGridDrvInfo);
	}
#else
		StringGridDrvInfo->Enabled=false;
		StringGridDrvInfo->Visible=false;
		this->ClientHeight=BitBtnMode->Height+StatusBar->Height;

#endif//#ifndef	   LiteVersion
}
//---------------------------------------------------------------------------


void __fastcall TFormMain::mnuAboutClick(TObject *Sender)
{
	AboutBox= new TAboutBox(Application);
	AboutBox->ShowModal();
	delete AboutBox;
}
//---------------------------------------------------------------------------

void __fastcall TFormMain::FormCreate(TObject *Sender)
{
	ExpertMode=true;
	BitBtnModeClick(NULL);
	
#ifdef		LiteVersion
	BitBtnMode->Visible=false;
	BitBtnMode->Enabled=false;
	BitBtnExit->Left=BitBtnMode->Left;
	BitBtnOption->Visible=false;
	BitBtnOption->Enabled=false;
	TBitBtn *bbAbout=new TBitBtn(this);
	bbAbout->Parent=this;
	bbAbout->Top=BitBtnExit->Top;
	bbAbout->Left=BitBtnExit->Left+BitBtnExit->Width;
	bbAbout->Width=BitBtnExit->Width;
	bbAbout->Height=BitBtnExit->Height;
	bbAbout->Caption="关于";		//About";
	bbAbout->OnClick=mnuAboutClick;
	//bbAbout->Visible=true;
	this->Width-=BitBtnMode->Width;
#endif

	ForceClose=false;

	ListAllDrivers(StringGridDrvInfo);
	//@ListFreeDriverLetter(ComboBoxDriver);

//SDZ added 01130916
#ifdef			IsMultiLang
	//SDZ added 0708020823>>
	if(!OpenIniFile())
	{
		return;
	}
	//<<

	AnsiString DDSTR;
	DDSTR=Application->GetNamePath()+YCIniFile->ReadString("Global",
		"LangFile","");
	LangFile = new TIniFile("./" + YCIniFile->ReadString("Global",
		"LangFile",""));
	if(!LangFile)
		delete LangFile;

	this->Caption=LangFile->ReadString("Title","MainForm","EZCrypt");
	BitBtnCreate->Caption=LangFile->ReadString("Title","NewButton","Create");
	BitBtnMount->Caption=LangFile->ReadString("Title","OpenButton","Open");
	BitBtnClose->Caption=LangFile->ReadString("Title","CloseButton","Close");
	
	if(ExpertMode)
		BitBtnMode->Caption=LangFile->ReadString("Title","ExpertButton","Easy");
	else
		BitBtnMode->Caption=LangFile->ReadString("Title","EasyButton","Expert");

	BitBtnOption->Caption=LangFile->ReadString("Title","OptionButton","Option");
	BitBtnExit->Caption=LangFile->ReadString("Title","HideButton","Hide");
#endif
	//SDZ added 01130916

/*
#ifdef	InDebuging
	//@EditPass1->Text="dracon523";
	//@EditPass2->Text="dracon523";
	//@EditFileName->Text="c:\\yourcrypt.dsk";//"\\Device\\Harddisk0\\Partition2";
	//@SpinEditSize->Value=32;
#else
#endif
*/

//SDZ>注册系统热键
	if(!RegisterHotKey(Handle,2000,ExitHotKey))
//这里注册的热键是Ctrl+F12
		ShowMessage("注册退出快捷键C+A+F12失败！");
	if(!RegisterHotKey(Handle,2001,CloseBoxHotKey))
		ShowMessage("注册关闭所有文件快捷键C+A+F11失败！");
	if(!RegisterHotKey(Handle,2002,ShowHideHotKey))
		ShowMessage("注册显示/隐藏快捷键C+A+F10失败！");

	if(!RegisterHotKey(Handle,2003,OpenBoxHotKey))
		ShowMessage("注册打开文件快捷键C+A+F1失败！");

//SDZ<
/*
#ifdef	  AlwaysShowTrayIcon
	TrayIcon->Visible=true;
#endif
*/

	//if(g_bIsRunAgain)this->Caption="g_bIsRunAgain is true.";
	if(!DriverControl(0x02))
	{
		//ReportError();
		DriverControl(0x08);
		DriverControl(0x01);
		if(!DriverControl(0x02))
		{
			ShowMessage("驱动程序出错，请重新启动或安装此程序！");
			g_bIsRunAgain=true;		//SDZ added 01181103
			//ForceClose=true;
			//this->Close();				//SDZ modified 01181103
			Application->Terminate();
		}
	}

	//SDZ added 01152110>>
	if(YCIniFile)
	{
		//memset(LogFileName,0,sizeof(LogFileName));
		strcpy(LogFileName,(YCIniFile->ReadString("Log","LogFile",
			ChangeFileExt( Application->ExeName, ".INI" ))).c_str());
		LogDetailLevel=YCIniFile->ReadInteger("Log","LogLevel",0);

#ifdef		LiteVersion
		if(GetMountedNum()<1)
		{
#endif
			strcpy(LastMountedFile ,
				YCIniFile->ReadString( "Files", "AutoMount","" ).c_str() );

			if(!IsFileExsist(LastMountedFile,false)||(strlen(LastMountedFile)==0))
			{
				ReportError("The file not exsist!",2);
				//delete FormMount;
				return;
			}

			//TFormMount	*FormMount;
			FormMount=new TFormMount(Application);
			FormMount->ShowModal();

			if(FormMount->CancelExit)
			{
				delete FormMount;
				return;
			}

		//memset(BoxFileName,0,sizeof(BoxFileName));
		sprintf(BoxFileName,"%s",LastMountedFile);


		//SDZ+ 200601072013	Support disk encrypt.>>
		if(strstr(StrLower(BoxFileName),"\\device\\")==BoxFileName)
		{
			if(!DefineDosDevice (DDD_RAW_TARGET_PATH, "tempdiskdriver",BoxFileName))
			{
				delete FormMount;
				return;
			}
			else
			{
				memset(BoxFileName,0,sizeof(BoxFileName));
				sprintf(BoxFileName,"\\\\.\\tempdiskdriver");
			}
		}
		//SDZ++<<

		if(FileMounted(BoxFileName))
		{
			ReportError("BoxFile already mounted!",2);
			delete FormMount;
			return;
		}

		//memset(EncPasswd,0,sizeof(EncPasswd));
		sprintf(EncPasswd,"%s",FormMount->EditPassword->Text);

		//memset(EncCombinStr,0,sizeof(EncCombinStr));
		sprintf(EncCombinStr,"/%s:%s","head",EncPasswd);

		CmdParaCount=5;
		/*
		if(CheckBoxCD->Checked)		//SDZ added 01102042	Mount as cdrom supported.
		{
			CmdParaCount=6;
			CmdParaArray[5]="/cd";
		}
		else
		*/
		if(FormMount->CheckBoxRO->Checked)
		{
			CmdParaCount=6;
			CmdParaArray[5]="/ro";
		}

		int ret;

		CmdParaArray[0]="YourDisk";
		CmdParaArray[4]=FormMount->ComboBoxDrivers->Text.c_str();	//SDZ added 01151930
		CmdParaArray[2]=BoxFileName;
		CmdParaArray[3]=EncCombinStr;

		CmdParaArray[1]="/mount";

		ret=MainEntry(CmdParaCount,CmdParaArray);
		if(!ret)
		{
			ReportError("Mount BoxFile OK.",3);

		}
		else
		if(ret==-5)
			ReportError("Password or alg error!",2);
		else
			ReportError("Mount BoxFile failed!",2);

		//@ListAllDrivers(ListBoxDrivers);
		//@ListFreeDriverLetter(ComboBoxDriver);\

		if(FormMount)	//sdz 070823
			delete FormMount;

#ifdef		LiteVersion
	}
#endif



		//if(EditFileName->Text.Length())
		//	ButtonMountClick(NULL);

		//FormMount= new TFormMount(Application);
		//FormMount->ShowModal();
		//delete FormMount;

		strcpy(LastMountedFile ,YCIniFile->ReadString( "Files", "LastMounted","" ).c_str());
		if(HideMainForm=YCIniFile->ReadBool("Global","HideOnStart",false))
		{
			//Hide();//not work properly.
			//Application->ShowMainForm=false;
			//sdz 070823>>
			//Application->MainForm->Visible=false;
			Visible=false;
			//<<
		}

		if(YCIniFile->ReadBool("Global","AlwaysTrayIcon",false))
		{
			TrayIcon->Visible=true;
		}

		UmountAllExit=YCIniFile->ReadBool("Files","UmountAllExit",false);
		DelDriverExit=YCIniFile->ReadBool("Global","DelDriverExit",false);
		
		DeepQuiet=YCIniFile->ReadBool("Global","DeepQuiet",false);
		AlwaysTrayIcon=YCIniFile->ReadBool("Global","AlwaysTrayIcon",false);
		NoExitConfirm=YCIniFile->ReadBool("Global","NoExitConfirm",false);
		
	}
	//SDZ added 01152110<<

	CenterForm(this);
}
//---------------------------------------------------------------------------

void __fastcall TFormMain::BitBtnCreateClick(TObject *Sender)
{
	if(OpenDialog->Execute())
	{
		if(IsFileExsist(OpenDialog->FileName.c_str(),FALSE))
		{
			ReportError("File not exsist!Retry!",2);
			return;
		}
		//TFormCreate *FormNewBox;
		FormNewBox= new TFormNewBox(Application);

		//strcpy(BoxFileName,OpenDialog->FileName.c_str());
		FormNewBox->EditFileName->Text=OpenDialog->FileName;//BoxFileName;
		FormNewBox->ShowModal();

		if(FormNewBox->CancelExit)
		{
			ReportError("创建取消...",3);
			delete FormNewBox;
			return;
		}
		//memset(BoxEncType,0,sizeof(BoxEncType));

		if(FormNewBox->LocalExpertMode)
		{
			switch(FormNewBox->RadioGroupAlgo->ItemIndex)
			{
				case 1:
					strcpy(BoxEncType,"2f\0");
					break;
				case 2:
					strcpy(BoxEncType,"aes128\0");
					break;
				case 3:
					strcpy(BoxEncType,"aes192\0");
					break;
				case 4:
					strcpy(BoxEncType,"aes256\0");
					break;
				default:
					strcpy(BoxEncType,"none\0");
            }
		}
		else
			strcpy(BoxEncType,"aes128\0");
			
		strcpy(BoxFileName,FormNewBox->EditFileName->Text.c_str());//,strlen(EditFileName->Text.c_str())+1);

	//if(FormNewBox->EditPassword1->Text ==FormNewBox->EditPassword2->Text)
	{
		strcpy(EncPasswd,FormNewBox->EditPassword1->Text.c_str());//,strlen(EditPass1->Text.c_str())+1);
	}
	/*
	else
	{
		ShowMessage("两次输入的密码不一致，请重试！");
		delete FormNewBox;
        return;
    }
    */
	//memset(BoxSizeStr,0,sizeof(BoxSizeStr));
	sprintf(BoxSizeStr,"%dM",FormNewBox->CSpinEditSize->Value);

	//SDZ added 01151813>>
	/*
	if(BoxFileName[1]==':')
	{
		char*	DriveStr="C:";
		memcpy(DriveStr,BoxFileName,2);

		ULARGE_INTEGER	FreeSpaceToCall,TotalSpace,FreeSpace;
		if(::GetDiskFreeSpaceEx(DriveStr,&FreeSpaceToCall,&TotalSpace,&FreeSpace))
		{
			if((FreeSpace.QuadPart/1024/1024)<(FormNewBox->CSpinEditSize->Value))
			{
				ShowMessage("指定的文件尺寸太大，空间不足！");
				FormNewBox->CSpinEditSize->Value=FreeSpace.QuadPart/1024/1024-1;
				delete FormNewBox;
				return;
			}
		}
		else
		{
			ShowMessage("检查可用剩余空间出错，请重试！");
			delete FormNewBox;
			return;
		}
	}
	*/
	//SDZ added 01151813<<

	//SDZ added 01172112>>
	/*
	if(BoxFileName[1]==':')
	{
		char*	ErrMsg="The partition type: 000000";
		sprintf(ErrMsg,"The partition type: %d",GetPartitionType(BoxFileName[0]));
		this->Caption=ErrMsg;
		//return;
	}
	*/
	//SDZ added 01172112<<


	//SDZ added 01170853>>
	/*
	if(IsFileExsist(BoxFileName,false))
	{
		ReportError("The file already exsist!",2);
		delete FormNewBox;
		return;
	}
	*/
	/*
	if(FileMounted(BoxFileName))
	{
		ReportError("BoxFile already mounted!",2);
		return;
	}
	*/
	//SDZ added 01170853

	//SDZ added 01171202>>
	/*
	if(CreateSparseFile(BoxFileName,BoxSizeStr)==-3)
	{
		ShowMessage("Create file failed!");
		//return;
	}
	*/
	//SDZ added 01171202<<

	//memset(EncCombinStr,0,sizeof(EncCombinStr));
	sprintf(EncCombinStr,"/%s:%s",BoxEncType,EncPasswd);

	//SDZ added 01170914>>

	//SDZ added 01170914<<

	CmdParaCount=5;
	CmdParaArray[0]="YourDisk";
	CmdParaArray[1]="/mount";
	//CmdParaArray[4]=ComboBoxDriver->Text.c_str();	//SDZ added 01151930
	CmdParaArray[2]=BoxFileName;
	CmdParaArray[3]=BoxSizeStr;
	CmdParaArray[4]=EncCombinStr;

	int ret=MainEntry(CmdParaCount,CmdParaArray);
	if(!ret)
		ReportError("Create Box file OK.",3);
	else
		ReportError("Create BoxFile failed!",2);

	ListAllDrivers(StringGridDrvInfo);
	//@ListFreeDriverLetter(ComboBoxDriver);

		delete FormNewBox;
	}
	
}
//---------------------------------------------------------------------------

void __fastcall TFormMain::BitBtnMountClick(TObject *Sender)
{

#ifdef	LiteVersion
	if(GetMountedNum())
	{
		ReportError("Free version can mount one boxfile only.",2);
		return;
	}
#endif

	if(OpenDialog->Execute())
	{
		//SDZ+ 200601072013	Support disk encrypt.>>
		//sprintf(BoxFileName,"\\device\\harddisk0\\partition4");
		if(strstr(StrLower(BoxFileName),"\\device\\")==BoxFileName)
		{
			if(!DefineDosDevice (DDD_RAW_TARGET_PATH, "tempdiskdriver",BoxFileName))
			{
				delete FormMount;
				return;
			}
			else
			{
				memset(BoxFileName,0,sizeof(BoxFileName));
				sprintf(BoxFileName,"\\\\.\\tempdiskdriver");
			}
		}
		//SDZ++<<


		if(!IsFileExsist(OpenDialog->FileName.c_str(),FALSE))
		{
			ReportError("文件不存在，请重新选取...",2);
			return;
        }
		//TFormMount	*FormMount;
		FormMount=new TFormMount(Application);
		FormMount->ShowModal();

		if(FormMount->CancelExit)
		{
            ReportError("打开被取消",3);
			delete FormMount;
			return;
		}

	strcpy(BoxEncType,"head\0");

	//memset(BoxFileName,0,sizeof(BoxFileName));
	sprintf(BoxFileName,"%s",OpenDialog->FileName);

	if(!IsFileExsist(BoxFileName,false))
	{
		ReportError("The file not exsist!",2);
		delete FormMount;
		return;
	}
	
	if(FileMounted(BoxFileName))
	{
		ReportError("BoxFile already mounted!",2);
		delete FormMount;
		return;
	}

	//memset(EncPasswd,0,sizeof(EncPasswd));
	//sprintf(EncPasswd,"%s",FormMount->EditPassword->Text);

	//memset(EncCombinStr,0,sizeof(EncCombinStr));
	sprintf(EncCombinStr,"/%s:%s",BoxEncType,FormMount->EditPassword->Text);

	CmdParaCount=5;
	/*
	if(CheckBoxCD->Checked)		//SDZ added 01102042	Mount as cdrom supported.
	{
		CmdParaCount=6;
		CmdParaArray[5]="/cd";
	}
	else
	*/
	if(FormMount->CheckBoxRO->Checked)
	{
		CmdParaCount=6;
		CmdParaArray[5]="/ro";
	}

	int ret;

	CmdParaArray[0]="YourDisk";
	CmdParaArray[4]=FormMount->ComboBoxDrivers->Text.c_str();	//SDZ added 01151930
	CmdParaArray[2]=BoxFileName;
	CmdParaArray[3]=EncCombinStr;

	/*
	if(!(CheckBoxCD->Checked))	//SDZ added 01102042
	{
		//SDZ added 200601091510	Test if the pass and alg valid>>
		CmdParaArray[1]="/test";

		ret=MainEntry(CmdParaCount,CmdParaArray);

		if(ret!=0)
		{
			ReportError("Password or alg error!",2);
			LogEvent("Password or alg error!");

			if(!CheckBoxForceMount->Checked )
				return;
			else
				if(MessageDlg((AnsiString)"FORCE MOUNT without valid password and alg?",mtConfirmation,mbYesNo,0)==mrNo)
				{
					return;
				}
		}
		//SDZ added 200601091510<<
	}
	*/
	
	CmdParaArray[1]="/mount";

	ret=MainEntry(CmdParaCount,CmdParaArray);
	if(!ret)
	{
		ReportError("Mount BoxFile OK.",3);

		//SDZ added 200708011555>>
		strcpy(LastMountedFile,BoxFileName);
		YCIniFile->WriteString("Files","LastMounted",LastMountedFile);
		//<<
	}
	else
	if(ret==-5)
		ReportError("Password or alg error!",2);
	else
		ReportError("Mount BoxFile failed!",2);
		
	ListAllDrivers(StringGridDrvInfo);
	//@ListFreeDriverLetter(ComboBoxDriver);


		delete FormMount;
	}
}
//---------------------------------------------------------------------------

void __fastcall TFormMain::mnuShowMainFormClick(TObject *Sender)
{
	this->Show();
}
//---------------------------------------------------------------------------


void __fastcall TFormMain::mnuExitClick(TObject *Sender)
{
	ForceClose=true;
	Close();	
}
//---------------------------------------------------------------------------

void __fastcall TFormMain::mnuCloseAllClick(TObject *Sender)
{
	BitBtnCloseClick(NULL);
}
//---------------------------------------------------------------------------

void __fastcall TFormMain::FormClose(TObject *Sender, TCloseAction &Action)
{
	if(!ForceClose)
	{
		Action=caNone;
		Application->Minimize();
		this->Hide();

	}
	else
	{
		if(!DeepQuiet || !NoExitConfirm)
		{
			if(MessageDlg((AnsiString)"您真的想关闭此程序吗？",mtConfirmation,mbYesNo,0)==mrNo)
			{
				Action=caNone;
				ForceClose=false;
				return;
			}
		}
//SDZ added 01130916>>
		if(YCIniFile)
		{
			YCIniFile->WriteString("Files", "LastMounted",LastMountedFile);
			delete YCIniFile;
		}
//SDZ added 01130916<<

		UnregisterHotKey(Handle, 2000);//退出程序时注销热键
		UnregisterHotKey(Handle, 2001);
		UnregisterHotKey(Handle, 2002);
		UnregisterHotKey(Handle, 2003);

		if(UmountAllExit)
		{
			BitBtnCloseClick(NULL);
		}
		
		if(UmountAllExit && DelDriverExit)
		{
			if(GetOsVersion()>0x0500)			//SDZ added 01121520
				DriverControl(0x08);			//Modified SDZ 01111653
				//DriverControl(0x08);			//Make sure driver removed.
        }
		//g_bIsRunAgain=false;
	}	
}
//---------------------------------------------------------------------------

void __fastcall TFormMain::TrayIconDblClick(TObject *Sender)
{
	this->Show();	
}
//---------------------------------------------------------------------------

void __fastcall TFormMain::BitBtnCloseClick(TObject *Sender)
{
	CmdParaCount=2;
	CmdParaArray[0]="YourDisk";
	CmdParaArray[1]="/umount";

	int ret=MainEntry(CmdParaCount,CmdParaArray);
	if(!ret)
		ReportError("Unmount BoxFile OK.",3);
	else
		ReportError("Unmount BoxFile failed!",2);

	ListAllDrivers(StringGridDrvInfo);
	//@ListFreeDriverLetter(ComboBoxDriver);
}
//---------------------------------------------------------------------------

void __fastcall TFormMain::OnHotKey(TMessage &Message)
{

	TForm::Dispatch(&Message);

	if(Message.LParamHi==VK_F12 && Message.LParamLo==MOD_CONTROL|MOD_ALT|MOD_SHIFT)
	{
		//ButtonUmountClick(NULL);
		ForceClose=true;
		//this->Show();
		//this->BringToFront();
		Close();
		//this->BringToFront();
	}
	else
	if(Message.LParamHi==VK_F11 && Message.LParamLo==MOD_CONTROL|MOD_ALT|MOD_SHIFT)
	{
		BitBtnCloseClick(NULL);
		//ForceClose=true;
		//Close();
	}
	else
	if(Message.LParamHi==VK_F1 && Message.LParamLo==MOD_CONTROL|MOD_ALT|MOD_SHIFT)
	{
		BitBtnMountClick(NULL);
		//ForceClose=true;
		//Close();
	}
	else	
	if(Message.LParamHi==VK_F10 && Message.LParamLo==MOD_CONTROL|MOD_ALT|MOD_SHIFT)
	{
		if (this->Visible)
		{
			Application->Minimize();
			this->Hide();
		}
		else
		{
			this->Show();
			Application->Restore();
			this->BringToFront();
		}

	}
}

/*
void __fastcall TFormMain::mnuCloseitClick(TObject *Sender)
{
	if(StringGridDrvInfo->RowCount <=0)	return;
	
	if(StringGridDrvInfo-> ==-1)
		return;
	else
	{
		AnsiString	DriverStr;
		AnsiString	LowerStr;
		char        UmountCmd[32];

		strcpy(UmountCmd,StringGridDrvInfo->Items[0][StringGridDrvInfo->ItemIndex].c_str());
		//DriverStr=ListBoxDrivers->Items->ValueFromIndex[ListBoxDrivers->ItemIndex];
		StrLower(UmountCmd);

		//DriverStr->LowerCase;
		if(UmountCmd[0]>='a' && UmountCmd[0]<='z')
		{
			//UmountCmd[0]=DriverStr[0];
			UmountCmd[2]='\0';
			CmdParaCount=3;
			CmdParaArray[0]="YourDisk";
			CmdParaArray[1]="/umount";
			CmdParaArray[2]=UmountCmd;
			int ret=MainEntry(CmdParaCount,CmdParaArray);
			if(!ret)
				ReportError("Unmount BoxFile OK.",3);
			else
				ReportError("Unmount BoxFile failed!",2);
		}
	}

	ListAllDrivers(StringGridDrvInfo);
	//@ListFreeDriverLetter(ComboBoxDriver);	
}
*/

//---------------------------------------------------------------------------

void __fastcall TFormMain::mnuCloseAllVolClick(TObject *Sender)
{
	if(StringGridDrvInfo->Cells[0][StringGridDrvInfo->Row].Length()!=2)
		return;
	BitBtnCloseClick(NULL);	
}
//---------------------------------------------------------------------------

void __fastcall TFormMain::FormShow(TObject *Sender)
{
	if(!AlwaysTrayIcon)
		TrayIcon->Visible=false;

	//this->WindowState=wsMaximized;
}
//---------------------------------------------------------------------------

void __fastcall TFormMain::FormHide(TObject *Sender)
{
	if(!DeepQuiet)
		TrayIcon->Visible=true;

	//this->WindowState=wsMinimized;
}
//---------------------------------------------------------------------------



void __fastcall TFormMain::BitBtnOptionClick(TObject *Sender)
{
#ifndef		LiteVersion
	//TFormOption	*FormOptionEx;
	FormOption=new TFormOption(Application);
	FormOption->ShowModal();

	delete FormOption;
#endif
}
//---------------------------------------------------------------------------

void __fastcall TFormMain::mnuCloseitClick(TObject *Sender)
{
	CmdParaCount=3;
	CmdParaArray[0]="YourDisk";
	CmdParaArray[1]="/umount";
	if(StringGridDrvInfo->Cells[0][StringGridDrvInfo->Row].Length()!=2)
		return;

	CmdParaArray[2]=(char *)StringGridDrvInfo->Cells[0][StringGridDrvInfo->Row].data();

	int ret=MainEntry(CmdParaCount,CmdParaArray);
	if(!ret)
		ReportError("Unmount BoxFile OK.",3);
	else
		ReportError("Unmount BoxFile failed!",2);

	ListAllDrivers(StringGridDrvInfo);

}
//---------------------------------------------------------------------------

void __fastcall TFormMain::StringGridDrvInfoContextPopup(TObject *Sender,
      TPoint &MousePos, bool &Handled)
{
	if(StringGridDrvInfo->Cells[0][0].Length()!=2)
	{
		Handled=true;
		return;
	}
}
//---------------------------------------------------------------------------

