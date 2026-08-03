#include "globalinputlistener.h"
#include <QDebug>

QList<GlobalInputListener *> GlobalInputListener::instances;
HHOOK GlobalInputListener::sharedHook = nullptr;

GlobalInputListener::GlobalInputListener(QObject *parent)
    : QObject(parent), keyboardHook(nullptr) {
    instances.append(this);
}

GlobalInputListener::~GlobalInputListener() {
    instances.removeOne(this);
    stopListening();
}

bool GlobalInputListener::startListening() {
    if (keyboardHook)
        return true;

    if (!sharedHook) {
        sharedHook = SetWindowsHookEx(WH_KEYBOARD_LL, keyboardHookProc, GetModuleHandle(nullptr), 0);
        if (!sharedHook) {
            qWarning() << "Failed to install keyboard hook:" << GetLastError();
            return false;
        }
        qDebug() << "Global input listening started";
    }

    keyboardHook = sharedHook;
    return true;
}

void GlobalInputListener::stopListening() {
    if (!keyboardHook)
        return;

    keyboardHook = nullptr;
    bool anyActive = false;
    for (GlobalInputListener *listener : instances)
        if (listener->keyboardHook) {
            anyActive = true;
            break;
        }
    if (!anyActive && sharedHook) {
        UnhookWindowsHookEx(sharedHook);
        sharedHook = nullptr;
        qDebug() << "Global input listening stopped";
    }
}

LRESULT CALLBACK GlobalInputListener::keyboardHookProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode >= 0) {
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

            for (GlobalInputListener *listener : instances)
                if (listener->keyboardHook)
                    emit listener->keyReleased(kbStruct->vkCode, modifiers);
        }
    }

    return CallNextHookEx(nullptr, nCode, wParam, lParam);
}
