#ifndef EDITTRACKDIALOG_H
#define EDITTRACKDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include "MetadataManager.h"

class EditTrackDialog : public QDialog {
    Q_OBJECT
public:
    explicit EditTrackDialog(const TrackMetadata &track, QWidget *parent = nullptr);
    ~EditTrackDialog();

    QString title() const;
    QString artist() const;
    QByteArray newCoverData() const;
    void refreshStyle();

private slots:
    void onChangeCoverClicked();
    void onSaveClicked();

private:
    TrackMetadata m_track;
    QByteArray m_newCoverData;
    
    QLabel *m_coverLabel;
    QLineEdit *m_titleInput;
    QLineEdit *m_artistInput;
    
    QPushButton *m_saveBtn;
    QPushButton *m_cancelBtn;
    
    void setupUI();
    void applyQSS();
    void loadCover(const QString &path);
};

#endif // EDITTRACKDIALOG_H
