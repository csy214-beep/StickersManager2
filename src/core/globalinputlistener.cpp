#include "globalinputlistener.h"
#include <QDebug>

GlobalInputListener *GlobalInputListener::instance = nullptr;

GlobalInputListener::GlobalInputListener(QObject *parent)
    : QObject(parent), keyboardHook(nullptr) {
    instance = this;
}

GlobalInputListener::~GlobalInputListener() {
    stopListening();
    instance = nullptr;
}

bool GlobalInputListener::startListening() {
    if (keyboardHook)
        return true;

    keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, keyboardHookProc, GetModuleHandle(nullptr), 0);
    if (!keyboardHook) {
        qWarning() << "Failed to install keyboard hook:" << GetLastError();
        return false;
    }

    qDebug() << "Global input listening started";
    return true;
}

void GlobalInputListener::stopListening() {
    if (keyboardHook) {
        UnhookWindowsHookEx(keyboardHook);
        keyboardHook = nullptr;
    }
    qDebug() << "Global input listening stopped";
}

LRESULT CALLBACK GlobalInputListener::keyboardHookProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode >= 0 && instance) {
        KBDLLHOOKSTRUCT *kbStruct = reinterpret_cast<KBDLLHOOKSTRUCT *>(lParam);

        if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
            ModifierKeys modifiers = NoModifier;
            if (GetAsyncKeyState(VK_SHIFT) & 0x8000)
                modifiers |= ShiftModifier;
            if (GetAsyncKeyState(VK_CONTROL) & 0x8000)
                modifiers |= ControlModifier;
            if (GetAsyncKeyState(VK_MENU) & 0x8000)
                modifiers |= AltModifier;
            if (GetAsyncKeyState(VK_LWIN) & 0x8000 || GetAsyncKeyState(VK_RWIN) & 0x8000)
                modifiers |= MetaModifier;

            emit instance->keyReleased(kbStruct->vkCode, modifiers);
        }
    }

    return CallNextHookEx(nullptr, nCode, wParam, lParam);
}
