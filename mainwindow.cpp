#include "mainwindow.h"

#include <QApplication>
#include <QClipboard>
#include <QComboBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QRegularExpression>
#include <QTextEdit>
#include <QUrl>
#include <QVBoxLayout>
#include <QWidget>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setupUi();
    setWindowTitle("磁力連結產生器 (Magnet Link Generator)");
    resize(720, 720);
}

void MainWindow::setupUi()
{
    auto *central = new QWidget(this);
    setCentralWidget(central);

    auto *mainLayout = new QVBoxLayout(central);

    // ---------- 必填欄位群組 ----------
    auto *requiredGroup = new QGroupBox("必填欄位", central);
    auto *requiredForm = new QFormLayout(requiredGroup);

    m_hashTypeCombo = new QComboBox(requiredGroup);
    m_hashTypeCombo->addItem("BTIH - SHA-1（BitTorrent v1，最常見）", "btih");
    m_hashTypeCombo->addItem("BTMH - SHA-256（BitTorrent v2）", "btmh");
    requiredForm->addRow("雜湊類型 (xt)：", m_hashTypeCombo);

    m_hashEdit = new QLineEdit(requiredGroup);
    m_hashEdit->setPlaceholderText("例如：40 碼十六進位或 32 碼 Base32 的 Info Hash");
    requiredForm->addRow("雜湊值 (Hash)：*", m_hashEdit);

    requiredGroup->setLayout(requiredForm);
    mainLayout->addWidget(requiredGroup);

    // ---------- 選填欄位群組 ----------
    auto *optionalGroup = new QGroupBox("選填欄位", central);
    auto *optionalForm = new QFormLayout(optionalGroup);

    m_nameEdit = new QLineEdit(optionalGroup);
    m_nameEdit->setPlaceholderText("顯示於下載工具中的檔名");
    optionalForm->addRow("顯示名稱 (dn)：", m_nameEdit);

    m_sizeEdit = new QLineEdit(optionalGroup);
    m_sizeEdit->setPlaceholderText("檔案總大小，單位：位元組 (bytes)");
    optionalForm->addRow("檔案大小 (xl)：", m_sizeEdit);

    m_keywordsEdit = new QLineEdit(optionalGroup);
    m_keywordsEdit->setPlaceholderText("以逗號分隔多個關鍵字，例如：movie,2024,1080p");
    optionalForm->addRow("關鍵字 (kt)：", m_keywordsEdit);

    m_webSeedEdit = new QLineEdit(optionalGroup);
    m_webSeedEdit->setPlaceholderText("HTTP/FTP 網頁種子網址（選填）");
    optionalForm->addRow("網頁種子 (ws)：", m_webSeedEdit);

    m_trackersEdit = new QPlainTextEdit(optionalGroup);
    m_trackersEdit->setPlaceholderText(
        "每行輸入一個 Tracker 網址，例如：\n"
        "udp://tracker.opentrackr.org:1337/announce\n"
        "udp://open.stealth.si:80/announce");
    m_trackersEdit->setFixedHeight(110);
    optionalForm->addRow("追蹤器清單 (tr)：", m_trackersEdit);

    optionalGroup->setLayout(optionalForm);
    mainLayout->addWidget(optionalGroup);

    // ---------- 按鈕列 ----------
    auto *buttonLayout = new QHBoxLayout();
    m_generateButton = new QPushButton("產生磁力連結", central);
    m_copyButton = new QPushButton("複製到剪貼簿", central);
    m_clearButton = new QPushButton("清除所有欄位", central);
    buttonLayout->addWidget(m_generateButton);
    buttonLayout->addWidget(m_copyButton);
    buttonLayout->addWidget(m_clearButton);
    mainLayout->addLayout(buttonLayout);

    // ---------- 結果區 ----------
    auto *resultGroup = new QGroupBox("產生結果", central);
    auto *resultLayout = new QVBoxLayout(resultGroup);
    m_resultEdit = new QTextEdit(resultGroup);
    m_resultEdit->setReadOnly(true);
    m_resultEdit->setPlaceholderText("按下「產生磁力連結」後，結果會顯示於此處。");
    m_resultEdit->setMinimumHeight(120);
    resultLayout->addWidget(m_resultEdit);
    resultGroup->setLayout(resultLayout);
    mainLayout->addWidget(resultGroup);

    m_statusLabel = new QLabel(" ", central);
    m_statusLabel->setStyleSheet("color: gray;");
    mainLayout->addWidget(m_statusLabel);

    connect(m_generateButton, &QPushButton::clicked, this, &MainWindow::generateMagnetLink);
    connect(m_copyButton, &QPushButton::clicked, this, &MainWindow::copyToClipboard);
    connect(m_clearButton, &QPushButton::clicked, this, &MainWindow::clearFields);
}

QString MainWindow::percentEncode(const QString &text)
{
    // 對非常安全字元以外的所有字元做百分比編碼，符合磁力連結 URI 規範
    static const QByteArray unreserved = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_.~";
    return QString::fromLatin1(QUrl::toPercentEncoding(text, unreserved));
}

bool MainWindow::validateHash(const QString &hash) const
{
    const QString type = m_hashTypeCombo->currentData().toString();
    const QString trimmed = hash.trimmed();

    if (trimmed.isEmpty())
        return false;

    if (type == "btih") {
        // 40 碼十六進位 或 32 碼 Base32
        static const QRegularExpression hex40("^[A-Fa-f0-9]{40}$");
        static const QRegularExpression base32("^[A-Za-z2-7]{32}$");
        return hex40.match(trimmed).hasMatch() || base32.match(trimmed).hasMatch();
    }

    // btmh：以十六進位表示的多雜湊值，長度需為偶數且至少 4 碼（含前綴）
    static const QRegularExpression hexEven("^[A-Fa-f0-9]+$");
    return hexEven.match(trimmed).hasMatch() && trimmed.length() % 2 == 0 && trimmed.length() >= 4;
}

void MainWindow::generateMagnetLink()
{
    const QString hash = m_hashEdit->text().trimmed();

    if (hash.isEmpty()) {
        QMessageBox::warning(this, "缺少必填欄位", "請輸入雜湊值 (Hash)，這是產生磁力連結的必要欄位。");
        m_hashEdit->setFocus();
        return;
    }

    if (!validateHash(hash)) {
        const auto reply = QMessageBox::question(
            this, "雜湊值格式可能不正確",
            "輸入的雜湊值格式看起來不符合預期（BTIH 應為 40 碼十六進位或 32 碼 Base32）。\n"
            "是否仍要繼續產生磁力連結？",
            QMessageBox::Yes | QMessageBox::No);
        if (reply != QMessageBox::Yes)
            return;
    }

    const QString type = m_hashTypeCombo->currentData().toString();

    QString magnet = QStringLiteral("magnet:?xt=urn:%1:%2").arg(type, hash.trimmed());

    // dn - 顯示名稱
    if (const QString name = m_nameEdit->text().trimmed(); !name.isEmpty())
        magnet += "&dn=" + percentEncode(name);

    // xl - 檔案大小，僅接受正整數
    if (const QString sizeStr = m_sizeEdit->text().trimmed(); !sizeStr.isEmpty()) {
        bool ok = false;
        const qint64 size = sizeStr.toLongLong(&ok);
        if (ok && size > 0) {
            magnet += "&xl=" + QString::number(size);
        } else {
            QMessageBox::warning(this, "檔案大小格式錯誤", "檔案大小 (xl) 必須是正整數（單位：bytes），已略過此欄位。");
        }
    }

    // kt - 關鍵字，逗號分隔，以 '+' 連接
    if (const QString kw = m_keywordsEdit->text().trimmed(); !kw.isEmpty()) {
        const QStringList keywords = kw.split(',', Qt::SkipEmptyParts);
        QStringList encoded;
        for (const QString &k : keywords)
            encoded << percentEncode(k.trimmed());
        if (!encoded.isEmpty())
            magnet += "&kt=" + encoded.join('+');
    }

    // ws - 網頁種子
    if (const QString ws = m_webSeedEdit->text().trimmed(); !ws.isEmpty())
        magnet += "&ws=" + percentEncode(ws);

    // tr - 追蹤器，每行一個，可有多個
    const QStringList trackers = m_trackersEdit->toPlainText().split('\n', Qt::SkipEmptyParts);
    for (const QString &tracker : trackers) {
        const QString t = tracker.trimmed();
        if (!t.isEmpty())
            magnet += "&tr=" + percentEncode(t);
    }

    m_resultEdit->setPlainText(magnet);
    m_statusLabel->setText(QString("已產生磁力連結，共包含 %1 個追蹤器。").arg(trackers.size()));
}

void MainWindow::copyToClipboard()
{
    const QString text = m_resultEdit->toPlainText();
    if (text.isEmpty()) {
        QMessageBox::information(this, "沒有內容可複製", "請先產生磁力連結，再進行複製。");
        return;
    }
    QApplication::clipboard()->setText(text);
    m_statusLabel->setText("已複製到剪貼簿。");
}

void MainWindow::clearFields()
{
    m_hashEdit->clear();
    m_nameEdit->clear();
    m_sizeEdit->clear();
    m_keywordsEdit->clear();
    m_webSeedEdit->clear();
    m_trackersEdit->clear();
    m_resultEdit->clear();
    m_hashTypeCombo->setCurrentIndex(0);
    m_statusLabel->setText(" ");
}
