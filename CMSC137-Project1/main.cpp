/*
Miguel Luis C. Sobrevinas
2012-25200
(asked for batchmate/classmate/roommate's help personally)
*/

#include "client.h"
#include "header.h"
#include "server.h"

#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDebug>
#include <iostream>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QCommandLineParser parser;
    parser.setApplicationDescription(" A Data Flow Control Layer Over UDP that Mimics the Transfer Control Protocol (TCP)");
    parser.addHelpOption();
    parser.addPositionalArgument("port", QCoreApplication::translate("main", "Port number"));

    // Option for listen
    QCommandLineOption listenOption("l", QCoreApplication::translate("main", "Specify that program shall listen rather than connect"));
    parser.addOption(listenOption);

    // Drop percent
    QCommandLineOption dropRateOption(QStringList() << "d" << "drop",QCoreApplication::translate("main", "Packet drop rate. 0=0%, 1=25%, 2=50%, 3=75%"), "rate");
    parser.addOption(dropRateOption);

    parser.process(a);

    quint16 port = 9999;

    if(parser.positionalArguments().length()==1){
        bool ok;
        quint16 pn = parser.positionalArguments().at(0).toUShort(&ok);
        if(ok) port=pn;
    }

    if(parser.isSet("l")){
        Server *s = new Server();

        if(parser.isSet("drop")){
            QString dr = parser.value(dropRateOption);
            bool ok;
            quint16 drr = dr.toUShort(&ok);
            if(ok&& drr<4){
                qDebug() << "Drop rate set to" << drr;
                s->setDrop(drr);
            }
        }

        s->listen(port);
    } else {
        Client *c = new Client();
        c->connectTo(QHostAddress::LocalHost, port);

        std::string msg;
        std::cout << "Enter message:";
        std::getline(std::cin, msg);

        c->sendData(QString::fromStdString(msg).toUtf8());
    }

    return a.exec();
}
