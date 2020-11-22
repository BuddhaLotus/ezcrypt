//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "CreateForm.h"

#include "Global.h"
#include <Dialogs.hpp>
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma link "cspin"
#pragma resource "*.dfm"
TFormNewBox *FormNewBox;


//---------------------------------------------------------------------------
__fastcall TFormNewBox::TFormNewBox(TComponent* Owner)
	: TForm(Owner)
{
	LocalExpertMode=true;
	CancelExit=true;
	ButtonExpertClick(NULL);
#ifdef		_LITE_VERSION_
	ButtonExpert->Visible=false;
	ButtonExpert->Enabled=false;
#endif
}
//---------------------------------------------------------------------------
void __fastcall TFormNewBox::ButtonExpertClick(TObject *Sender)
{
	if(LocalExpertMode)
	{
		this->Height=this->Height-RadioGroupAlgo->Height;
		RadioGroupAlgo->Enabled=false;

		ButtonCreate->Top=RadioGroupAlgo->Top;
		ButtonExpert->Top=ButtonCreate->Top;

		RadioGroupAlgo->Top=ButtonCancel->Top;

		ButtonCancel->Top=ButtonCreate->Top;
		RadioGroupAlgo->Visible=false;

	}
	else
	{
		this->Height+=RadioGroupAlgo->Height;

		ButtonCreate->Top=RadioGroupAlgo->Top;
		ButtonExpert->Top=ButtonCreate->Top;

		RadioGroupAlgo->Top=ButtonCancel->Top;

		ButtonCancel->Top=ButtonCreate->Top;
		RadioGroupAlgo->Enabled=true;
		RadioGroupAlgo->Visible=true;
	}

	LocalExpertMode=!LocalExpertMode;
}
//---------------------------------------------------------------------------

void __fastcall TFormNewBox::ButtonCancelClick(TObject *Sender)
{
	CancelExit=true;
	Close();
}
//---------------------------------------------------------------------------


void __fastcall TFormNewBox::FormShow(TObject *Sender)
{
	EditPassword1->SetFocus();	
}
//---------------------------------------------------------------------------

void __fastcall TFormNewBox::ButtonCreateClick(TObject *Sender)
{
	if(EditFileName->Text.Length()<=0)
	{
		ShowMessage("没有给出文件名，请先点击浏览给出文件名！");
		ButtonBrowse->SetFocus();
		return;
    }
	if((EditPassword1->Text !=EditPassword2->Text) || (EditPassword1->Text.Length()<8))
	{
		ShowMessage("两次输入的密码不一致或密码长度小于8，请更正！");
		EditPassword2->SetFocus();
		return;
	}

	if(IsFileExsist(EditFileName->Text.c_str(),false))
	{
		ShowMessage("文件已存在，请重新选择文件名！");
		ButtonBrowse->SetFocus();
		return;
	}

	//SDZ added 01151813>>
	char*	DriveStr="C:";
	memcpy(DriveStr,EditFileName->Text.c_str(),2);

	if(DriveStr[1]==':')
	{

		ULARGE_INTEGER	FreeSpaceToCall,TotalSpace,FreeSpace;
		if(::GetDiskFreeSpaceEx(DriveStr,&FreeSpaceToCall,&TotalSpace,&FreeSpace))
		{
			if((FreeSpace.QuadPart/1024/1024)<(CSpinEditSize->Value))
			{
				ShowMessage("指定的文件尺寸太大，空间不足！");
				CSpinEditSize->Value=FreeSpace.QuadPart/1024/1024-1;
				return;
			}
		}
		else
		{
			ShowMessage("检查可用剩余空间出错，请重试！");
			return;
		}
	}
	else
		return;

	CancelExit=false;
	Close();	
}
//---------------------------------------------------------------------------

void __fastcall TFormNewBox::FormCreate(TObject *Sender)
{
	CenterForm(this);
}
//---------------------------------------------------------------------------

void __fastcall TFormNewBox::ButtonBrowseClick(TObject *Sender)
{
	TOpenDialog* OpenDialog=new TOpenDialog(this);

	if(OpenDialog->Execute())
	{
		if(IsFileExsist(OpenDialog->FileName.c_str(),FALSE))
		{
            ShowMessage("文件已存在，请重新命名加密容器文件！");
			return;
        }
		EditFileName->Text=OpenDialog->FileName;
    }	
}
//---------------------------------------------------------------------------

