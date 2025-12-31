#include <QApplication>
#include <QSharedPointer>
#include "MainWindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("VaultArchive");
    app.setApplicationVersion("0.3.27");
    app.setOrganizationName("LotusOsCore");
    app.setOrganizationDomain("https://lotuschain.org");
    
    // Set application-wide stylesheet
    app.setStyleSheet(R"(
        QMainWindow {
            background-color: #f5f5f5;
        }
        QMenuBar {
            background-color: #e0e0e0;
            border-bottom: 1px solid #ccc;
        }
        QMenuBar::item:selected {
            background-color: #d0d0d0;
        }
        QMenu {
            background-color: #f5f5f5;
            border: 1px solid #ccc;
        }
        QMenu::item:selected {
            background-color: #0078d7;
            color: white;
        }
        QStatusBar {
            background-color: #e0e0e0;
        }
        QTableWidget {
            background-color: white;
            border: 1px solid #ccc;
            gridline-color: #e0e0e0;
        }
        QHeaderView::section {
            background-color: #f0f0f0;
            padding: 4px;
            border: 1px solid #ccc;
        }
        QToolBar {
            background-color: #f5f5f5;
            border-bottom: 1px solid #ccc;
            spacing: 4px;
            padding: 2px;
        }
        QPushButton {
            min-width: 80px;
            padding: 6px 12px;
            border-radius: 4px;
            background-color: #0078d7;
            color: white;
            border: none;
        }
        QPushButton:hover {
            background-color: #106ebe;
        }
        QPushButton:pressed {
            background-color: #005a9e;
        }
        QPushButton:disabled {
            background-color: #cccccc;
            color: #888888;
        }
        QLineEdit {
            padding: 6px;
            border: 1px solid #ccc;
            border-radius: 4px;
        }
        QLineEdit:focus {
            border-color: #0078d7;
        }
        QLabel {
            color: #333;
        }
        QGroupBox {
            font-weight: bold;
            border: 1px solid #ccc;
            border-radius: 4px;
            margin-top: 10px;
            padding-top: 10px;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 10px;
            padding: 0 5px;
        }
        QProgressBar {
            border: 1px solid #ccc;
            border-radius: 4px;
            text-align: center;
        }
        QProgressBar::chunk {
            background-color: #0078d7;
        }
        QMessageBox {
            background-color: white;
        }
    )");

    MainWindow window;
    window.show();

    return app.exec();
}
