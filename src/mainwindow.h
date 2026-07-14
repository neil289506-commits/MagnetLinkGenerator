#pragma once

#include <QMainWindow>

QT_BEGIN_NAMESPACE
class QLineEdit;
class QPlainTextEdit;
class QComboBox;
class QTextEdit;
class QPushButton;
class QLabel;
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

private slots:
    void generateMagnetLink();
    void copyToClipboard();
    void clearFields();
    void loadBuiltInTrackers();
    void testWithQBittorrent();

private:
    void setupUi();
    void applyStyles();
    static QString percentEncode(const QString &text);
    bool validateHash(const QString &hash) const;
    static QStringList builtInTrackers();
    bool findQBittorrentExecutable(QString &outPath) const;

    QComboBox      *m_hashTypeCombo   = nullptr; // xt: btih (SHA-1) / btmh (SHA-256)
    QLineEdit      *m_hashEdit        = nullptr; // 雜湊值（必填）
    QLineEdit      *m_nameEdit        = nullptr; // dn: 顯示名稱
    QLineEdit      *m_sizeEdit        = nullptr; // xl: 檔案大小（bytes）
    QLineEdit      *m_keywordsEdit    = nullptr; // kt: 關鍵字（逗號分隔）
    QPlainTextEdit *m_trackersEdit    = nullptr; // tr: 追蹤器（每行一個）
    QLineEdit      *m_webSeedEdit     = nullptr; // ws: 網頁種子（選填）
    QTextEdit      *m_resultEdit      = nullptr; // 產生結果
    QLabel         *m_statusLabel     = nullptr;

    QPushButton *m_generateButton     = nullptr;
    QPushButton *m_copyButton         = nullptr;
    QPushButton *m_clearButton        = nullptr;
    QPushButton *m_loadTrackersButton = nullptr;
    QPushButton *m_testButton         = nullptr;
};
