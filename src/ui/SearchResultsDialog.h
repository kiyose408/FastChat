#ifndef SEARCHRESULTSDIALOG_H
#define SEARCHRESULTSDIALOG_H

#include <QDialog>
#include <QJsonArray>
#include <QListWidget>

class QLineEdit;
class QPushButton;
class QListWidget;

class SearchResultsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SearchResultsDialog(QWidget *parent = nullptr);
    ~SearchResultsDialog();
    
    void setResults(const QJsonArray& results, const QString& query);
    void clearResults();

signals:
    void messageSelected(int friendId, const QString& friendName, int messageId);

private slots:
    void onItemClicked(QListWidgetItem* item);
    void onSearchClicked();

private:
    QLineEdit* m_searchEdit;
    QPushButton* m_searchBtn;
    QListWidget* m_resultsList;
    QJsonArray m_results;
    
    void setupUI();
};

#endif // SEARCHRESULTSDIALOG_H
