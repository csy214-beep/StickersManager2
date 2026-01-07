#pragma once
#include <QMessageBox>
#include <QtNetwork/QTcpSocket>
#include <QtNetwork/QTcpServer>

class PortListener : public QObject {
    Q_OBJECT

private:
    QTcpServer *m_server; // 存储服务器实例

public:
    explicit PortListener(QObject *parent = nullptr) : QObject(parent), m_server(nullptr) {
    }

    ~PortListener() {
        if (m_server) {
            m_server->close();
            delete m_server;
        }
    }

    // 非静态方法，用于检查单实例并设置监听
    bool checkSingleInstance(int port) {
        QTcpSocket singleInstanceSocket;
        singleInstanceSocket.connectToHost("127.0.0.1", port);

        if (singleInstanceSocket.waitForConnected(100)) {
            QMessageBox::warning(nullptr, "Warning",
                                 "端口 " + QString::number(port) +
                                 " 正在使用，可能已经有实例在运行，或者请更改端口。");
            return false;
        }
        m_server = new QTcpServer(this);
        if (!m_server->listen(QHostAddress::LocalHost, port)) {
            QMessageBox::critical(nullptr, "Error",
                                  "无法监听端口 " + QString::number(port));
            delete m_server;
            m_server = nullptr;
            return false;
        }

        return true;
    }

    // 可选：获取服务器实例
    QTcpServer *server() const { return m_server; }
};
