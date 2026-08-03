#include "hotkeycapture.h"
#include "globalinputlistener.h"
#include "convertcodetostring.hpp"

#include <QKeyEvent>

namespace {
constexpr int VK_ESCAPE_KEY = 27;
constexpr int VK_BACKSPACE_KEY = 8;
}

HotkeyCaptureButton::HotkeyCaptureButton(QWidget *parent)
    : QPushButton(parent)
{
    setToolTip("Click to record a hotkey (Esc: cancel, Backspace: clear)");
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    connect(this, &QPushButton::clicked, this, [this]() {
        if (m_recording)
            stopRecording();
        else
            startRecording();
    });
    refreshText();
}

HotkeyCaptureButton::~HotkeyCaptureButton()
{
    stopRecording();
}

void HotkeyCaptureButton::setHotkey(const QString &hotkey)
{
    m_hotkey = hotkey;
    m_conflict = m_conflictChecker ? m_conflictChecker(m_hotkey) : false;
    applyConflictVisual();
    refreshText();
    emit hotkeyChanged();
}

void HotkeyCaptureButton::setConflict(bool conflict)
{
    m_conflict = conflict;
    applyConflictVisual();
}

void HotkeyCaptureButton::startRecording()
{
    m_recording = true;
    setDown(true);
    refreshText();

    if (!m_listener)
        m_listener = new GlobalInputListener(this);
    disconnect(m_listener, nullptr, this, nullptr);
    connect(m_listener, &GlobalInputListener::keyReleased, this, [this](int keyCode, ModifierKeys modifiers) {
        if (!m_recording)
            return;

        QString keyName = keyCodeToKeyString(keyCode);
        if (keyName.isEmpty())
            return;

        if (keyCode == VK_ESCAPE_KEY) {
            m_lastHandledKey = keyCode;
            stopRecording();
            return;
        }
        if (keyCode == VK_BACKSPACE_KEY) {
            m_lastHandledKey = keyCode;
            stopRecording();
            setHotkey(QString());
            return;
        }

        QString modifierName = modifiersToString(modifiers);
        setHotkey(modifierName.isEmpty() ? keyName : modifierName + "+" + keyName);
        m_lastHandledKey = keyCode;
        stopRecording();
    });

    if (!m_listener->startListening())
        stopRecording();
}

void HotkeyCaptureButton::stopRecording()
{
    m_recording = false;
    setDown(false);
    if (m_listener)
        m_listener->stopListening();
    refreshText();
    applyConflictVisual();
}

void HotkeyCaptureButton::refreshText()
{
    if (m_recording)
        setText("Press hotkey... (Esc: cancel, Backspace: clear)");
    else if (m_hotkey.isEmpty())
        setText("None - click to set");
    else
        setText(m_hotkey);
}

void HotkeyCaptureButton::applyConflictVisual()
{
    if (m_conflict && !m_recording)
        setStyleSheet("color: #c0392b; border: 1px solid #c0392b;");
    else
        setStyleSheet(QString());
}

void HotkeyCaptureButton::keyPressEvent(QKeyEvent *event)
{
    if (m_lastHandledKey != 0) {
        if (event->nativeVirtualKey() == m_lastHandledKey || event->key() == Qt::Key_Escape) {
            m_lastHandledKey = 0;
            event->accept();
            return;
        }
    }
    if (m_recording) {
        event->accept();
        return;
    }
    if (event->key() == Qt::Key_Escape) {
        event->accept();
        return;
    }
    QPushButton::keyPressEvent(event);
}

void HotkeyCaptureButton::hideEvent(QHideEvent *event)
{
    stopRecording();
    QPushButton::hideEvent(event);
}
