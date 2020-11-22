//---------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "AboutForm.h"

#include "Global.h"
//#include "TestForm.h"

//--------------------------------------------------------------------- 
#pragma resource "*.dfm"
TAboutBox *AboutBox;
//--------------------------------------------------------------------- 
__fastcall TAboutBox::TAboutBox(TComponent* AOwner)
	: TForm(AOwner)
{
}
//---------------------------------------------------------------------
void __fastcall TAboutBox::FormCreate(TObject *Sender)
{
	CenterForm(this);	
}
//---------------------------------------------------------------------------

void __fastcall TAboutBox::OKButtonClick(TObject *Sender)
{
/*
	TFormTest *abcform=new TFormTest(Application);
	abcform->Show();
*/
}
//---------------------------------------------------------------------------
 
