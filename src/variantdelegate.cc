/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "variantdelegate.h"

#include <QDateTime>
#include <QLineEdit>
#include <QRegularExpressionValidator>
#include <QVector3D>


VariantDelegate::VariantDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
    boolExp.setPattern("true|false");
    boolExp.setPatternOptions(QRegularExpression::CaseInsensitiveOption);

    byteArrayExp.setPattern("[\\x00-\\xff]*");
    charExp.setPattern(".");
    colorExp.setPattern("^\\(([0-9]*),([0-9]*),([0-9]*),([0-9]*)\\)$");
    doubleExp.setPattern("");
    pointExp.setPattern("^\\((-?[0-9]*),(-?[0-9]*)\\)$");
    rectExp.setPattern("^\\((-?[0-9]*),(-?[0-9]*),(-?[0-9]*),(-?[0-9]*)\\)$");
    signedIntegerExp.setPattern("-?[0-9]*");
    sizeExp = pointExp;
    unsignedIntegerExp.setPattern("[0-9]*");

    dateExp.setPattern("([0-9]{,4})-([0-9]{,2})-([0-9]{,2})");
    timeExp.setPattern("([0-9]{,2}):([0-9]{,2}):([0-9]{,2})");
    dateTimeExp.setPattern(dateExp.pattern() + 'T' + timeExp.pattern());
}

void VariantDelegate::paint(QPainter *painter,
                            const QStyleOptionViewItem &option,
                            const QModelIndex &index) const
{
    if (index.column() == 2) {
        QVariant value = index.model()->data(index, Qt::UserRole);
        if (!isSupportedType(value.type())) {
            QStyleOptionViewItem myOption = option;
            myOption.state &= ~QStyle::State_Enabled;
            QStyledItemDelegate::paint(painter, myOption, index);
            return;
        }
    }

    QStyledItemDelegate::paint(painter, option, index);
}

QWidget *VariantDelegate::createEditor(QWidget *parent,
        const QStyleOptionViewItem & /* option */,
        const QModelIndex &index) const
{
    if (index.column() != 2)
        return nullptr;

    QVariant originalValue = index.model()->data(index, Qt::UserRole);
    if (!isSupportedType(originalValue.type()))
        return nullptr;

    QLineEdit *lineEdit = new QLineEdit(parent);
    lineEdit->setFrame(false);

    QRegularExpression regExp;

    switch (originalValue.type()) {
    case QVariant::Bool:
        regExp = boolExp;
        break;
    case QVariant::ByteArray:
        regExp = byteArrayExp;
        break;
    case QVariant::Char:
        regExp = charExp;
        break;
    case QVariant::Color:
        regExp = colorExp;
        break;
    case QVariant::Date:
        regExp = dateExp;
        break;
    case QVariant::DateTime:
        regExp = dateTimeExp;
        break;
    case QVariant::Double:
        regExp = doubleExp;
        break;
    case QVariant::Int:
    case QVariant::LongLong:
        regExp = signedIntegerExp;
        break;
    case QVariant::Point:
        regExp = pointExp;
        break;
    case QVariant::Rect:
        regExp = rectExp;
        break;
    case QVariant::Size:
        regExp = sizeExp;
        break;
    case QVariant::Time:
        regExp = timeExp;
        break;
    case QVariant::UInt:
    case QVariant::ULongLong:
        regExp = unsignedIntegerExp;
        break;
    default:
        break;
    }

    if (regExp.isValid()) {
        QValidator *validator = new QRegularExpressionValidator(regExp, lineEdit);
        lineEdit->setValidator(validator);
    }

    return lineEdit;
}

void VariantDelegate::setEditorData(QWidget *editor,
                                    const QModelIndex &index) const
{
    QVariant value = index.model()->data(index, Qt::UserRole);
    if (QLineEdit *lineEdit = qobject_cast<QLineEdit *>(editor))
        lineEdit->setText(displayText(value, (QMetaType::Type)value.type()));
}

void VariantDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                   const QModelIndex &index) const
{
    QLineEdit *lineEdit = qobject_cast<QLineEdit *>(editor);
    if (!lineEdit->isModified())
        return;

    QString text = lineEdit->text();
    const QValidator *validator = lineEdit->validator();
    if (validator) {
        int pos;
        if (validator->validate(text, pos) != QValidator::Acceptable)
            return;
    }

    QVariant originalValue = index.model()->data(index, Qt::UserRole);
    QVariant value;
    QRegularExpressionMatch match;

    switch (originalValue.type()) {
    case QVariant::Char:
        value = text.at(0);
        break;
    case QVariant::Color:
        match = colorExp.match(text);
        value = QColor(qMin(match.captured(1).toInt(), 255),
                       qMin(match.captured(2).toInt(), 255),
                       qMin(match.captured(3).toInt(), 255),
                       qMin(match.captured(4).toInt(), 255));
        break;
    case QVariant::Date:
        {
            QDate date = QDate::fromString(text, Qt::ISODate);
            if (!date.isValid())
                return;
            value = date;
        }
        break;
    case QVariant::DateTime:
        {
            QDateTime dateTime = QDateTime::fromString(text, Qt::ISODate);
            if (!dateTime.isValid())
                return;
            value = dateTime;
        }
        break;
    case QVariant::Point:
        match = pointExp.match(text);
        value = QPoint(match.captured(1).toInt(), match.captured(2).toInt());
        break;
    case QVariant::Rect:
        match = rectExp.match(text);
        value = QRect(match.captured(1).toInt(), match.captured(2).toInt(),
                      match.captured(3).toInt(), match.captured(4).toInt());
        break;
    case QVariant::Size:
        match = sizeExp.match(text);
        value = QSize(match.captured(1).toInt(), match.captured(2).toInt());
        break;
    case QVariant::StringList:
        value = text.split(',');
        break;
    case QVariant::Time:
        {
            QTime time = QTime::fromString(text, Qt::ISODate);
            if (!time.isValid())
                return;
            value = time;
        }
        break;
    default:
        value = text;
        value.convert(originalValue.type());
    }

    model->setData(index, displayText(value, (QMetaType::Type)value.type()), Qt::DisplayRole);
    model->setData(index, value, Qt::UserRole);
}

bool VariantDelegate::isSupportedType(QVariant::Type type)
{
    switch (type) {
    case QVariant::Bool:
    case QVariant::ByteArray:
    case QVariant::Char:
    case QVariant::Color:
    case QVariant::Date:
    case QVariant::DateTime:
    case QVariant::Double:
    case QVariant::Int:
    case QVariant::LongLong:
    case QVariant::Point:
    case QVariant::Rect:
    case QVariant::Size:
    case QVariant::String:
    case QVariant::StringList:
    case QVariant::Time:
    case QVariant::UInt:
    case QVariant::ULongLong:
	case QMetaType::QVector3D:
        return true;
    default:
        return false;
    }
}

QString VariantDelegate::displayText(const QVariant &value, QMetaType::Type type)
{
    switch (type) {
    case QMetaType::Bool:
    case QMetaType::QByteArray:
    case QMetaType::Double:
	case QMetaType::Float:
    case QMetaType::Int:
    case QMetaType::LongLong:
    case QMetaType::QString:
    case QMetaType::UInt:
    case QMetaType::ULongLong:
        return value.toString();
    case QMetaType::QChar:
	    return QString(QChar::fromLatin1((char) value.toUInt()));
    case QMetaType::QColor:
        {
            QColor color = QColor::fromRgba(value.toUInt());
            return QString("(r%1,g%2,b%3,a%4)")
                   .arg(color.red()).arg(color.green())
                   .arg(color.blue()).arg(color.alpha());
        }
    case QMetaType::QDate:
        return value.toDate().toString(Qt::ISODate);
    case QMetaType::QDateTime:
        return value.toDateTime().toString(Qt::ISODate);
    case QVariant::Invalid:
        return "<Invalid>";
    case QMetaType::QPoint:
        {
            QPoint point = value.toPoint();
            return QString("(%1,%2)").arg(point.x()).arg(point.y());
        }
    case QMetaType::QRect:
        {
            QRect rect = value.toRect();
            return QString("(%1,%2,%3,%4)")
                   .arg(rect.x()).arg(rect.y())
                   .arg(rect.width()).arg(rect.height());
        }
    case QMetaType::QSize:
        {
            QSize size = value.toSize();
            return QString("(%1,%2)").arg(size.width()).arg(size.height());
        }
    case QMetaType::QStringList:
        return value.toStringList().join(',');
    case QMetaType::QTime:
        return value.toTime().toString(Qt::ISODate);
    case QMetaType::QVector3D: {
			QVector3D vec = value.value<QVector3D>();
			return QString("<%1,%2,%3>").arg(vec.x()).arg(vec.y()).arg(vec.z());
	    }
    default:
        break;
    }
    return QString("<%1>").arg(value.typeName());
}
