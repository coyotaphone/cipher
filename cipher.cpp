#include "cipher.hpp"
#include "resource.h"

using namespace std;

CONST UINT WM_FINDREPLACE = RegisterWindowMessage(FINDMSGSTRING);

INT WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ INT nCmdShow) {
  
  WNDCLASS wndClass {
    0U, WindowProc, 0, 0, hInstance,
    LoadIcon(nullptr, IDI_APPLICATION),
    LoadCursor(nullptr, IDC_ARROW),
    CreateSolidBrush(RGB(192, 192, 192)),
    nullptr, TEXT(NAME_STR)
  };
  auto lpClassName = (LPCWSTR)RegisterClass(&wndClass);
  
  RECT rect { 0L, 0L, 1200L, 675L };
  constexpr DWORD dwStyle = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
  AdjustWindowRect(&rect, dwStyle, FALSE);
  int nWidth = rect.right - rect.left;
  int nHeight = rect.bottom - rect.top;
  SystemParametersInfo(SPI_GETWORKAREA, 0U, &rect, 0U);

  HFONT hFont = CreateFont(
    20, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET,
    OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
    DEFAULT_PITCH | FF_SWISS, L"Consolas"
  );
  
  HWND hWnd = CreateWindow(
    lpClassName, L"No data - Cipher", dwStyle,
    (rect.left + rect.right - nWidth) >> 1,
    (rect.top + rect.bottom - nHeight) >> 1,
    nWidth, nHeight, nullptr, nullptr,
    hInstance, hFont
  );
  ShowWindow(hWnd, nCmdShow);

  MSG msg;
  while (GetMessage(&msg, nullptr, 0U, 0U))
    if (!IsDialogMessage(GetFindDlg(hWnd), &msg)) {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }

  DeleteObject(hFont);

  UnregisterClass(lpClassName, hInstance);

  return 0;

}

LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
  
  switch (uMsg) {

  case WM_CREATE: {

    auto data = new USERDATA { 0, new CHAR[0], nullptr, nullptr, L"", false };
    SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)data);

    auto hInstance = (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE);
    HWND controls[CONTROL_COUNT] {
      CreateWindow(
        WC_EDIT, L"", WS_VISIBLE | WS_CHILD | WS_VSCROLL | WS_BORDER |
        ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL | ES_NOHIDESEL | ES_READONLY,
        20, 20, 940, 400, hWnd, (HMENU)DataEdit, hInstance, nullptr
      ),
      CreateWindow(
        WC_EDIT, L"", WS_VISIBLE | WS_CHILD | WS_VSCROLL | WS_BORDER |
        ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL | ES_NOHIDESEL,
        20, 440, 940, 215, hWnd, (HMENU)AccessMessageEdit, hInstance, nullptr
      ),
      CreateWindow(
        WC_BUTTON, L"Load data from file", WS_VISIBLE | WS_CHILD | BS_FLAT,
        980, 20, 200, 50, hWnd, (HMENU)LoadButton, hInstance, nullptr
      ),
      CreateWindow(
        WC_BUTTON, L"Save data to file", WS_VISIBLE | WS_CHILD | BS_FLAT,
        980, 90, 200, 50, hWnd, (HMENU)SaveButton, hInstance, nullptr
      ),
      CreateWindow(
        WC_BUTTON, L"Edit data as text", WS_VISIBLE | WS_CHILD | BS_FLAT | WS_DISABLED,
        980, 160, 200, 50, hWnd, (HMENU)EditButton, hInstance, nullptr
      ),
      CreateWindow(
        WC_BUTTON, L"Representation", WS_VISIBLE | WS_CHILD | BS_FLAT | BS_GROUPBOX | BS_CENTER,
        980, 230, 200, 80, hWnd, (HMENU)RepresentationGroupBox, hInstance, nullptr
      ),
      CreateWindow(
        WC_BUTTON, L"Binary", WS_VISIBLE | WS_CHILD | BS_FLAT | BS_AUTORADIOBUTTON | BS_CENTER,
        990, 255, 80, 40, hWnd, (HMENU)BinaryRadio, hInstance, nullptr
      ),
      CreateWindow(
        WC_BUTTON, L"Text", WS_VISIBLE | WS_CHILD | BS_FLAT | BS_AUTORADIOBUTTON | BS_CENTER,
        1090, 255, 80, 40, hWnd, (HMENU)TextRadio, hInstance, nullptr
      ),
      CreateWindow(
        WC_BUTTON, L"Erase data", WS_VISIBLE | WS_CHILD | BS_FLAT,
        980, 330, 200, 50, hWnd, (HMENU)EraseButton, hInstance, nullptr
      ),
      CreateWindow(
        WC_BUTTON, L"Menu", WS_VISIBLE | WS_CHILD | BS_FLAT | BS_GROUPBOX | BS_CENTER,
        980, 440, 200, 215, hWnd, (HMENU)MenuGroupBox, hInstance, nullptr
      ),
      CreateWindow(
        WC_BUTTON, L"Encrypt data", WS_VISIBLE | WS_CHILD | BS_FLAT | WS_DISABLED,
        1005, 475, 150, 40, hWnd, (HMENU)EncryptionButton, hInstance, nullptr
      ),
      CreateWindow(
        WC_BUTTON, L"Decrypt data", WS_VISIBLE | WS_CHILD | BS_FLAT | WS_DISABLED,
       1005, 535, 150, 40, hWnd, (HMENU)DecryptionButton, hInstance, nullptr
      ),
      CreateWindow(
        WC_BUTTON, L"Approve access message", WS_VISIBLE | WS_CHILD | BS_FLAT |
        BS_AUTOCHECKBOX | BS_MULTILINE | BS_CENTER,
        1005, 585, 150, 50, hWnd, (HMENU)ApprovalCheckbox, hInstance, nullptr
      )
    };

    SetWindowSubclass(controls[DataEdit - 1], EditSubclassProc,
      DataSubclass, (DWORD_PTR)new BOOL(FALSE));
    SetWindowSubclass(controls[AccessMessageEdit - 1], EditSubclassProc,
      AccessMessageSubclass, (DWORD_PTR)new BOOL(FALSE));

    CheckDlgButton(hWnd, BinaryRadio, BST_CHECKED);

    auto font = (WPARAM)((LPCREATESTRUCT)lParam)->lpCreateParams;
    for (HWND* ptr = controls, *end = controls + CONTROL_COUNT; ptr < end; ++ptr)
      SendMessage(*ptr, WM_SETFONT, font, TRUE);

    return 0LL;
  }
  
  case WM_DESTROY: {
    auto data = (LPUSERDATA)GetWindowLongPtr(hWnd, GWLP_USERDATA);
    for (WORD id = CONTROL_BEGIN; id <= CONTROL_END; ++id)
      DestroyWindow(GetDlgItem(hWnd, id));
    if (data->buffer)
      delete[] data->buffer;
    delete data;
    PostQuitMessage(0);
    return 0LL;
  }

  case WM_CLOSE:
    if (((LPUSERDATA)GetWindowLongPtr(hWnd, GWLP_USERDATA))->saved ||
      !GetWindowTextLength(GetDlgItem(hWnd, DataEdit)) ||
      MessageBox(hWnd, L"Current data will be lost. Are you sure you want to exit?",
      L"Warning", MB_ICONWARNING | MB_YESNO) == IDYES)
      DestroyWindow(hWnd);
    return 0LL;

  case WM_COMMAND:
    switch (LOWORD(wParam)) {
      case LoadButton: {
        auto data = (LPUSERDATA)GetWindowLongPtr(hWnd, GWLP_USERDATA);
        if (!data->saved && GetWindowTextLength(GetDlgItem(hWnd, DataEdit)) &&
          MessageBox(hWnd, L"Current data will be lost. Are you sure you want to load new data?",
          L"Warning", MB_ICONWARNING | MB_YESNO) != IDYES)
          return 0LL;

        WCHAR path[MAX_PATH] = L"";
        OPENFILENAME ofn {
          sizeof(OPENFILENAME), hWnd, nullptr,
          L"All Files\000*.*\000", nullptr, 0UL, 1UL, path,
          MAX_PATH, nullptr, 0UL, nullptr, nullptr,
          OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST,
          0ui16, 0ui16, nullptr, 0LL, nullptr, nullptr
        };
        if (!GetOpenFileName(&ofn))
          return 0LL;

        ifstream file(path, ios::in | ios::ate | ios::binary);
        if (!file.is_open()) {
          MessageBox(hWnd, L"Failed to open the file", L"Error", MB_OK | MB_ICONERROR);
          return 0LL;
        }

        data->length = (int)file.tellg();
        if (data->buffer)
          delete[] data->buffer;
        data->buffer = new CHAR[data->length];
        file.seekg(0LL, ios::beg);
        file.read(data->buffer, data->length);

        SendMessage(GetDlgItem(hWnd, BinaryRadio), BM_CLICK, 0ULL, 0LL);
        SetWindowText(hWnd, L"Data loaded - Cipher");
        data->saved = true;
        return 0LL;
      }
      case SaveButton: {
        WCHAR path[MAX_PATH] = L"";
        OPENFILENAME ofn {
          sizeof(OPENFILENAME), hWnd, nullptr,
          L"All Files\000*.*\000", nullptr, 0UL, 1UL, path,
          MAX_PATH, nullptr, 0UL, nullptr, nullptr,
          OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT,
          0ui16, 0ui16, nullptr, 0LL, nullptr, nullptr
        };
        if (!GetSaveFileName(&ofn))
          return 0LL;

        ofstream file(path, ios::out | ios::binary);
        if (!file.is_open()) {
          MessageBox(hWnd, L"Failed to open the file", L"Error", MB_OK | MB_ICONERROR);
          return 0LL;
        }

        auto data = (LPUSERDATA)GetWindowLongPtr(hWnd, GWLP_USERDATA);
        if (!data->buffer)
          FromText(GetDlgItem(hWnd, DataEdit), data->length, data->buffer);
        file.write(data->buffer, data->length);

        SetWindowText(hWnd, L"Data saved - Cipher");
        data->saved = true;
        return 0LL;
      }
      case EditButton: {
        auto data = (LPUSERDATA)GetWindowLongPtr(hWnd, GWLP_USERDATA);
        HWND edit = GetDlgItem(hWnd, DataEdit);
        if (data->buffer) {
          if (GetWindowTextLength(edit) && MessageBox(hWnd, L"Possible data loss!\r\n"
          L"This will irreversibly convert current binary data to UTF-8 CRLF text. "
          L"Incompatible values of data will be corrupted and lost. "
          L"It is not recommended to proceed if the data is not actually a text. "
          L"Are you sure you want to continue?",
          L"Warning", MB_ICONWARNING | MB_YESNO) != IDYES)
            return 0LL;
          delete[] data->buffer;
          data->buffer = nullptr;
        }
        BOOL check = GetWindowLongPtr(edit, GWL_STYLE) & ES_READONLY;
        SetDlgItemText(hWnd, EditButton, check ? L"Stop editing text" : L"Edit data as text");
        SendMessage(edit, EM_SETREADONLY, !check, 0LL);
        SetWindowText(hWnd, L"Data edited as text - Cipher");
        data->saved = false;
        return 0LL;
      }
      case BinaryRadio: {
        auto data = (LPUSERDATA)GetWindowLongPtr(hWnd, GWLP_USERDATA);
        if (!data->buffer)
          FromText(GetDlgItem(hWnd, DataEdit), data->length, data->buffer);

        if (data->length > 1 << 20) {
          SetDlgItemText(hWnd, DataEdit, L"< The data is too large to represent (over than 1 MB) >");
          updAfterRadio(hWnd, FALSE);
          return 0LL;
        }

        wstringstream ss;
        ss << hex << uppercase << setfill(L'0');
        for (auto ptr = (uint8_t*)data->buffer, end = ptr + data->length; ptr < end; ++ptr) {
          ss << setw(2) << *ptr;
          if (ptr < end - 1)
            ss << L' ';
        }
        SetDlgItemText(hWnd, DataEdit, ss.str().c_str());

        updAfterRadio(hWnd, FALSE);
        return 0LL;
      }
      case TextRadio: {
        auto data = (LPUSERDATA)GetWindowLongPtr(hWnd, GWLP_USERDATA);
        if (!data->buffer)
          return 0LL;

        if (data->length > 1 << 20) {
          SetDlgItemText(hWnd, DataEdit, L"< The data is too large to represent (over than 1 MB) >");
          updAfterRadio(hWnd, FALSE);
          return 0LL;
        }

        int wSize = MultiByteToWideChar(CP_UTF8, 0UL, data->buffer, data->length, nullptr, 0);
        LPWSTR buffer = new WCHAR[wSize + 1];
        MultiByteToWideChar(CP_UTF8, 0UL, data->buffer, data->length, buffer, wSize);
        buffer[wSize] = L'\000';
        for (PWCHAR ptr = buffer, end = buffer + wSize; ptr < end; ++ptr)
          if (!*ptr)
            *ptr = L' ';
        SetDlgItemText(hWnd, DataEdit, buffer);
        delete[] buffer;

        updAfterRadio(hWnd, TRUE);
        return 0LL;
      }
      case EraseButton: {
        auto data = (LPUSERDATA)GetWindowLongPtr(hWnd, GWLP_USERDATA);
        if (!GetWindowTextLength(GetDlgItem(hWnd, DataEdit)) || (!data->saved && MessageBox(hWnd,
          L"Current data will be lost. Are you sure you want to erase it?",
          L"Warning", MB_YESNO | MB_ICONWARNING) != IDYES))
          return 0LL;
        SetDlgItemText(hWnd, DataEdit, L"");
        if (data->buffer) {
          delete[] data->buffer;
          data->buffer = nullptr;
        }
        SetWindowText(hWnd, L"Data erased - Cipher");
        return 0LL;
      }
      case EncryptionButton: {
        auto data = (LPUSERDATA)GetWindowLongPtr(hWnd, GWLP_USERDATA);
        HWND edit = GetDlgItem(hWnd, DataEdit);
        if (!data->buffer)
          FromText(edit, data->length, data->buffer);

        int remainder = data->length & 0x0f;
        int count = 0x10 - remainder;
        int length = data->length + count;
        LPSTR buffer = new CHAR[length];
        CopyMemory(buffer, data->buffer, data->length);
        delete[] data->buffer;
        data->buffer = buffer;

        SetWindowText(hWnd, L"Encryption in process - Cipher");

        key256_t key;
        GetKey(hWnd, key);
        uint8_t* ptr = (uint8_t*)data->buffer;
        for (uint8_t* end = ptr + (data->length & ~0x0f); ptr < end; ptr += 0x10)
          AES256(ptr, key, ptr);
        FillMemory(ptr + remainder, count, count);
        AES256(ptr, key, ptr);
        data->length = length;

        updAfterAES(hWnd, data, true);
        return 0LL;
      }
      case DecryptionButton: {
        auto data = (LPUSERDATA)GetWindowLongPtr(hWnd, GWLP_USERDATA);
        HWND edit = GetDlgItem(hWnd, DataEdit);
        if (!data->buffer)
          FromText(edit, data->length, data->buffer);

        if (data->length < 0x10) {
          MessageBox(hWnd, L"There has to be at least 16 bytes of data to decrypt!",
            L"Error", MB_OK | MB_ICONERROR);
          updAfterAES(hWnd, data, false);
          return 0LL;
        }

        if ((data->length & 0x0f) && MessageBox(hWnd, L"Decryption canceled! "
          L"The data will be truncated to a multiple of 16 bytes. "
          L"Remaining data will be clipped, ignored and lost. "
          L"Are you sure you want to continue the decryption?",
          L"Warning", MB_YESNO | MB_ICONWARNING) != IDYES)
          return 0LL;
        data->length &= ~0x0f;

        SetWindowText(hWnd, L"Decryption in process - Cipher");

        key256_t key;
        GetKey(hWnd, key);
        uint8_t* ptr = (uint8_t*)data->buffer;
        for (uint8_t* end = ptr + data->length; ptr < end; ptr += 0x10)
          InvAES256(ptr, key, ptr);
         
        int count = data->buffer[data->length - 1];
        if (WrongPKCS(data, count))
          MessageBox(hWnd, L"Data has been decrypted but the truncation failed! "
            L"Invalid PKCS padding.", L"Error", MB_ICONERROR | MB_OK);
        else {
          data->length -= count;
          LPSTR buffer = new CHAR[data->length];
          CopyMemory(buffer, data->buffer, data->length);
          delete[] data->buffer;
          data->buffer = buffer;
        }
        
        updAfterAES(hWnd, data, false);
        return 0LL;
      }
      case ApprovalCheckbox: {
        BOOL check = IsDlgButtonChecked(hWnd, ApprovalCheckbox) == BST_CHECKED;
        EnableWindow(GetDlgItem(hWnd, EncryptionButton), check);
        EnableWindow(GetDlgItem(hWnd, DecryptionButton), check);
        SendMessage(GetDlgItem(hWnd, AccessMessageEdit), EM_SETREADONLY, check, 0LL);
        return 0LL;
      }
      default:
        return 0LL;
    }
  
  case WM_CTLCOLORSTATIC: {
    auto child = (HWND)lParam;
    if (child != GetDlgItem(hWnd, RepresentationGroupBox) &&
      child != GetDlgItem(hWnd, TextRadio) &&
      child != GetDlgItem(hWnd, BinaryRadio) &&
      child != GetDlgItem(hWnd, MenuGroupBox) &&
      child != GetDlgItem(hWnd, ApprovalCheckbox))
      return DefWindowProc(hWnd, uMsg, wParam, lParam);
    SetBkMode((HDC)wParam, TRANSPARENT);
    return GetClassLongPtr(hWnd, GCLP_HBRBACKGROUND);
  }
  
  default:
    if (uMsg == WM_FINDREPLACE) {
      auto find = (LPFINDREPLACE)lParam;
      auto data = (LPUSERDATA)GetWindowLongPtr(hWnd, GWLP_USERDATA);
      if (find->Flags & FR_DIALOGTERM) {
        delete find;
        data->hDlg = nullptr;
        data->hEdit = nullptr;
        ZeroMemory(data->req, sizeof(data->req));
        return 0LL;
      }
      if (!(find->Flags & FR_FINDNEXT))
        return 0LL;
      int direction = find->Flags & FR_DOWN ? 1 : -1;
      BOOL matchCase = find->Flags & FR_MATCHCASE;
      int textLength = GetWindowTextLength(data->hEdit);
      LPWSTR editText = new WCHAR[textLength + 1];
      GetWindowText(data->hEdit, editText, textLength + 1);
      DWORD beg;
      DWORD end;
      SendMessage(data->hEdit, EM_GETSEL, (WPARAM)&beg, (LPARAM)&end);
      int findLength = (int)wcslen(data->req);
      bool notfound = true;
      for (int i = direction == 1 ? end : beg - 1;
        direction == 1 ? i <= textLength - findLength : i >= 0;
        i += direction)
        if ((matchCase && !wcsncmp(editText + i, data->req, findLength)) ||
          (!matchCase && CustomCompare(editText + i, data->req, findLength))) {
          SendMessage(data->hEdit, EM_SETSEL, i, i + findLength);
          SendMessage(data->hEdit, EM_SCROLLCARET, 0ULL, 0LL);
          notfound = false;
          break;
        }
      delete[] editText;
      if (notfound)
        MessageBeep(MB_ICONEXCLAMATION);
      return 0LL;
    }

    return DefWindowProc(hWnd, uMsg, wParam, lParam);

  }

}

void updAfterRadio(HWND hWnd, BOOL bEnable) {
  EnableWindow(GetDlgItem(hWnd, EditButton), bEnable);
  SetDlgItemText(hWnd, EditButton, L"Edit data as text");
  SendMessage(GetDlgItem(hWnd, DataEdit), EM_SETREADONLY, TRUE, 0LL);
}

void updAfterAES(HWND hWnd, LPUSERDATA data, bool enc) {
  SendMessage(GetDlgItem(hWnd, IsDlgButtonChecked(hWnd, BinaryRadio) == BST_CHECKED ?
    BinaryRadio : TextRadio), BM_CLICK, 0ULL, 0LL);
  SendMessage(GetDlgItem(hWnd, ApprovalCheckbox), BM_CLICK, 0ULL, 0LL);
  SetWindowText(hWnd, enc ? L"Data encrypted - Cipher" : L"Data decrypted - Cipher");
  data->saved = false;
}

bool CustomCompare(LPCWSTR str1, LPCWSTR str2, int length) {
  LPWSTR lowerStr1 = new WCHAR[length + 1];
  LPWSTR lowerStr2 = new WCHAR[length + 1];
  LCMapString(LOCALE_USER_DEFAULT, LCMAP_LOWERCASE, str1, length, lowerStr1, length + 1);
  LCMapString(LOCALE_USER_DEFAULT, LCMAP_LOWERCASE, str2, length, lowerStr2, length + 1);
  bool result = wcsncmp(lowerStr1, lowerStr2, length) == 0;
  delete[] lowerStr1;
  delete[] lowerStr2;
  return result;
}

void FromText(HWND edit, int& length, LPSTR& buffer) {
  int len = GetWindowTextLength(edit);
  LPWSTR str = new WCHAR[len + 1];
  GetWindowText(edit, str, len + 1);
  length = WideCharToMultiByte(CP_UTF8, 0UL, str, len, nullptr, 0, nullptr, nullptr);
  buffer = new CHAR[length];
  WideCharToMultiByte(CP_UTF8, 0UL, str, len, buffer, length, nullptr, nullptr);
  delete[] str;
}

void GetKey(HWND hWnd, key256_t key) {

  int length;
  LPSTR buffer;
  FromText(GetDlgItem(hWnd, AccessMessageEdit), length, buffer);

  SHA256* msg;
  CreateSHA256(&msg);
  WriteSHA256(msg, (size_t)length << 3, buffer);
  delete[] buffer;
  SnapshotSHA256(msg, key);
  DestroySHA256(msg);

}

bool WrongPKCS(LPUSERDATA data, int count) {
  if (count < 0x01 || count > 0x10 || data->length < count)
    return true;
  for (PCHAR ptr = data->buffer + data->length - 1, end = ptr - count; ptr > end; --ptr)
    if (*ptr != count)
      return true;
  return false;
}

LRESULT CALLBACK EditSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData) {
  
  switch (uMsg) {

  case WM_DESTROY:
    delete (LPBOOL)dwRefData;
    if (HWND hDlg = GetFindDlg(GetParent(hWnd)))
      EndDialog(hDlg, 0LL);
    break;

  case WM_KEYDOWN:
    if (!(GetKeyState(VK_CONTROL) & 0x8000))
      break;
    switch (wParam) {
    case L'A':
      SendMessage(hWnd, EM_SETSEL, 0ULL, -1LL);
      *(LPBOOL)dwRefData = TRUE;
      break;
    case L'F': {
      HWND hWndParent = GetParent(hWnd);
      auto data = (LPUSERDATA)GetWindowLongPtr(hWndParent, GWLP_USERDATA);
      if (data->hDlg) {
        if (data->hEdit != hWnd) {
          data->hEdit = hWnd;
          ZeroMemory(data->req, sizeof(data->req));
          SetWindowText(FindWindowEx(data->hDlg, nullptr, WC_EDIT, nullptr), data->req);
          SetWindowText(data->hDlg, hWnd == GetDlgItem(hWndParent, DataEdit) ?
            L"Find - Data field" : L"Find - Access message field");
        }
        SetForegroundWindow(data->hDlg);
      }
      else {
        data->hEdit = hWnd;
        data->hDlg = FindText(new FINDREPLACE {
          sizeof(FINDREPLACE), hWndParent,
          nullptr, FR_DOWN | FR_HIDEWHOLEWORD,
          data->req, nullptr, 256ui16, 0ui16, 0LL,
          nullptr, nullptr
        });
        SetWindowText(data->hDlg, hWnd == GetDlgItem(hWndParent, DataEdit) ?
          L"Find - Data field" : L"Find - Access message");
      }
      *(LPBOOL)dwRefData = TRUE;
      break;
    }
    }
    break;

  case WM_CHAR: {
    auto ctrl = (LPBOOL)dwRefData;
    if (*ctrl) {
      *ctrl = FALSE;
      return 0LL;
    }
    break;
  }

  }

  return DefSubclassProc(hWnd, uMsg, wParam, lParam);

}
