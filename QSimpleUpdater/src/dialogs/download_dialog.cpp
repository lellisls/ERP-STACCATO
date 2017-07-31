/*
 * (C) Copyright 2014 Alex Spataru
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the GNU Lesser General Public License
 * (LGPL) version 2.1 which accompanies this distribution, and is available at
 * http://www.gnu.org/licenses/lgpl-2.1.html
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 */

#include "download_dialog.h"
#include "ui_download_dialog.h"

#include <QMutex>

DownloadDialog::DownloadDialog(QWidget *parent) : QWidget(parent), ui(new Ui::DownloadDialog) {

  // Setup the UI
  ui->setupUi(this);

  // Make the window look like a dialog
  QIcon _blank;
  setWindowIcon(_blank);
  setWindowModality(Qt::WindowModal);
  setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);

  // Connect SIGNALS/SLOTS
  connect(ui->stopButton, &QAbstractButton::clicked, this, &DownloadDialog::cancelDownload);
  connect(ui->openButton, &QAbstractButton::clicked, this, &DownloadDialog::installUpdate);

  // Configure open button
  ui->openButton->setEnabled(false);
  ui->openButton->setVisible(false);

  // Initialize the network access manager
  m_manager = new QNetworkAccessManager(this);

  // Avoid SSL issues
  connect(m_manager, &QNetworkAccessManager::sslErrors, this, &DownloadDialog::ignoreSslErrors);
}

DownloadDialog::~DownloadDialog() { delete ui; }

void DownloadDialog::beginDownload(const QUrl &url) {
  Q_ASSERT(not url.isEmpty());

  // Reset the UI
  ui->progressBar->setValue(0);
  ui->stopButton->setText(tr("Parar"));
  ui->downloadLabel->setText(tr("Baixando atualizações"));
  ui->timeLabel->setText(tr("Tempo restante") + ": " + tr("desconhecido"));

  // Begin the download
  m_reply = m_manager->get(QNetworkRequest(url));
  m_start_time = QDateTime::currentDateTime().toTime_t();

  // Update the progress bar value automatically
  connect(m_reply, &QNetworkReply::downloadProgress, this, &DownloadDialog::updateProgress);

  // Write the file to the hard disk once the download is finished
  connect(m_reply, &QNetworkReply::finished, this, &DownloadDialog::downloadFinished);

  // Show the dialog
  showNormal();
}

void DownloadDialog::installUpdate() {
  openDownload();
  qApp->closeAllWindows();
}

void DownloadDialog::openDownload() {
  if (not m_path.isEmpty()) {
    QString url = m_path;

    if (url.startsWith("/")) {
      url = "file://" + url;
    } else {
      url = "file:///" + url;
    }

    QDesktopServices::openUrl(url);
  }
}

void DownloadDialog::cancelDownload() {
  if (not m_reply->isFinished()) {
    QMessageBox _message;
    _message.setWindowTitle(tr("Atualizador"));
    _message.setIcon(QMessageBox::Question);
    _message.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    _message.setText(tr("Você tem certeza que quer cancelar a atualização?"));
    _message.setButtonText(QMessageBox::Yes, "Cancelar");
    _message.setButtonText(QMessageBox::No, "Continuar");

    if (_message.exec() == QMessageBox::Yes) {
      hide();
      m_reply->abort();
    }
  } else {
    hide();
  }
}

void DownloadDialog::downloadFinished() {
  ui->stopButton->setText(tr("Fechar"));
  ui->downloadLabel->setText(tr("Download completo!"));
  ui->timeLabel->setText(tr("O instalador vai abrir em uma janela separada..."));

  QByteArray data = m_reply->readAll();

  if (not data.isEmpty()) {
    QStringList list = m_reply->url().toString().split("/");
    QFile file(QDir::tempPath() + "/" + list.at(list.count() - 1));
    QMutex _mutex;

    if (file.open(QIODevice::WriteOnly)) {
      _mutex.lock();
      file.write(data);
      m_path = file.fileName();
      file.close();
      _mutex.unlock();
    }

    installUpdate();
  }
}

void DownloadDialog::updateProgress(qint64 received, qint64 total) {
  // We know the size of the download, so we can calculate the progress....
  if (total > 0 and received > 0) {
    ui->progressBar->setMinimum(0);
    ui->progressBar->setMaximum(100);

    int _progress = (int)((received * 100) / total);
    ui->progressBar->setValue(_progress);

    QString _total_string;
    QString _received_string;

    float _total = total;
    float _received = received;

    if (_total < 1024) {
      _total_string = tr("%1 bytes").arg(_total);
    } else if (_total < 1024 * 1024) {
      _total = roundNumber(_total / 1024);
      _total_string = tr("%1 KB").arg(_total);
    } else {
      _total = roundNumber(_total / (1024 * 1024));
      _total_string = tr("%1 MB").arg(_total);
    }

    if (_received < 1024) {
      _received_string = tr("%1 bytes").arg(_received);
    } else if (received < 1024 * 1024) {
      _received = roundNumber(_received / 1024);
      _received_string = tr("%1 KB").arg(_received);
    } else {
      _received = roundNumber(_received / (1024 * 1024));
      _received_string = tr("%1 MB").arg(_received);
    }

    ui->downloadLabel->setText(tr("Baixando atualizações") + " (" + _received_string + " " + tr("de") + " " +
                               _total_string + ")");

    uint _diff = QDateTime::currentDateTime().toTime_t() - m_start_time;

    if (_diff > 0) {
      QString _time_string;
      float _time_remaining = total / (received / _diff);

      if (_time_remaining > 7200) {
        _time_remaining /= 3600;
        _time_string = tr("Por volta de %1 horas").arg(int(_time_remaining + 0.5));
      } else if (_time_remaining > 60) {
        _time_remaining /= 60;
        _time_string = tr("Por volta de %1 minutos").arg(int(_time_remaining + 0.5));
      } else
        _time_string = tr("%1 segundos").arg(int(_time_remaining + 0.5));

      ui->timeLabel->setText(tr("Tempo restante") + ": " + _time_string);
    }
  } else { // We do not know the size of the download, so we avoid scaring the shit out of the user
    ui->progressBar->setValue(-1);
    ui->progressBar->setMinimum(0);
    ui->progressBar->setMaximum(0);
    ui->downloadLabel->setText(tr("Baixando atualizações"));
    ui->timeLabel->setText(tr("Tempo restante") + ": " + tr("desconhecido"));
  }
}

void DownloadDialog::ignoreSslErrors(QNetworkReply *reply, const QList<QSslError> &error) {
#ifndef Q_OS_IOS
  reply->ignoreSslErrors(error);
#else
  Q_UNUSED(reply);
  Q_UNUSED(error);
#endif
}

float DownloadDialog::roundNumber(const float &input) { return roundf(input * 100) / 100; }
