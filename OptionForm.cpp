//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "OptionForm.h"

#include "stdio.h"
#include "global.h"
#include <Dialogs.hpp>
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TFormOption *FormOption;
TIniFile	*MainIniFile;

//---------------------------------------------------------------------------
__fastcall TFormOption::TFormOption(TComponent* Owner)
	: TForm(Owner)
{
}
//---------------------------------------------------------------------------
void __fastcall TFormOption::ButtonBrowseClick(TObject *Sender)
{
	TOpenDialog* OpenDialog=new TOpenDialog(this);
	if(OpenDialog->Execute())
	{

		if(OpenDialog->FileName.Length()>2)
		{
			if (Sender==ButtonBrowseAuto) {
				EditAutoMount->Text=OpenDialog->FileName;
			}
			else
			if (Sender==ButtonBrowse)
				EditBoxName->Text=OpenDialog->FileName;
		}
		/*
		TFormMount	*FormMountEx;
		FormMountEx=new TFormMount(Application);
		FormMountEx->ShowModal();

		if(FormMountEx->CancelExit)
		{
			delete FormMountEx;
			return;
		}
		*/

	}
}
//---------------------------------------------------------------------------
void __fastcall TFormOption::ButtonChpassClick(TObject *Sender)
{
	char		EncCombinStr[64];
	char*		CmdParaArray[6];
	char		BoxName[MAX_PATH];

	sprintf(BoxName,"%s",EditBoxName->Text );

	if((strlen(BoxName)<=0) || !IsFileExsist(BoxName,false))
	{
		ShowMessage("文件不存在，请重新指定文件名！");
		ButtonBrowse->SetFocus();
		return;
    }
	if(FileMounted(BoxName))
	{
		ShowMessage("文件已打开，请先将其关闭，然后再试！");
		return;
	}
	
	if((EditPass1->Text.Length()>7) && (EditPass2->Text.Length()>7) && (EditPass1->Text==EditPass2->Text))
	{
		sprintf(EncCombinStr,"/chpass:%s",EditOldPass->Text);

		CmdParaArray[0]="YourDisk";
		CmdParaArray[1]="/chpass";
		CmdParaArray[2]=BoxName;
		CmdParaArray[3]=EncCombinStr;
		CmdParaArray[4]=EditPass1->Text.c_str();

		int ret=MainEntry(5,CmdParaArray);
		if(!ret)
		{
			LogEvent("Change password OK.");
			Close();
		}
		else
		{
			LogEvent("Change password failed!");
			ShowMessage("修改密码失败，可能是当前密码输入错误，请重试！");
	    }
	}
	else
	{
        ShowMessage("两次输入的密码不一致或密码长度小于8，请更正！");
    }	
}
//---------------------------------------------------------------------------
void __fastcall TFormOption::ButtonCloseClick(TObject *Sender)
{
	Close();	
}
//---------------------------------------------------------------------------
void __fastcall TFormOption::FormCreate(TObject *Sender)
{
	CenterForm(this);
	MainIniFile = new TIniFile( ChangeFileExt( Application->ExeName, ".INI" ) );

	//AnsiString AutoMountFileName;
	if (MainIniFile==NULL) {
		//YCIniFile->Free();
		//if(!OpenIniFile())
			return;
	}
	//YCIniFile->UpdateFile();
	
	EditAutoMount->Text = MainIniFile->ReadString("Files","AutoMount","");
	if(EditAutoMount->Text.Length())
	{
		CheckBoxAutoMount->Checked=true;
		/*
		LabelAutoMount->Enabled=false;
		EditAutoMount->Enabled=false;
		ButtonBrowseAuto->Enabled=false;
		*/

	}
	else
	{
		CheckBoxAutoMount->Checked=false;
		LabelAutoMount->Enabled=false;
		EditAutoMount->Enabled=false;
		ButtonBrowseAuto->Enabled=false;

	}
}
//---------------------------------------------------------------------------

void __fastcall TFormOption::CheckBoxAutoMountClick(TObject *Sender)
{
	if (!CheckBoxAutoMount->Checked)
	{
		EditAutoMount->Enabled=false;
		ButtonBrowseAuto->Enabled=false;

	}
	else
	{
		EditAutoMount->Enabled=true;
		ButtonBrowseAuto->Enabled=true;

	}
}
//---------------------------------------------------------------------------

void __fastcall TFormOption::ButtonOKClick(TObject *Sender)
{
	if (CheckBoxAutoMount->Checked)
	{
		MainIniFile->WriteString("Files","AutoMount",EditAutoMount->Text);
	}
	else
    	MainIniFile->WriteString("Files","AutoMount","");
	Close();
}
//---------------------------------------------------------------------------

