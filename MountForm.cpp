//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "MountForm.h"

#include "Global.h"

//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TFormMount *FormMount;

//---------------------------------------------------------------------------

__fastcall TFormMount::TFormMount(TComponent* Owner)
	: TForm(Owner)
{
	LocalExpertMode=true;
	CancelExit=true;
	ButtonExpertClick(NULL);
#ifdef		LiteVersion
	ButtonExpert->Visible=false;
	ButtonExpert->Enabled=false;
	//sdz 070823
	ButtonMount->Left+=30;
	ButtonCancel->Left-=30;
#endif
}
//---------------------------------------------------------------------------



void __fastcall TFormMount::ButtonExpertClick(TObject *Sender)
{
	if(LocalExpertMode)
	{
		PanelExpert->Visible=false;
		PanelExpert->Enabled=false;

		ButtonMount->Top=PanelExpert->Top;
		ButtonExpert->Top=ButtonMount->Top;

		PanelExpert->Top=ButtonCancel->Top;

		ButtonCancel->Top=ButtonMount->Top;

		this->ClientHeight-=PanelExpert->Height;
	}
	else
	{
		ButtonMount->Top=PanelExpert->Top;
		ButtonExpert->Top=ButtonMount->Top;

		PanelExpert->Top=ButtonCancel->Top;

		ButtonCancel->Top=ButtonMount->Top;

		PanelExpert->Visible=true;
		PanelExpert->Enabled=true;
		
		this->ClientHeight+=PanelExpert->Height;

	}

	LocalExpertMode=!LocalExpertMode;
}
//---------------------------------------------------------------------------

void __fastcall TFormMount::ButtonCancelClick(TObject *Sender)
{
    CancelExit=true;
	Close();
	//this->res	
}
//---------------------------------------------------------------------------

void __fastcall TFormMount::FormShow(TObject *Sender)
{
	EditPassword->SetFocus();
}
//---------------------------------------------------------------------------


void __fastcall TFormMount::ButtonMountClick(TObject *Sender)
{
	CancelExit=false;
	Close();
}
//---------------------------------------------------------------------------

void __fastcall TFormMount::EditPasswordKeyPress(TObject *Sender, char &Key)
{
	if(Key=='\r')
    	ButtonMountClick(NULL);
}
//---------------------------------------------------------------------------

void __fastcall TFormMount::FormCreate(TObject *Sender)
{
	ListFreeDriverLetter(ComboBoxDrivers);
	CenterForm(this);	
}
//---------------------------------------------------------------------------

