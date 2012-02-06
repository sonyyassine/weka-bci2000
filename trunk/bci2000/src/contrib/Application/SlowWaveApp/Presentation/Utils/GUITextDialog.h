/////////////////////////////////////////////////////////////////////////////
//
// File: GUITextDialog.h
//
// Date: Nov 22, 2001
//
// Author: Juergen Mellinger
//
// Description: A class that opens a GUI window and displays text inside it.
//
// Changes:
//
// (C) 2000-2008, BCI2000 Project
// http://www.bci2000.org
//////////////////////////////////////////////////////////////////////////////

#ifndef GUI_TEXT_DIALOG_H
#define GUI_TEXT_DIALOG_H

#include "OSIncludes.h"

class TGUITextDialog
{
  public:
            TGUITextDialog();
            ~TGUITextDialog();

    void    ShowTextNonmodal(   const char  *inTitle,
                                const char  *inText );
    void    ShowTextModal( const char   *inTitle,
                           const char   *inText );

  private:
#ifdef VCL
    class TTextForm : public TForm
    {
      public:
             __fastcall TTextForm();
             __fastcall ~TTextForm() {}

        void __fastcall OKButtonClickHandler(   TObject* );
        void __fastcall FormCloseHandler(   TObject*,
                                            TCloseAction &Action );
        TMemo   *memo;
    };
    TTextForm   *form;
#endif // VCL
};

#endif // GUI_TEXT_DIALOG_H
 
