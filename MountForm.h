//---------------------------------------------------------------------------

#ifndef MountFormH
#define MountFormH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
//---------------------------------------------------------------------------
class TFormMount : public TForm
{
__published:	// IDE-managed Components
	TButton *ButtonMount;
	TButton *ButtonCancel;
	TEdit *EditPassword;
	TLabel *LabelPassword;
	TButton *ButtonExpert;
	TPanel *PanelExpert;
	TLabel *LabelDrivers;
	TComboBox *ComboBoxDrivers;
	TCheckBox *CheckBoxRO;
	TCheckBox *CheckBoxMountExit;
	void __fastcall ButtonExpertClick(TObject *Sender);
	void __fastcall ButtonCancelClick(TObject *Sender);
	void __fastcall FormShow(TObject *Sender);
	void __fastcall ButtonMountClick(TObject *Sender);
	void __fastcall EditPasswordKeyPress(TObject *Sender, char &Key);
	void __fastcall FormCreate(TObject *Sender);
private:	// User declarations
public:		// User declarations
	__fastcall TFormMount(TComponent* Owner);
	BOOL			LocalExpertMode;
	BOOL			CancelExit;	
};
//---------------------------------------------------------------------------
extern PACKAGE TFormMount *FormMount;
//---------------------------------------------------------------------------
#endif
