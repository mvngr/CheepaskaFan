#include "tcpconnection.h"

TcpConnection::TcpConnection(QObject *parent, QPlainTextEdit *log) : QObject(parent)
{
    log_ = log; //для вывода ui
    socket_ = new QTcpSocket(this);
    connect(socket_, &QTcpSocket::readyRead, this, &TcpConnection::readyRead); //коннект на обработчик приёма
}

/*!
 * \brief Выполняет подключение к хосту
 * \param host Хост, к которому будем подключаться
 * \return удачно ли подключились
 */
bool TcpConnection::connectToHost(const QString &host)
{
    socket_->connectToHost(host, port_);
    bool res = socket_->waitForConnected(5000);
    if(!res)
        qDebug() << "Not connected!";
    return res;
}
/*!
 * \brief Метод посылает пакет входа под гостевым пользователем
 * \return удачно ли выполнено отправление
 */
bool TcpConnection::registerAccount()
{
    return writeMessage("sign_in#guest#\n");
}

bool TcpConnection::writeMessage(const QString &message)
{
    if(socket_->state() == QAbstractSocket::ConnectedState)
    {
        socket_->write(message.toLocal8Bit());
        socket_->flush();
        log_->appendPlainText("Sent: " + message);
        return socket_->waitForBytesWritten();
    }
    else
    {
        qDebug() << QString("The '%1' message was not sent!").arg(message);
        return false;
    }
}

/*!
 * \brief Обработчик приходящих tcp пакетов
 */
void TcpConnection::readyRead()
{
    while (!socket_->atEnd()) {
        QByteArray data = socket_->readAll();
        log_->appendPlainText("Recieved: " + data);
        QString string(QString::fromStdString(data.toStdString()));
        if(!heyStepChecked_)
            heyStep(string);
        if(!requestTimeChecked_)
            requestTimeStep(string);
        matchFoundStep(string);
        yourTurnStep(string);
        winStep(string);

    }
}

/*!
 * \brief Обработчик принятого имени от сервера
 * \param str строка с полученным именем гостя
 * \return удачно ли выполнился шаг
 */
bool TcpConnection::heyStep(const QString &str)
{
    if(str.contains(QRegExp("^hey#")))
    {
        writeMessage("sync_time#\n"); //посылаем синхронизацию времени
        heyStepChecked_ = true;
        return true;
    }
    else
    {
        return false;
    }
}

/*!
 * \brief Обработчик отсинхронизированного времени
 * \param str принятое отсинхронизированное время
 * \return удачно ли выполнена обработка шага
 */
bool TcpConnection::requestTimeStep(const QString &str)
{
    if(str.contains(QRegExp("^sync_time#")))
    {
        writeMessage("request_play\n");
        requestTimeChecked_ = true;
        return true;
    }
    else
    {
        return false;
    }
}

/*!
 * \brief Обработчик найденной игры
 * \param str строка от сервера по информации об игре
 * \return удачно ли обработано
 */
bool TcpConnection::matchFoundStep(const QString &str)
{
    //match_found#ranking#0.6 0.28 0 r,0.28 0.5 1 r,0.47 0.48 2 r,0.6 0.14 3 r,0.37 0.23 4 r,0.47 0.34 5 r,0.85 0.43 6 r,0.78 0.27 7 r,0.85 0.13 8 r,0.15 0.29 9 r,0.32 0.38 10 r,0.67 0.82 11 g,0.85 0.91 12 g,0.15 1.07 13 g,0.22 0.93 14 g,0.15 0.77 15 g,0.53 0.86 16 g,0.63 0.97 17 g,0.4 1.06 18 g,0.53 0.72 19 g,0.72 0.7 20 g,0.4 0.92 21 g,0.5 0.6 22 g,#r g#Funtame R#g#true#10#def def#-\null
    if(str.contains(QRegExp("^match_found#")))
    {
        if(!log_)
        {
            qDebug() << "LOG NOT FOUND!";
            return false;
        }

        /*!
         * \brief последовательность данных от севрера
         *
         * \example
         * match_found
         * ranking
         * 0.6 0.28 0 r,0.28 0.5 1 r,0.47 0.48 2 r,0.6 0.14 3 r,0.37 0.23 4 r,0.47 0.34 5 r,0.85 0.43 6 r,0.78 0.27 7 r,0.85 0.13 8 r,0.15 0.29 9 r,0.32 0.38 10 r,0.67 0.82 11 g,0.85 0.91 12 g,0.15 1.07 13 g,0.22 0.93 14 g,0.15 0.77 15 g,0.53 0.86 16 g,0.63 0.97 17 g,0.4 1.06 18 g,0.53 0.72 19 g,0.72 0.7 20 g,0.4 0.92 21 g,0.5 0.6 22 g,
         * r g
         * Funtame R
         * g
         * true
         * 10
         * def def
         * -\null
         */
        QStringList commands = str.split("#"); //получаем список данных

        if(commands.at(1) == QString("ranking")) //пока сделал обрбаотчик только для рангового, для поединка наверное ничего сложного
        {
            pointsList_ = commands.at(2).split(",");
            colour_ = commands.at(5);

        }


        return true;
    }
    else
    {
        return false;
    }
}

/*!
 * \brief Обработчик сообщения о том, что я должен ходить
 * \param str соответсвующее сообщение
 * \return удачно ли обработан шаг
 */
bool TcpConnection::yourTurnStep(const QString &str)
{
    if(str.contains(QRegExp("^your_turn#")))
    {
        //находим первую попавшуюся нашу точку
        auto found = std::find_if(pointsList_.cbegin(), pointsList_.cend(), [this](const QString &str){ return (str.at(str.size() - 1) == colour_.at(0)); });
        if(found == pointsList_.cend())
        {
            qDebug() << "NOT FOUND POINT WITH COLOR!";
            return false;
        }
        else
        {
            QStringList whiteList; //создаем список только наших точек
            //Чистим шашки от шашек противника
            for(int i = 0; i < pointsList_.size(); i++) //проходимся циклом по всем точкам
            {
                QString selectedPoint = pointsList_.at(i);
                if(!selectedPoint.isEmpty() && selectedPoint.at(selectedPoint.size() - 1) == colour_.at(0)) //если цвет точки соответсвует нашему
                {
                    QStringList selectedPointData = selectedPoint.split(" "); //получаем данные
                    //так как тут проблема в том, что изначально нам приходят данные координата координата номер точки цвет
                    //а нужно в итоге отдать в sync_data номер точки координата координата цвет
                    whiteList.push_back(selectedPointData[2] + " " + selectedPointData[0] + " " + selectedPointData[1] + " " + selectedPointData[3]);
                    //создаем отформатированный список только наших точек в правильном формате для sync_data
                }
            }

            QString point = *found; //та самая первая найденная точка
            QStringList pointData = point.split(" ");
            QString pointNum = pointData.at(2); //получаем ее номер
            writeMessage(pointNum + " 0.0168 5.361592653589794 0.10101\n"); //и делаем чисто формальный запуск неважно в какую сторону

            writeMessage(QString("sync_data#") + whiteList.join(",") + "\n"); //а теперь сихроним дату, где остались только наши точки
            //причем тут можно постоянно отправлять одни и те же данные, он не ругается
            return true;
        }
    }
    else
    {
        return false;
    }
}

/*!
 * \brief Обработчик шага победы
 * \param str искомая строка о победе
 * \return удачно ли обработан шаг
 */
bool TcpConnection::winStep(const QString &str)
{
    //тут проблема в том, что бывают разные ситууации
    if(str.contains(QRegExp("^win#")) || str.contains(QRegExp("^money_update#")) || str.contains(QRegExp("^experience#")))
    {
        //если мы получили хоть одно из сообщений, которое символизирует конец игры
        writeMessage("request_play\n"); //начинаем новую и уходим в цикл
        return true;
    }
    else
    {
        return false;
    }
}
