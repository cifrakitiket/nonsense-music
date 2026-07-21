#include "AuthView.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGraphicsDropShadowEffect>
#include "StyleManager.h"

AuthView::AuthView(CloudManager *cloudManager, QWidget *parent)
    : QWidget(parent), m_cloudManager(cloudManager) {
    setupUI();
    
    connect(m_cloudManager, &CloudManager::authSuccess, this, &AuthView::onAuthSuccess);
    connect(m_cloudManager, &CloudManager::authFailed, this, &AuthView::onAuthFailed);
}

void AuthView::setupUI() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setAlignment(Qt::AlignCenter);
    
    QWidget *card = new QWidget(this);
    card->setObjectName("AuthCard");
    card->setFixedSize(400, 450);
    
    QVBoxLayout *cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(40, 40, 40, 40);
    cardLayout->setSpacing(20);
    
    m_authFormWidget = new QWidget(card);
    QVBoxLayout *formLayout = new QVBoxLayout(m_authFormWidget);
    formLayout->setContentsMargins(0, 0, 0, 0);
    formLayout->setSpacing(20);
    
    m_titleLabel = new QLabel(tr("Nonsense Cloud"), m_authFormWidget);
    m_titleLabel->setAlignment(Qt::AlignCenter);
    
    m_emailInput = new QLineEdit(m_authFormWidget);
    m_emailInput->setPlaceholderText(tr("Email"));
    m_emailInput->setFixedHeight(40);
    
    m_passwordInput = new QLineEdit(m_authFormWidget);
    m_passwordInput->setPlaceholderText(tr("Password"));
    m_passwordInput->setEchoMode(QLineEdit::Password);
    m_passwordInput->setFixedHeight(40);
    
    m_loginBtn = new QPushButton(tr("Login"), m_authFormWidget);
    m_loginBtn->setCursor(Qt::PointingHandCursor);
    m_loginBtn->setFixedHeight(40);
    
    m_registerBtn = new QPushButton(tr("Register"), m_authFormWidget);
    m_registerBtn->setCursor(Qt::PointingHandCursor);
    m_registerBtn->setFixedHeight(40);
    
    m_errorLabel = new QLabel(m_authFormWidget);
    m_errorLabel->setAlignment(Qt::AlignCenter);
    m_errorLabel->setWordWrap(true);
    m_errorLabel->hide();
    
    formLayout->addWidget(m_titleLabel);
    formLayout->addStretch();
    formLayout->addWidget(m_emailInput);
    formLayout->addWidget(m_passwordInput);
    formLayout->addWidget(m_errorLabel);
    formLayout->addStretch();
    formLayout->addWidget(m_loginBtn);
    formLayout->addWidget(m_registerBtn);
    
    m_loggedInWidget = new QWidget(card);
    QVBoxLayout *loggedInLayout = new QVBoxLayout(m_loggedInWidget);
    loggedInLayout->setContentsMargins(0, 0, 0, 0);
    loggedInLayout->setSpacing(20);
    loggedInLayout->setAlignment(Qt::AlignCenter);
    
    m_loggedInTitle = new QLabel(tr("Welcome!"), m_loggedInWidget);
    m_loggedInTitle->setAlignment(Qt::AlignCenter);
    
    m_loggedInEmail = new QLabel("", m_loggedInWidget);
    m_loggedInEmail->setAlignment(Qt::AlignCenter);
    
    m_logoutBtn = new QPushButton(tr("Logout"), m_loggedInWidget);
    m_logoutBtn->setCursor(Qt::PointingHandCursor);
    m_logoutBtn->setFixedHeight(40);
    
    loggedInLayout->addWidget(m_loggedInTitle);
    loggedInLayout->addWidget(m_loggedInEmail);
    loggedInLayout->addStretch();
    loggedInLayout->addWidget(m_logoutBtn);
    
    cardLayout->addWidget(m_authFormWidget);
    cardLayout->addWidget(m_loggedInWidget);
    
    mainLayout->addWidget(card);
    
    connect(m_loginBtn, &QPushButton::clicked, this, &AuthView::onLoginClicked);
    connect(m_registerBtn, &QPushButton::clicked, this, &AuthView::onRegisterClicked);
    connect(m_logoutBtn, &QPushButton::clicked, this, &AuthView::onLogoutClicked);
    
    updateViewState();
    refreshStyle();
}

void AuthView::retranslateUI() {
    m_titleLabel->setText(tr("Nonsense Cloud"));
    m_emailInput->setPlaceholderText(tr("Email"));
    m_passwordInput->setPlaceholderText(tr("Password"));
    m_loginBtn->setText(tr("Login"));
    m_registerBtn->setText(tr("Register"));
    m_loggedInTitle->setText(tr("Welcome!"));
    m_logoutBtn->setText(tr("Logout"));
}

void AuthView::refreshStyle() {
    QString cardStyle = QString(
        "QWidget#AuthCard { background-color: %1; border-radius: 12px; border: 1px solid %2; }"
    ).arg(StyleManager::bgElevated(), StyleManager::border());
    
    QWidget *cardWidget = findChild<QWidget*>("AuthCard");
    if (cardWidget) cardWidget->setStyleSheet(cardStyle);
    
    m_titleLabel->setStyleSheet(QString("font-size: 24px; font-weight: bold; color: %1; border: none;").arg(StyleManager::textPrimary()));
    m_errorLabel->setStyleSheet(QString("color: %1; font-size: 13px; border: none;").arg(StyleManager::error()));
    m_loggedInTitle->setStyleSheet(QString("font-size: 24px; font-weight: bold; color: %1; border: none;").arg(StyleManager::textPrimary()));
    m_loggedInEmail->setStyleSheet(QString("font-size: 16px; color: %1; border: none;").arg(StyleManager::textSecondary()));
    
    m_emailInput->setStyleSheet(StyleManager::inputStyle());
    m_passwordInput->setStyleSheet(StyleManager::inputStyle());
    
    m_loginBtn->setStyleSheet(StyleManager::primaryButtonStyle());
    m_registerBtn->setStyleSheet(StyleManager::secondaryButtonStyle());
    m_logoutBtn->setStyleSheet(StyleManager::primaryButtonStyle());
}

void AuthView::onLoginClicked() {
    m_errorLabel->hide();
    QString email = m_emailInput->text().trimmed();
    QString password = m_passwordInput->text();
    if (email.isEmpty() || password.isEmpty()) {
        m_errorLabel->setText(tr("Please enter email and password."));
        m_errorLabel->show();
        return;
    }
    m_loginBtn->setEnabled(false);
    m_registerBtn->setEnabled(false);
    emit loginRequested(email, password);
}

void AuthView::onRegisterClicked() {
    m_errorLabel->hide();
    QString email = m_emailInput->text().trimmed();
    QString password = m_passwordInput->text();
    if (email.isEmpty() || password.isEmpty()) {
        m_errorLabel->setText(tr("Please enter email and password."));
        m_errorLabel->show();
        return;
    }
    m_loginBtn->setEnabled(false);
    m_registerBtn->setEnabled(false);
    emit registerRequested(email, password);
}

void AuthView::onAuthSuccess(const QString &email) {
    m_loginBtn->setEnabled(true);
    m_registerBtn->setEnabled(true);
    m_emailInput->clear();
    m_passwordInput->clear();
    updateViewState();
    emit loginSuccessful();
}

void AuthView::onAuthFailed(const QString &errorMsg) {
    m_loginBtn->setEnabled(true);
    m_registerBtn->setEnabled(true);
    m_errorLabel->setText(errorMsg);
    m_errorLabel->show();
}

void AuthView::onLogoutClicked() {
    m_cloudManager->logout();
    updateViewState();
}

void AuthView::updateViewState() {
    if (m_cloudManager->isAuthenticated()) {
        m_authFormWidget->hide();
        m_loggedInWidget->show();
        m_loggedInEmail->setText(m_cloudManager->currentUserEmail());
    } else {
        m_loggedInWidget->hide();
        m_authFormWidget->show();
    }
}
