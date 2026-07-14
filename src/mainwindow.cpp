#include "mainwindow.h"

#include <QApplication>
#include <QClipboard>
#include <QComboBox>
#include <QDesktopServices>
#include <QDir>
#include <QFileInfo>
#include <QFormLayout>
#include <QFont>
#include <QGroupBox>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QProcess>
#include <QPushButton>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QStyleHints>
#include <QTextEdit>
#include <QUrl>
#include <QVBoxLayout>
#include <QWidget>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setupUi();
    applyStyles();
    setWindowTitle("磁力連結產生器 (Magnet Link Generator)");
    resize(760, 820);

    // 系統色彩主題（淺色/深色）變更時即時套用對應樣式
    connect(QGuiApplication::styleHints(), &QStyleHints::colorSchemeChanged,
            this, [this](Qt::ColorScheme) { applyStyles(); });
}

void MainWindow::setupUi()
{
    auto *central = new QWidget(this);
    central->setObjectName("centralWidget");
    setCentralWidget(central);

    auto *mainLayout = new QVBoxLayout(central);
    mainLayout->setSpacing(16);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    // ---------- 標題 ----------
    auto *titleLabel = new QLabel("🧲  磁力連結產生器", central);
    titleLabel->setObjectName("titleLabel");
    auto *subtitleLabel = new QLabel("填寫欄位後一鍵產生標準 magnet 連結", central);
    subtitleLabel->setObjectName("subtitleLabel");
    mainLayout->addWidget(titleLabel);
    mainLayout->addWidget(subtitleLabel);

    // ---------- 必填欄位群組 ----------
    auto *requiredGroup = new QGroupBox("必填欄位", central);
    auto *requiredForm = new QFormLayout(requiredGroup);
    requiredForm->setSpacing(10);

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
    optionalForm->setSpacing(10);

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

    auto *trackerHeaderLayout = new QHBoxLayout();
    auto *trackerLabel = new QLabel("追蹤器清單 (tr)：", optionalGroup);
    m_loadTrackersButton = new QPushButton("＋ 載入常用 20 個追蹤器", optionalGroup);
    m_loadTrackersButton->setObjectName("secondaryButton");
    m_loadTrackersButton->setCursor(Qt::PointingHandCursor);
    trackerHeaderLayout->addWidget(trackerLabel);
    trackerHeaderLayout->addStretch();
    trackerHeaderLayout->addWidget(m_loadTrackersButton);

    m_trackersEdit = new QPlainTextEdit(optionalGroup);
    m_trackersEdit->setPlaceholderText(
        "每行輸入一個 Tracker 網址，例如：\n"
        "udp://tracker.opentrackr.org:1337/announce\n"
        "udp://open.stealth.si:80/announce\n\n"
        "也可以按右上角按鈕自動載入常用的 20 個公開追蹤器。");
    m_trackersEdit->setFixedHeight(150);
    optionalForm->addRow(trackerHeaderLayout);
    optionalForm->addRow(m_trackersEdit);

    optionalGroup->setLayout(optionalForm);
    mainLayout->addWidget(optionalGroup);

    // ---------- 按鈕列 ----------
    auto *buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(10);

    m_generateButton = new QPushButton("⚡ 產生磁力連結", central);
    m_generateButton->setObjectName("primaryButton");
    m_copyButton = new QPushButton("📋 複製", central);
    m_copyButton->setObjectName("secondaryButton");
    m_testButton = new QPushButton("🚀 測試 (以 qBittorrent 開啟)", central);
    m_testButton->setObjectName("secondaryButton");
    m_clearButton = new QPushButton("🗑 清除", central);
    m_clearButton->setObjectName("dangerButton");

    for (auto *btn : {m_generateButton, m_copyButton, m_testButton, m_clearButton})
        btn->setCursor(Qt::PointingHandCursor);

    buttonLayout->addWidget(m_generateButton, 2);
    buttonLayout->addWidget(m_copyButton, 1);
    buttonLayout->addWidget(m_testButton, 2);
    buttonLayout->addWidget(m_clearButton, 1);
    mainLayout->addLayout(buttonLayout);

    // ---------- 結果區 ----------
    auto *resultGroup = new QGroupBox("產生結果", central);
    auto *resultLayout = new QVBoxLayout(resultGroup);
    m_resultEdit = new QTextEdit(resultGroup);
    m_resultEdit->setObjectName("resultEdit");
    m_resultEdit->setReadOnly(true);
    m_resultEdit->setPlaceholderText("按下「產生磁力連結」後，結果會顯示於此處。");
    m_resultEdit->setMinimumHeight(110);
    QFont monoFont("Consolas");
    monoFont.setStyleHint(QFont::Monospace);
    monoFont.setPointSize(10);
    m_resultEdit->setFont(monoFont);
    resultLayout->addWidget(m_resultEdit);
    resultGroup->setLayout(resultLayout);
    mainLayout->addWidget(resultGroup);

    m_statusLabel = new QLabel(" ", central);
    m_statusLabel->setObjectName("statusLabel");
    mainLayout->addWidget(m_statusLabel);

    connect(m_generateButton, &QPushButton::clicked, this, &MainWindow::generateMagnetLink);
    connect(m_copyButton, &QPushButton::clicked, this, &MainWindow::copyToClipboard);
    connect(m_clearButton, &QPushButton::clicked, this, &MainWindow::clearFields);
    connect(m_loadTrackersButton, &QPushButton::clicked, this, &MainWindow::loadBuiltInTrackers);
    connect(m_testButton, &QPushButton::clicked, this, &MainWindow::testWithQBittorrent);
}

void MainWindow::applyStyles()
{
    const bool isDark = QGuiApplication::styleHints()->colorScheme() == Qt::ColorScheme::Dark;

    if (isDark) {
        setStyleSheet(R"(
            #centralWidget {
                background-color: #1b2030;
            }
            #titleLabel {
                font-size: 22px;
                font-weight: 700;
                color: #f5f7fb;
            }
            #subtitleLabel {
                font-size: 12px;
                color: #9aa4bf;
                margin-bottom: 4px;
            }
            QGroupBox {
                background-color: #232838;
                border: 1px solid #333a4d;
                border-radius: 10px;
                margin-top: 14px;
                padding: 14px;
                font-weight: 600;
                color: #c7cede;
            }
            QGroupBox::title {
                subcontrol-origin: margin;
                left: 12px;
                padding: 0 6px;
                color: #c7cede;
            }
            QLabel {
                color: #dfe3ee;
                font-size: 12px;
            }
            QLineEdit, QPlainTextEdit, QComboBox, QTextEdit {
                background-color: #2a2f42;
                border: 1px solid #3a4059;
                border-radius: 6px;
                padding: 6px 8px;
                font-size: 12px;
                color: #eef1fa;
                selection-background-color: #4a63f0;
            }
            QLineEdit:focus, QPlainTextEdit:focus, QComboBox:focus, QTextEdit:focus {
                border: 1px solid #6c8cff;
            }
            QComboBox QAbstractItemView {
                background-color: #2a2f42;
                color: #eef1fa;
                selection-background-color: #4a63f0;
            }
            #resultEdit {
                background-color: #0b111d;
                color: #7ce38b;
                border-radius: 8px;
                border: 1px solid #333a4d;
            }
            QPushButton {
                border-radius: 8px;
                padding: 9px 14px;
                font-size: 12px;
                font-weight: 600;
            }
            #primaryButton {
                background-color: #4a63f0;
                color: #ffffff;
                border: none;
            }
            #primaryButton:hover {
                background-color: #5a72ff;
            }
            #primaryButton:pressed {
                background-color: #3d52d1;
            }
            #secondaryButton {
                background-color: #2f3448;
                color: #dfe3ee;
                border: 1px solid #3a4059;
            }
            #secondaryButton:hover {
                background-color: #383e56;
            }
            #dangerButton {
                background-color: #3a2020;
                color: #ff8f8f;
                border: 1px solid #5c2b2b;
            }
            #dangerButton:hover {
                background-color: #482727;
            }
            #statusLabel {
                color: #9aa4bf;
                font-size: 11px;
                padding-top: 2px;
            }
        )");
    } else {
        setStyleSheet(R"(
            #centralWidget {
                background-color: #f4f6fb;
            }
            #titleLabel {
                font-size: 22px;
                font-weight: 700;
                color: #1f2a44;
            }
            #subtitleLabel {
                font-size: 12px;
                color: #6b7280;
                margin-bottom: 4px;
            }
            QGroupBox {
                background-color: #ffffff;
                border: 1px solid #e2e5ec;
                border-radius: 10px;
                margin-top: 14px;
                padding: 14px;
                font-weight: 600;
                color: #33415c;
            }
            QGroupBox::title {
                subcontrol-origin: margin;
                left: 12px;
                padding: 0 6px;
                color: #4f5b76;
            }
            QLabel {
                color: #33415c;
                font-size: 12px;
            }
            QLineEdit, QPlainTextEdit, QComboBox, QTextEdit {
                background-color: #fbfcfe;
                border: 1px solid #d7dce5;
                border-radius: 6px;
                padding: 6px 8px;
                font-size: 12px;
                color: #1f2a44;
                selection-background-color: #6c8cff;
            }
            QLineEdit:focus, QPlainTextEdit:focus, QComboBox:focus, QTextEdit:focus {
                border: 1px solid #6c8cff;
            }
            QComboBox QAbstractItemView {
                background-color: #ffffff;
                color: #1f2a44;
                selection-background-color: #6c8cff;
            }
            #resultEdit {
                background-color: #101826;
                color: #7ce38b;
                border-radius: 8px;
                border: 1px solid #1f2a44;
            }
            QPushButton {
                border-radius: 8px;
                padding: 9px 14px;
                font-size: 12px;
                font-weight: 600;
            }
            #primaryButton {
                background-color: #4a63f0;
                color: white;
                border: none;
            }
            #primaryButton:hover {
                background-color: #3d52d1;
            }
            #primaryButton:pressed {
                background-color: #33449f;
            }
            #secondaryButton {
                background-color: #eef1fa;
                color: #33415c;
                border: 1px solid #d7dce5;
            }
            #secondaryButton:hover {
                background-color: #e2e7f7;
            }
            #dangerButton {
                background-color: #fdeeee;
                color: #c23b3b;
                border: 1px solid #f4c9c9;
            }
            #dangerButton:hover {
                background-color: #fbdada;
            }
            #statusLabel {
                color: #6b7280;
                font-size: 11px;
                padding-top: 2px;
            }
        )");
    }
}

QStringList MainWindow::builtInTrackers()
{
    return {
        "udp://tracker.opentrackr.org:1337/announce",
        "udp://open.stealth.si:80/announce",
        "udp://exodus.desync.com:6969/announce",
        "udp://tracker.torrent.eu.org:451/announce",
        "udp://tracker.moeking.me:6969/announce",
        "udp://explodie.org:6969/announce",
        "udp://tracker.dler.org:6969/announce",
        "udp://opentracker.i2p.rocks:6969/announce",
        "udp://tracker.tiny-vps.com:6969/announce",
        "udp://tracker.theoks.net:6969/announce",
        "udp://tracker.bittor.pw:1337/announce",
        "udp://tracker.zerobytes.xyz:1337/announce",
        "udp://retracker.lanta-net.ru:2710/announce",
        "udp://tracker.army:6969/announce",
        "udp://tracker.leech.ie:1337/announce",
        "http://tracker.openbittorrent.com:80/announce",
        "udp://open.demonii.com:1337/announce",
        "udp://tracker.internetwarriors.net:1337/announce",
        "wss://tracker.btorrent.xyz",
        "udp://p4p.arenabg.com:1337/announce",
    };
}

void MainWindow::loadBuiltInTrackers()
{
    QStringList existing = m_trackersEdit->toPlainText().split('\n', Qt::SkipEmptyParts);
    for (QString &t : existing)
        t = t.trimmed();

    int added = 0;
    for (const QString &tracker : builtInTrackers()) {
        if (!existing.contains(tracker, Qt::CaseInsensitive)) {
            existing << tracker;
            ++added;
        }
    }

    m_trackersEdit->setPlainText(existing.join('\n'));
    m_statusLabel->setText(QString("已載入常用追蹤器，新增 %1 筆（共 %2 筆，已略過重複項目）。")
                                .arg(added)
                                .arg(existing.size()));
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

bool MainWindow::findQBittorrentExecutable(QString &outPath) const
{
    // 1. 先檢查 PATH 環境變數
    QString found = QStandardPaths::findExecutable("qbittorrent");
    if (found.isEmpty())
        found = QStandardPaths::findExecutable("qbittorrent.exe");

    if (!found.isEmpty()) {
        outPath = found;
        return true;
    }

    // 2. 檢查常見安裝路徑
    QStringList candidates;
#if defined(Q_OS_WIN)
    const QString pf = qEnvironmentVariable("ProgramFiles");
    const QString pf86 = qEnvironmentVariable("ProgramFiles(x86)");
    const QString localAppData = qEnvironmentVariable("LOCALAPPDATA");
    if (!pf.isEmpty())
        candidates << pf + "/qBittorrent/qbittorrent.exe";
    if (!pf86.isEmpty())
        candidates << pf86 + "/qBittorrent/qbittorrent.exe";
    if (!localAppData.isEmpty())
        candidates << localAppData + "/qBittorrent/qbittorrent.exe";
#elif defined(Q_OS_MACOS)
    candidates << "/Applications/qbittorrent.app/Contents/MacOS/qbittorrent";
#else
    candidates << "/usr/bin/qbittorrent"
               << "/usr/local/bin/qbittorrent"
               << "/snap/bin/qbittorrent"
               << "/var/lib/flatpak/exports/bin/org.qbittorrent.qBittorrent";
#endif

    for (const QString &path : candidates) {
        if (QFileInfo::exists(path)) {
            outPath = path;
            return true;
        }
    }

    return false;
}

void MainWindow::testWithQBittorrent()
{
    QString magnet = m_resultEdit->toPlainText().trimmed();

    if (magnet.isEmpty()) {
        // 尚未產生過，先嘗試自動產生一次
        generateMagnetLink();
        magnet = m_resultEdit->toPlainText().trimmed();
        if (magnet.isEmpty())
            return; // generateMagnetLink 已顯示錯誤訊息
    }

    QString qbtPath;
    if (findQBittorrentExecutable(qbtPath)) {
        const bool started = QProcess::startDetached(qbtPath, {magnet});
        if (started) {
            m_statusLabel->setText("已呼叫 qBittorrent 開啟磁力連結。");
        } else {
            QMessageBox::warning(this, "呼叫失敗", "找到 qBittorrent，但無法啟動程序，請確認檔案是否損壞或權限是否足夠。");
        }
        return;
    }

    // 找不到 qBittorrent，詢問是否前往下載
    const auto reply = QMessageBox::question(
        this, "找不到 qBittorrent",
        "在系統中找不到 qBittorrent，可能尚未安裝。\n是否要開啟官方下載頁面？",
        QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        QDesktopServices::openUrl(QUrl("https://www.qbittorrent.org/download"));
        m_statusLabel->setText("已開啟 qBittorrent 官方下載頁面。");
    } else {
        m_statusLabel->setText("已取消呼叫 qBittorrent。");
    }
}
