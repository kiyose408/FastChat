#include "SearchResultsDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QListWidget>
#include <QLabel>
#include <QJsonObject>
#include <QDateTime>
#include <QDebug>

SearchResultsDialog::SearchResultsDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUI();
}

SearchResultsDialog::~SearchResultsDialog()
{
}

void SearchResultsDialog::setupUI()
{
    setWindowTitle(QString::fromUtf8("搜索消息"));
    setWindowFlags(Qt::Dialog | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);
    setMinimumSize(400, 500);
    resize(450, 550);
    
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(15, 15, 15, 15);
    
    QHBoxLayout* searchLayout = new QHBoxLayout();
    searchLayout->setSpacing(10);
    
    m_searchEdit = new QLineEdit();
    m_searchEdit->setPlaceholderText(QString::fromUtf8("输入搜索内容..."));
    m_searchEdit->setMinimumHeight(35);
    
    m_searchBtn = new QPushButton(QString::fromUtf8("搜索"));
    m_searchBtn->setFixedSize(70, 35);
    m_searchBtn->setStyleSheet("QPushButton { background-color: #3498DB; color: white; border: none; border-radius: 5px; }"
                                "QPushButton:hover { background-color: #2980B9; }");
    
    searchLayout->addWidget(m_searchEdit);
    searchLayout->addWidget(m_searchBtn);
    
    mainLayout->addLayout(searchLayout);
    
    QLabel* resultsLabel = new QLabel(QString::fromUtf8("搜索结果"));
    resultsLabel->setFont(QFont("Microsoft YaHei", 10, QFont::Bold));
    mainLayout->addWidget(resultsLabel);
    
    m_resultsList = new QListWidget();
    m_resultsList->setStyleSheet("QListWidget { border: 1px solid #E0E0E0; border-radius: 5px; }"
                                  "QListWidget::item { padding: 10px; border-bottom: 1px solid #EEEEEE; }"
                                  "QListWidget::item:hover { background-color: #F5F5F5; }"
                                  "QListWidget::item:selected { background-color: #E3F2FD; color: black; }");
    mainLayout->addWidget(m_resultsList);
    
    connect(m_resultsList, &QListWidget::itemClicked, this, &SearchResultsDialog::onItemClicked);
}

void SearchResultsDialog::setResults(const QJsonArray& results, const QString& query)
{
    m_results = results;
    m_searchEdit->setText(query);
    m_resultsList->clear();
    
    if (results.isEmpty()) {
        QListWidgetItem* emptyItem = new QListWidgetItem(QString::fromUtf8("未找到相关消息"));
        emptyItem->setFlags(emptyItem->flags() & ~Qt::ItemIsSelectable);
        m_resultsList->addItem(emptyItem);
        return;
    }
    
    for (const QJsonValue& value : results) {
        QJsonObject msg = value.toObject();
        
        int friendId = msg["friend_id"].toInt();
        QString friendName = msg["friend_username"].toString();
        QString content = msg["content"].toString();
        QString createdAt = msg["created_at"].toString();
        int senderId = msg["sender_id"].toInt();
        
        QDateTime dateTime = QDateTime::fromString(createdAt, Qt::ISODate);
        QString timeStr = dateTime.toString("yyyy-MM-dd hh:mm");
        
        QString displayText;
        if (senderId == friendId) {
            displayText = QString("[%1] %2\n%3").arg(friendName, timeStr, content);
        } else {
            displayText = QString(QString::fromUtf8("[我 -> %1] %2\n%3")).arg(friendName, timeStr, content);
        }
        
        QListWidgetItem* item = new QListWidgetItem(displayText);
        item->setData(Qt::UserRole, friendId);
        item->setData(Qt::UserRole + 1, friendName);
        item->setData(Qt::UserRole + 2, msg["id"].toInt());
        
        if (content.length() > 100) {
            content = content.left(100) + "...";
        }
        
        m_resultsList->addItem(item);
    }
}

void SearchResultsDialog::clearResults()
{
    m_results = QJsonArray();
    m_resultsList->clear();
    m_searchEdit->clear();
}

void SearchResultsDialog::onItemClicked(QListWidgetItem* item)
{
    int friendId = item->data(Qt::UserRole).toInt();
    QString friendName = item->data(Qt::UserRole + 1).toString();
    int messageId = item->data(Qt::UserRole + 2).toInt();
    
    if (friendId > 0) {
        emit messageSelected(friendId, friendName, messageId);
        close();
    }
}

void SearchResultsDialog::onSearchClicked()
{
    QString query = m_searchEdit->text().trimmed();
    if (!query.isEmpty()) {
        qDebug() << "Search clicked:" << query;
    }
}
