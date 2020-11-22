//---------------------------------------------------------------------------

#ifndef CreateFormH
#define CreateFormH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include "cspin.h"
#include <ExtCtrls.hpp>
//#include <Dialogs.hpp>
//---------------------------------------------------------------------------
class TFormNewBox : public TForm
{
__published:	// IDE-managed Components
	TEdit *EditFileName;
	TButton *ButtonBrowse;
	TEdit *EditPassword1;
	TEdit *EditPassword2;
	TCSpinEdit *CSpinEditSize;
	TButton *ButtonCreate;
	TButton *ButtonExpert;
	TButton *ButtonCancel;
	TLabel *LabelFileName;
	TLabel *LabelPassword;
	TLabel *LabelSize;
	TLabel *LabelM;
	TRadioGroup *RadioGroupAlgo;
	TLabel *Label1;
	void __fastcall ButtonExpertClick(TObject *Sender);
	void __fastcall ButtonCancelClick(TObject *Sender);
	void __fastcall FormShow(TObject *Sender);
	void __fastcall ButtonCreateClick(TObject *Sender);
	void __fastcall FormCreate(TObject *Sender);
	void __fastcall ButtonBrowseClick(TObject *Sender);
private:	// User declarations
public:		// User declarations
	__fastcall TFormNewBox(TComponent* Owner);
	//char		BoxFileName[MAX_PATH+1];
	BOOL			LocalExpertMode;
	BOOL			CancelExit;
};
//---------------------------------------------------------------------------
extern PACKAGE TFormNewBox *FormNewBox;
//---------------------------------------------------------------------------
#endif
