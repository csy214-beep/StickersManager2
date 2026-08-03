#pragma once

#include <QObject>
#include <windows.h>

enum ModifierKey {
    NoModifier = 0x00,
    ShiftModifier = 0x01,
    ControlModifier = 0x02,
    AltModifier = 0x04,
    MetaModifier = 0x08
};

Q_DECLARE_FLAGS(ModifierKeys, ModifierKey)
Q_DECLARE_OPERATORS_FOR_FLAGS(ModifierKeys)

class GlobalInputListener : public QObject {
    Q_OBJECT

public:
    explicit GlobalInputListener(QObject *parent = nullptr);
    ~GlobalInputListener();

    bool startListening();
    void stopListening();
    bool isListening() const { return keyboardHook != nullptr; }

signals:
    void keyReleased(int keyCode, ModifierKeys modifiers);

private:
    static LRESULT CALLBACK keyboardHookProc(int nCode, WPARAM wParam, LPARAM lParam);
    HHOOK keyboardHook;
    static GlobalInputListener *instance;
};
