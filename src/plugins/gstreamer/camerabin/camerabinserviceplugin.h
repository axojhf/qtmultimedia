/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/


#ifndef CAMERABINSERVICEPLUGIN_H
#define CAMERABINSERVICEPLUGIN_H

#include <qmediaserviceproviderplugin.h>
#include <private/qgstreamervideoinputdevicecontrol_p.h>

#include <gst/gst.h>

QT_BEGIN_NAMESPACE

class CameraBinServicePlugin
    : public QMediaServiceProviderPlugin
    , public QMediaServiceSupportedDevicesInterface
    , public QMediaServiceFeaturesInterface
{
    Q_OBJECT
    Q_INTERFACES(QMediaServiceSupportedDevicesInterface)
    Q_INTERFACES(QMediaServiceFeaturesInterface)
    Q_PLUGIN_METADATA(IID "org.qt-project.qt.mediaserviceproviderfactory/5.0" FILE "camerabin.json")
public:
    CameraBinServicePlugin();
    ~CameraBinServicePlugin();

    QMediaService* create(const QString &key) override;
    void release(QMediaService *service) override;

    QMediaServiceFeaturesInterface::Features supportedFeatures(const QByteArray &service) const override;

    QByteArray defaultDevice(const QByteArray &service) const override;
    QList<QByteArray> devices(const QByteArray &service) const override;
    QString deviceDescription(const QByteArray &service, const QByteArray &device) override;
    QVariant deviceProperty(const QByteArray &service, const QByteArray &device, const QByteArray &property);

private:
    GstElementFactory *sourceFactory() const;

    mutable GstElementFactory *m_sourceFactory;
};

QT_END_NAMESPACE

#endif // QGSTREAMERCAPTURESERVICEPLUGIN_H
