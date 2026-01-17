#pragma once
#include <windows.h>
#include <shlobj.h>
#include <vector>
#include <string>
#include <iostream>

/*
 *
 * 该文件尚未使用
 *
 */

inline bool CopyFileToClipboard(const std::wstring &filePath) {
    // 打开剪贴板
    if (!OpenClipboard(NULL)) {
        std::cerr << "无法打开剪贴板" << std::endl;
        return false;
    }

    // 清空剪贴板
    EmptyClipboard();

    // 创建DROPFILES结构
    size_t pathSize = (filePath.length() + 2) * sizeof(wchar_t); // +2 用于双null终止
    size_t totalSize = sizeof(DROPFILES) + pathSize;

    HGLOBAL hGlobal = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, totalSize);
    if (!hGlobal) {
        CloseClipboard();
        return false;
    }

    DROPFILES *pDropFiles = (DROPFILES *) GlobalLock(hGlobal);
    if (!pDropFiles) {
        GlobalFree(hGlobal);
        CloseClipboard();
        return false;
    }

    // 设置DROPFILES结构
    pDropFiles->pFiles = sizeof(DROPFILES); // 偏移量
    pDropFiles->pt.x = 0;
    pDropFiles->pt.y = 0;
    pDropFiles->fNC = FALSE;
    pDropFiles->fWide = TRUE; // 使用Unicode

    // 复制文件路径
    wchar_t *pData = (wchar_t *) ((BYTE *) pDropFiles + sizeof(DROPFILES));
    wcscpy_s(pData, filePath.length() + 1, filePath.c_str());
    pData[filePath.length() + 1] = L'\0'; // 双null终止

    GlobalUnlock(hGlobal);

    // 设置剪贴板数据
    SetClipboardData(CF_HDROP, hGlobal);

    CloseClipboard();
    return true;
}

void SysCopyFile(QString &filePath) {
    // Windows系统：使用clip命令
#ifdef Q_OS_WINDOWS
    QString command = QString("echo %1 | clip").arg(filePath);
    system(command.toStdString().c_str());

    // Linux系统（需要xclip或xsel）
#elif defined(Q_OS_LINUX)
    QString command = QString("echo -n \"%1\" | xclip -selection clipboard").arg(filePath);
    system(command.toStdString().c_str());
    // macOS
#elif defined(Q_OS_MACOS)
    QString command = QString("echo \"%1\" | pbcopy").arg(filePath);
    system(command.toStdString().c_str());

    // 使用Qt方法作为回退
#else
    QMimeData *mimeData = new QMimeData();
    mimeData->setUrls({QUrl::fromLocalFile(filePath)});
    QApplication::clipboard()->setMimeData(mimeData);
#endif

    qDebug() << "已复制文件路径到剪贴板:" << filePath;
    TrayIcon::showMessage("Info", "File path copied: " + QFileInfo(filePath).fileName());
}


// int main() {
//     std::wstring filePath = L"C:\\Users\\Example\\test.txt";
//     if (CopyFileToClipboard(filePath)) {
//         std::cout << "文件已复制到剪贴板" << std::endl;
//     } else {
//         std::cout << "复制失败" << std::endl;
//     }
//
//     return 0;
// }
