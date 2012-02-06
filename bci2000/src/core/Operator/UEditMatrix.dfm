object fEditMatrix: TfEditMatrix
  Left = 534
  Top = 252
  AutoScroll = False
  Caption = 'Edit Matrix'
  ClientHeight = 328
  ClientWidth = 363
  Color = clBtnFace
  Constraints.MinHeight = 60
  Constraints.MinWidth = 60
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Default'
  Font.Style = []
  OldCreateOrder = False
  OnClose = FormClose
  DesignSize = (
    363
    328)
  PixelsPerInch = 96
  TextHeight = 13
  object tComment: TLabel
    Left = 8
    Top = 3
    Width = 351
    Height = 40
    Anchors = [akLeft, akTop, akRight]
    AutoSize = False
    Caption = 'Comment'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -11
    Font.Name = 'MS Sans Serif'
    Font.Style = [fsBold]
    ParentFont = False
    WordWrap = True
  end
  object Label1: TLabel
    Left = 8
    Top = 32
    Width = 61
    Height = 13
    Caption = '# of columns'
  end
  object Label2: TLabel
    Left = 87
    Top = 32
    Width = 44
    Height = 13
    Caption = '# of rows'
  end
  object StringGrid: TStringGrid
    Left = 8
    Top = 72
    Width = 350
    Height = 246
    Anchors = [akLeft, akTop, akRight, akBottom]
    DefaultColWidth = 32
    DefaultRowHeight = 16
    Options = [goFixedVertLine, goFixedHorzLine, goVertLine, goHorzLine, goRangeSelect, goDrawFocusSelected, goColSizing, goEditing]
    ParentShowHint = False
    ShowHint = True
    TabOrder = 0
    OnDrawCell = StringGridDrawCell
    OnSelectCell = StringGridSelectCell
  end
  object cNumCols: TCSpinEdit
    Left = 8
    Top = 48
    Width = 41
    Height = 22
    MaxValue = 32767
    MinValue = 1
    TabOrder = 1
    Value = 4
  end
  object cNumRows: TCSpinEdit
    Left = 87
    Top = 48
    Width = 41
    Height = 22
    MaxValue = 32767
    MinValue = 1
    TabOrder = 2
    Value = 4
  end
  object bChangeMatrixSize: TButton
    Left = 149
    Top = 45
    Width = 108
    Height = 24
    Caption = 'Set new matrix size'
    TabOrder = 3
    OnClick = bChangeMatrixSizeClick
  end
end
