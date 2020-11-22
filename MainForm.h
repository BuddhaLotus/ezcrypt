//---------------------------------------------------------------------------

#ifndef MainFormH
#define MainFormH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <Buttons.hpp>
#include <ComCtrls.hpp>
#include <Dialogs.hpp>
#include <ExtCtrls.hpp>
#include <Menus.hpp>
#include <Grids.hpp>
//---------------------------------------------------------------------------
class TFormMain : public TForm
{
__published:	// IDE-managed Components
	TBitBtn *BitBtnCreate;
	TBitBtn *BitBtnMount;
	TBitBtn *BitBtnClose;
	TBitBtn *BitBtnExit;
	TBitBtn *BitBtnMode;
	TStatusBar *StatusBar;
	TOpenDialog *OpenDialog;
	TTrayIcon *TrayIcon;
	TPopupMenu *TrayPopupMenu;
	TPopupMenu *VolsPopupMenu;
	TMenuItem *mnuAbout;
	TBitBtn *BitBtnOption;
	TMenuItem *mnuShowMainForm;
	TMenuItem *mnuCloseAll;
	TMenuItem *mnuExit;
	TMenuItem *mnuSpliter0;
	TMenuItem *mnuCloseit;
	TMenuItem *mnuCloseAllVol;
	TStringGrid *StringGridDrvInfo;
	void __fastcall BitBtnExitClick(TObject *Sender);
	void __fastcall BitBtnModeClick(TObject *Sender);
	void __fastcall mnuAboutClick(TObject *Sender);
	void __fastcall FormCreate(TObject *Sender);
	void __fastcall BitBtnCreateClick(TObject *Sender);
	void __fastcall BitBtnMountClick(TObject *Sender);

	void __fastcall ReportError(char* ErrorString,int ReportLevel);
	void __fastcall mnuShowMainFormClick(TObject *Sender);
	void __fastcall mnuExitClick(TObject *Sender);
	void __fastcall mnuCloseAllClick(TObject *Sender);
	void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
	void __fastcall TrayIconDblClick(TObject *Sender);
	void __fastcall BitBtnCloseClick(TObject *Sender);
	//void __fastcall mnuCloseitClick(TObject *Sender);
	void __fastcall mnuCloseAllVolClick(TObject *Sender);
	void __fastcall FormShow(TObject *Sender);
	void __fastcall FormHide(TObject *Sender);
	void __fastcall BitBtnOptionClick(TObject *Sender);
	void __fastcall mnuCloseitClick(TObject *Sender);
	void __fastcall StringGridDrvInfoContextPopup(TObject *Sender,
          TPoint &MousePos, bool &Handled);

private:	// User declarations
public:		// User declarations
	int 	CmdArgc;
	LPWSTR* CmdArgv;

	__fastcall TFormMain(TComponent* Owner);

	BEGIN_MESSAGE_MAP
		MESSAGE_HANDLER(WM_HOTKEY,TMessage,OnHotKey)
	END_MESSAGE_MAP(TForm)
	void __fastcall OnHotKey(TMessage &Message);
	
};
//---------------------------------------------------------------------------
extern PACKAGE TFormMain *FormMain;
//---------------------------------------------------------------------------
#endif
