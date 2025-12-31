#include "ProgressDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>

ProgressDialog::ProgressDialog(const QString &title, const QString &label,
                               int minimum, int maximum, QWidget *parent)
    : QProgressDialog(label, tr("Cancel"), minimum, maximum, parent)
{
    setWindowTitle(title);
    setWindowModality(Qt::WindowModal);
    setMinimumSize(350, 120);
    setMaximumSize(500, 150);
    setSizeGripEnabled(false);

    setupUi(title, label);

    connect(this, &QProgressDialog::canceled, this, &ProgressDialog::canceled);
}

ProgressDialog::~ProgressDialog()
{
}

void ProgressDialog::setCurrentFile(const QString &filename)
{
    m_currentFileLabel->setText(tr("Processing: %1").arg(filename));
    QApplication::processEvents();
}

void ProgressDialog::setStatus(const QString &status)
{
    m_currentFileLabel->setText(status);
    QApplication::processEvents();
}

void ProgressDialog::setupUi(const QString &title, const QString &label)
{
    Q_UNUSED(title)

    // This is a simple wrapper around QProgressDialog
    // The label is already set by the constructor

    // We can customize further if needed
    setStyleSheet(R"(
        QProgressDialog {
            background-color: white;
        }
        QProgressBar {
            border: 1px solid #ccc;
            border-radius: 4px;
            text-align: center;
            min-height: 20px;
        }
        QProgressBar::chunk {
            background-color: #0078d7;
            border-radius: 3px;
        }
        QPushButton {
            min-width: 80px;
        }
    )");
}
