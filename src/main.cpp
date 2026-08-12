#include <QApplication>
#include <QFile>
#include <QIcon>
#include <QTextStream>

#include "ui/MainWindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setOrganizationName("ELFOULADH");
    app.setApplicationName("InvoiceExtractor");
    app.setWindowIcon(QIcon(QStringLiteral(":/logo.png")));

    // The reference screens read right-to-left (Arabic labels, French field
    // text) — mirror the whole UI so the sidebar sits on the right, matching
    // the original design.
    app.setLayoutDirection(Qt::RightToLeft);

    QFile styleFile(":/resources/style.qss");
    if (styleFile.open(QFile::ReadOnly | QFile::Text)) {
        QTextStream stream(&styleFile);
        app.setStyleSheet(stream.readAll());
    }

    MainWindow window;
    window.show();

    return app.exec();
}
