#ifndef AUTHVIEW_H
#define AUTHVIEW_H

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include "CloudManager.h"

class AuthView : public QWidget {
    Q_OBJECT
public:
    explicit AuthView(CloudManager *cloudManager, QWidget *parent = nullptr);

    void retranslateUI();
    void refreshStyle();

signals:
    void loginSuccessful();
    void loginRequested(const QString &email, const QString &password);
    void registerRequested(const QString &email, const QString &password);

private slots:
    void onLoginClicked();
    void onRegisterClicked();
    void onLogoutClicked();
    void onAuthSuccess(const QString &email);
    void onAuthFailed(const QString &errorMsg);
    void updateViewState();

private:
    CloudManager *m_cloudManager;
    
    QWidget *m_authFormWidget;
    QLabel *m_titleLabel;
    QLineEdit *m_emailInput;
    QLineEdit *m_passwordInput;
    QPushButton *m_loginBtn;
    QPushButton *m_registerBtn;
    QLabel *m_errorLabel;
    
    QWidget *m_loggedInWidget;
    QLabel *m_loggedInTitle;
    QLabel *m_loggedInEmail;
    QPushButton *m_logoutBtn;
    
    void setupUI();
};

#endif // AUTHVIEW_H
