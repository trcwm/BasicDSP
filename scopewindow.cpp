#include <QRadioButton>
#include <QSpinBox>
#include "scopewindow.h"
#include "ui_scopewindow.h"

ScopeWindow::ScopeWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ScopeWindow)
{
    setWindowFlags(Qt::Tool);
    ui->setupUi(this);

    m_scope = new ScopeWidget(this);
    ui->mainLayout->addWidget(m_scope, 1);

    QHBoxLayout* m_hsizer = new QHBoxLayout(this);

    m_chan1 = new QLineEdit(this);
    m_chan2 = new QLineEdit(this);
    QLabel *m_chan1Label = new QLabel(this);
    QLabel *m_chan2Label = new QLabel(this);
    m_chan1Label->setText(" CH1 ");
    m_chan2Label->setText(" CH2 ");
    m_chan1Label->setStyleSheet("background-color: green");
    m_chan2Label->setStyleSheet("background-color: yellow");
    m_hsizer->addWidget(m_chan1Label);
    m_hsizer->addWidget(m_chan1);
    m_hsizer->addWidget(m_chan2Label);
    m_hsizer->addWidget(m_chan2);
    ui->mainLayout->addLayout(m_hsizer);

    connect(m_chan1, SIGNAL(textChanged(QString)), this, SLOT(chan1Changed()));
    connect(m_chan2, SIGNAL(textChanged(QString)), this, SLOT(chan2Changed()));

    QHBoxLayout *triggerLayout = new QHBoxLayout(this);

    // create trigger channel selection
    QGroupBox *gb = createTriggerChannelGroup();
    triggerLayout->addWidget(gb);

    // create trigger level
    QGroupBox *gb2 = createTriggerLevelGroup();
    triggerLayout->addWidget(gb2);

    ui->mainLayout->addLayout(triggerLayout);

    // override default trigger setup of scope to
    // be inline with the GUI defaults
    m_scope->setTriggerState(false);
    m_scope->setTriggerChannel(0);
    m_scope->setTriggerLevel(0.0f);
}

ScopeWindow::~ScopeWindow()
{
    delete ui;
}

QGroupBox *ScopeWindow::createTriggerChannelGroup()
{
    QGroupBox *groupBox = new QGroupBox(tr("Trigger setup"));

    m_trigNone = new QRadioButton(tr("None"));
    m_trigCh1  = new QRadioButton(tr("Channel 1"));
    m_trigCh2  = new QRadioButton(tr("Channel 2"));

    connect(m_trigNone, SIGNAL(clicked(bool)), this, SLOT(triggerChanged()));
    connect(m_trigCh1, SIGNAL(clicked(bool)), this, SLOT(triggerChanged()));
    connect(m_trigCh2, SIGNAL(clicked(bool)), this, SLOT(triggerChanged()));

    m_trigNone->setChecked(true);

    QVBoxLayout *vbox = new QVBoxLayout();
    vbox->addWidget(m_trigNone);
    vbox->addWidget(m_trigCh1);
    vbox->addWidget(m_trigCh2);
    //vbox->addStretch(0);
    groupBox->setLayout(vbox);

    return groupBox;
}

QGroupBox *ScopeWindow::createTriggerLevelGroup()
{
    QGroupBox *groupBox = new QGroupBox(tr("Trigger level"));
    m_triggerSpin = new QSpinBox();
    m_triggerSpin->setRange(-100,100);
    m_triggerSpin->setSingleStep(10);

    connect(m_triggerSpin, SIGNAL(valueChanged(int)), this, SLOT(triggerLevelChanged(int)));

    //spin->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

    QHBoxLayout *hbox = new QHBoxLayout();
    hbox->addWidget(new QLabel("Level (percent)"));
    hbox->addWidget(m_triggerSpin);
    groupBox->setLayout(hbox);

    return groupBox;
}

void ScopeWindow::submit256Samples(VirtualMachine::ring_buffer_data_t *buffer)
{
    m_scope->submit256Samples(buffer);
}

void ScopeWindow::setSampleRate(float rate)
{
    m_scope->setSampleRate(rate);
}

std::string ScopeWindow::getChannelName(uint32_t channel)
{
    if (channel == 0)
        return m_chan1->text().toStdString();

    if (channel == 1)
        return m_chan2->text().toStdString();

    return std::string("");
}

void ScopeWindow::chan1Changed()
{
    emit channelChanged(0);
}

void ScopeWindow::chan2Changed()
{
    emit channelChanged(1);
}

void ScopeWindow::triggerChanged()
{
    if (m_trigCh1->isChecked())
    {
        m_scope->setTriggerState(true);
        m_scope->setTriggerChannel(0);
    }
    else if (m_trigCh2->isChecked())
    {
        m_scope->setTriggerState(true);
        m_scope->setTriggerChannel(1);
    }
    else
    {
        m_scope->setTriggerState(false);
    }
}

void ScopeWindow::triggerLevelChanged(int value)
{
    m_scope->setTriggerLevel(static_cast<float>(value)/100.0f);
}
