#ifndef HOTKEYCAPTURE_H
#define HOTKEYCAPTURE_H

#include <QPushButton>
#include <functional>

class GlobalInputListener;
class QKeyEvent;
class QHideEvent;

class HotkeyCaptureButton : public QPushButton {
    Q_OBJECT
public:
    explicit HotkeyCaptureButton(QWidget *parent = nullptr);
    ~HotkeyCaptureButton() override;

    QString hotkey() const { return m_hotkey; }
    void setHotkey(const QString &hotkey);
    bool isRecording() const { return m_recording; }
    void setConflict(bool conflict);
    void setConflictChecker(std::function<bool(const QString &)> checker) { m_conflictChecker = std::move(checker); }

signals:
    void hotkeyChanged();

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void hideEvent(QHideEvent *event) override;

private:
    void startRecording();
    void stopRecording();
    void refreshText();
    void applyConflictVisual();

    QString m_hotkey;
    bool m_recording = false;
    bool m_conflict = false;
    int m_lastHandledKey = 0;
    GlobalInputListener *m_listener = nullptr;
    std::function<bool(const QString &)> m_conflictChecker;
};

#endif // HOTKEYCAPTURE_H
