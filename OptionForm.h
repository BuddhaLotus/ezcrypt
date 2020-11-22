//---------------------------------------------------------------------------

#ifndef OptionFormH
#define OptionFormH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>

//---------------------------------------------------------------------------
class TFormOption : public TForm
{
__published:	// IDE-managed Components
	TButton *ButtonBrowse;
	TEdit *EditBoxName;
	TLabel *LabelBoxName;
	TGroupBox *GroupBoxChpass;
	TEdit *EditOldPass;
	TEdit *EditPass1;
	TEdit *EditPass2;
	TButton *ButtonChpass;
	TButton *ButtonClose;
	TLabel *LabelOldPass;
	TLabel *LabelNewPass1;
	TLabel *LabelNewPass2;
	TGroupBox *GroupBox1;
	TCheckBox *CheckBoxAutoMount;
	TButton *ButtonBrowseAuto;
	TEdit *EditAutoMount;
	TLabel *LabelAutoMount;
	TButton *ButtonOK;
	void __fastcall ButtonBrowseClick(TObject *Sender);
	void __fastcall ButtonChpassClick(TObject *Sender);
	void __fastcall ButtonCloseClick(TObject *Sender);
	void __fastcall FormCreate(TObject *Sender);
	void __fastcall CheckBoxAutoMountClick(TObject *Sender);
	void __fastcall ButtonOKClick(TObject *Sender);
private:	// User declarations
public:		// User declarations
	__fastcall TFormOption(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TFormOption *FormOption;
//---------------------------------------------------------------------------
#endif
