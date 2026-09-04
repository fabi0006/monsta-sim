/*
 * BSD 3-Clause License
 *
 * Copyright (c) 2010-2025, Viktor Seib | 2026, Fabian Schneider
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, this
 * * list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * * Neither the name of the copyright holder nor the names of its
 *   contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */


#include "MainWindow.h"
#include "ui_MainWindow.h"

#include <QPushButton>
#include <QKeyEvent>
#include "MonstaWidget.h"
#include <iostream>
#include "licensepage.h"
#include "versionpage.h"


MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    m_MonstaWidget = new MonstaWidget();

    ui->verticalLayout->setSizeConstraint(QLayout::SetFixedSize);
    ui->verticalLayout->addWidget(m_MonstaWidget);
    m_MonstaWidget->setFocus();
    m_MonstaWidget->startSimulation();

    m_StartButton = new QPushButton("Start");
    m_StopButton = new QPushButton("Stop");
    m_GridButton = new QPushButton("Grid");

    ui->mainToolBar->addWidget(m_StartButton);
    ui->mainToolBar->addWidget(m_StopButton);
    ui->mainToolBar->addWidget(m_GridButton);

    connect(ui->actionClose_2, &QAction::triggered, this, &MainWindow::closeSimulator);
    connect(ui->actionLicense, &QAction::triggered, this, &MainWindow::openLicenseWindow);
    connect(ui->actionVersion, &QAction::triggered, this, &MainWindow::openVersionWindow);
    connect(ui->actionSmall, &QAction::triggered, this, &MainWindow::windowSmall);
    connect(ui->actionMedium, &QAction::triggered, this, &MainWindow::windowMedium);
    connect(ui->actionLarge, &QAction::triggered, this, &MainWindow::windowLarge);
    connect(ui->actionStart, &QAction::triggered, m_MonstaWidget, &MonstaWidget::startButtonClicked);
    connect(ui->actionStop, &QAction::triggered, m_MonstaWidget, &MonstaWidget::stopButtonClicked);
    connect(ui->actionGrid, &QAction::triggered, m_MonstaWidget, &MonstaWidget::gridButtonClicked);
    connect(m_StartButton, SIGNAL(clicked()), m_MonstaWidget,  SLOT(startButtonClicked()));
    connect(m_StopButton, SIGNAL(clicked()), m_MonstaWidget,  SLOT(stopButtonClicked()));
    connect(m_GridButton, SIGNAL(clicked()), m_MonstaWidget,  SLOT(gridButtonClicked()));

    setFixedSize(m_MonstaWidget->width(),
                 m_MonstaWidget->height()+ui->mainToolBar->height()*4
                 );
    
    
}

MainWindow::~MainWindow()
{
    delete m_MonstaWidget;
    delete m_StartButton;
    delete m_StopButton;
    delete m_GridButton;
    delete ui;
}

void MainWindow::changeEvent(QEvent *e)
{
    QMainWindow::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void MainWindow::openLicenseWindow() {
    LicensePage *licPag = new LicensePage(this);
    licPag->setAttribute(Qt::WA_DeleteOnClose); 
    licPag->show();
}

void MainWindow::openVersionWindow() {
    versionPage *verPag = new versionPage(this);
    verPag->setAttribute(Qt::WA_DeleteOnClose); 
    verPag->show();
}

void MainWindow::closeSimulator() {
    this->close();
}

void MainWindow::windowSmall() {
    this->setFixedSize(1200, 900);
}

void MainWindow::windowMedium() {
    this->setFixedSize(1200, 900);
}

void MainWindow::windowLarge() {
    this->setFixedSize(1250, 1050);
}