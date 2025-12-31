#include "AboutDialog.h"
#include "ui_AboutDialog.h"

AboutDialog::AboutDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::AboutDialog)
{
    ui->setupUi(this);

    // Set version information
    ui->versionLabel->setText(tr("Version 0.3.27"));

    // Set copyright
    ui->copyrightLabel->setText(tr("Copyright © 2025 LotusOS Core"));

    // Set description
    ui->descriptionLabel->setText(tr(
        "VaultArchive is a secure file archiving utility that provides:\n"
        "• AES-256-CBC encryption for data security\n"
        "• Zlib compression for efficient storage\n"
        "• SHA-256 integrity verification\n"
        "• Multi-file archive support\n"
        "• Both command-line and graphical interfaces"
    ));

    // Set license
    ui->licenseLabel->setText(tr(
        "Permission is hereby granted, free of charge, to any person obtaining a copy\n"
        "of this software and associated documentation files, to deal in the Software\n"
        "without restriction, including without limitation the rights to use, copy,\n"
        "modify, merge, publish, distribute, sublicense, and/or sell copies of the\n"
        "Software, and to permit persons to whom the Software is furnished to do so."
    ));

    // Set style for better appearance
    setStyleSheet(R"(
        AboutDialog {
            background-color: white;
        }
        QLabel#iconLabel {
            min-width: 64px;
            min-height: 64px;
        }
        QLabel#titleLabel {
            font-size: 18px;
            font-weight: bold;
            color: #0078d7;
        }
        QLabel#versionLabel {
            color: #666;
        }
        QLabel#copyrightLabel {
            color: #888;
            font-size: 11px;
        }
        QTextEdit {
            background-color: #f5f5f5;
            border: 1px solid #ddd;
            border-radius: 4px;
        }
    )");

    // Make dialog resizable but with reasonable minimum
    setMinimumSize(450, 350);
    resize(500, 400);
}

AboutDialog::~AboutDialog()
{
    delete ui;
}
