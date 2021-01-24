#ifndef TCPCONNECTION_H
#define TCPCONNECTION_H

#include <QObject>
#include <QTcpSocket>
#include <QPlainTextEdit>

class TcpConnection : public QObject
{
    Q_OBJECT
public:
    explicit TcpConnection(QObject *parent = nullptr, QPlainTextEdit *log = nullptr);

public slots:

    bool connectToHost(const QString &host);
    void readyRead();
    bool registerAccount();
    bool writeMessage(const QString &message);

private:
    bool heyStepChecked_ = false;
    bool heyStep(const QString &str);

    bool requestTimeChecked_ = false;
    bool requestTimeStep(const QString &str);

    bool matchFoundStep(const QString &str);
    bool yourTurnStep(const QString &str);
    bool winStep(const QString &str);

    QPlainTextEdit *log_;
    QTcpSocket *socket_;
    quint16 port_ = 53697;

    QStringList pointsList_;
    QString colour_;
};

#endif // TCPCONNECTION_H
