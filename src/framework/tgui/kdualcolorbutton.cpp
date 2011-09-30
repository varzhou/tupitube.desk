/***************************************************************************
 *   Project TUPI: Magia 2D                                                *
 *   Project Contact: info@maefloresta.com                                 *
 *   Project Website: http://www.maefloresta.com                           *
 *   Project Leader: Gustav Gonzalez <info@maefloresta.com>                *
 *                                                                         *
 *   Developers:                                                           *
 *   2010:                                                                 *
 *    Gustavo Gonzalez / xtingray                                          *
 *                                                                         *
 *   KTooN's versions:                                                     * 
 *                                                                         *
 *   2006:                                                                 *
 *    David Cuadrado                                                       *
 *    Jorge Cuadrado                                                       *
 *   2003:                                                                 *
 *    Fernado Roldan                                                       *
 *    Simena Dinas                                                         *
 *                                                                         *
 *   Copyright (C) 2010 Gustav Gonzalez - http://www.maefloresta.com       *
 *   License:                                                              *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 ***************************************************************************/

#include "kdualcolorbutton.h"
#include "tdebug.h"

#include "kcolorarrow.xpm"
#include "kcolorreset.xpm"

#include <qdrawutil.h>
#include <QPainter>
#include <QBitmap>
#include <QDragEnterEvent>
#include <QPixmap>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QDropEvent>
#include <QApplication>

struct KDualColorButton::Private
{
    QPixmap arrowBitmap;
    QPixmap resetPixmap;
    QBrush fgBrush;
    QBrush bgBrush;
    ColorSpace currentSpace;
};

KDualColorButton::KDualColorButton(QWidget *parent) : QWidget(parent), k(new Private)
{
    k->arrowBitmap = QPixmap((const char **)dcolorarrow_bits);
    k->resetPixmap = QPixmap((const char **)dcolorreset_xpm);

    k->fgBrush = QBrush(Qt::black, Qt::SolidPattern);
    k->bgBrush = QBrush(QColor(0, 0, 0, 0), Qt::SolidPattern);
	
    k->currentSpace = Foreground;

    if (sizeHint().isValid())
        setMinimumSize(sizeHint());
}

KDualColorButton::KDualColorButton(const QBrush &fgColor, const QBrush &bgColor, QWidget *parent) : QWidget(parent), k(new Private)
{
    k->arrowBitmap = QPixmap((const char **)dcolorarrow_bits);
    k->resetPixmap = QPixmap((const char **)dcolorreset_xpm);

    k->fgBrush = fgColor;
    k->bgBrush = bgColor;

    k->currentSpace = Foreground;

    if (sizeHint().isValid())
        setMinimumSize(sizeHint());
}

KDualColorButton::~KDualColorButton()
{
}

QBrush KDualColorButton::foreground() const
{
    return k->fgBrush;
}

QBrush KDualColorButton::background() const
{
    return k->bgBrush;
}

KDualColorButton::ColorSpace KDualColorButton::current() const
{
    return k->currentSpace;
}

QBrush KDualColorButton::currentColor() const
{
    return (k->currentSpace == Background ? k->bgBrush : k->fgBrush);
}

QSize KDualColorButton::sizeHint() const
{
    return QSize(34, 34);
}

void KDualColorButton::setForeground(const QBrush &c)
{
    k->fgBrush = c;
    update();

    // emit fgChanged(k->fgBrush.color());
}

void KDualColorButton::setBackground(const QBrush &c)
{
    k->bgBrush = c;
    update();

    // emit bgChanged(k->bgBrush.color());
}

void KDualColorButton::setCurrentColor(const QBrush &c)
{
    if (k->currentSpace == Background)
        k->bgBrush = c;
    else
        k->fgBrush = c;

    update();
}

void KDualColorButton::setCurrent(ColorSpace s)
{
    k->currentSpace = s;
    update();
}

void KDualColorButton::metrics(QRect &fgRect, QRect &bgRect)
{
    fgRect = QRect(0, 0, width()-14, height()-14);
    bgRect = QRect(14, 14, width()-14, height()-14);
}

void KDualColorButton::paintEvent(QPaintEvent *)
{
    QPalette pal = palette();
    QPainter painter(this);

    QRect fgRect, bgRect;
    metrics(fgRect, bgRect);

    QBrush defBrush = pal.color(QPalette::Button);

    QBrush bgAdjusted = k->bgBrush;
    QBrush fgAdjusted = k->fgBrush;

    qDrawShadeRect(&painter, bgRect, pal, k->currentSpace == Background, 2, 0, isEnabled() ? &bgAdjusted: &defBrush);
    qDrawShadeRect(&painter, fgRect,  pal, k->currentSpace == Foreground, 2, 0, isEnabled() ? &fgAdjusted : &defBrush);

    painter.setPen(QPen(palette().shadow().color()));
    painter.drawPixmap(fgRect.right() + 2, 0, k->arrowBitmap);
    painter.drawPixmap(0, fgRect.bottom() + 2, k->resetPixmap);
}

void KDualColorButton::mousePressEvent(QMouseEvent *event)
{
    QPoint mPos = event->pos();

    QRect fgRect, bgRect;
    metrics(fgRect, bgRect);

    if (fgRect.contains(mPos)) {
        k->currentSpace = Foreground;
        tFatal() << "KDualColorButton::mousePressEvent() - emitting foreground signal!";
        emit selectionChanged(Foreground);
    } else if (bgRect.contains(mPos)) {
               k->currentSpace = Background;
               tFatal() << "KDualColorButton::mousePressEvent() - emitting background signal!";
               emit selectionChanged(Background);
    } else if (event->pos().x() > fgRect.width()) {
               // We handle the swap and reset controls as soon as the mouse is
               // is pressed and ignore further events on this click (mosfet).
               QBrush tmpBrush = k->fgBrush;
               k->fgBrush = k->bgBrush;
               k->bgBrush = tmpBrush;
               emit switchColors();
               // emit fgChanged(k->fgBrush);
               // emit bgChanged(k->bgBrush);
    } else if (event->pos().x() < bgRect.x()) {
               k->fgBrush.setColor(Qt::black);
               k->bgBrush.setColor(QColor(0,0,0,0));
               emit resetColors();
               // emit fgChanged(k->fgBrush);
               // emit bgChanged(k->bgBrush);
    }

    update();
}
