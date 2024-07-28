#pragma once

#include "framework.hpp"

LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

extern CONST UINT WM_FINDREPLACE;

typedef struct {

  INT length;
  LPSTR buffer;

  HWND hDlg;
  HWND hEdit;
  WCHAR req[256];

  bool saved;

} USERDATA, *LPUSERDATA;

void updAfterRadio(HWND hWnd, BOOL bEnable);
void updAfterAES(HWND hWnd, LPUSERDATA data, bool enc);

inline HWND GetFindDlg(HWND hWnd) { return ((LPUSERDATA)GetWindowLongPtr(hWnd, GWLP_USERDATA))->hDlg; }
bool CustomCompare(LPCWSTR str1, LPCWSTR str2, int length);

enum ControlID : WORD {

  CONTROL_BEGIN = 1ui16,

  DataEdit = CONTROL_BEGIN,
  AccessMessageEdit,
  LoadButton,
  SaveButton,
  EditButton,
  RepresentationGroupBox,
  BinaryRadio,
  TextRadio,
  EraseButton,
  MenuGroupBox,
  EncryptionButton,
  DecryptionButton,
  ApprovalCheckbox,

  CONTROL_END = ApprovalCheckbox,

  CONTROL_COUNT = CONTROL_END - CONTROL_BEGIN + 1ui16

};

void FromText(HWND edit, int& length, LPSTR& buffer);
void GetKey(HWND hWnd, key256_t key);
bool WrongPKCS(LPUSERDATA data, int count);

enum SubclassID : UINT_PTR {

  DataSubclass = 1ULL,
  AccessMessageSubclass

};

LRESULT CALLBACK EditSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);
