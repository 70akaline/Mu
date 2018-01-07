/*
 * Copyright (C) Kreogist Dev Team
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#include <QBoxLayout>
#include <QTimeLine>
#include <QPainter>
#include <QLabel>

#include "knopacitybutton.h"
#include "knimagelabel.h"
#include "knthememanager.h"
#include "knconfiguremanager.h"
#include "kndpimanager.h"

#include "knpreferencepanelitem.h"

#include <QDebug>

#define PreferenceSingleItemHeight  34
#define PreferenceCaptionWidth      185

KNPreferencePanelItem::KNPreferencePanelItem(QWidget *parent) :
    QWidget(parent),
    m_path(QStringList()),
    m_defaultValue(QVariant()),
    m_highlight(QLinearGradient(
                    QPointF(0, 0),
                    QPointF(0, knDpi->height(PreferenceSingleItemHeight)))),
    m_highLightOpacity(0.0),
    m_titleLabel(new QLabel(this)),
    m_hintLabel(new KNImageLabel(this)),
    m_undoButton(new KNOpacityButton(this)),
    m_backgroundAnime(new QTimeLine(100, this))
{
    //Set properties.
    setContentsMargins(0, 0, 0, 0);
    //Set the palette.
    QPalette pal=knTheme->getPalette("PreferencePanelItem");
    setPalette(pal);
    //Configure the label.
    m_titleLabel->setFixedSize(knDpi->size(PreferenceCaptionWidth,
                                           PreferenceSingleItemHeight));
    m_titleLabel->setContentsMargins(knDpi->margins(10, 0, 10, 0));

    //Configure the hint label.
    m_hintLabel->setFixedSize(knDpi->size(PreferenceSingleItemHeight,
                                          PreferenceSingleItemHeight));
    m_hintLabel->setPixmap(QPixmap("://preference/PreferenceHint.png"));
    //Configure the hint label.
    m_undoButton->setFixedSize(knDpi->size(PreferenceSingleItemHeight,
                                           PreferenceSingleItemHeight));
    m_undoButton->setIcon(QIcon("://preference/PreferenceUndo.png"));
    connect(m_undoButton, &KNOpacityButton::clicked,
            [=]
            {
                //Hide the undo button.
                m_undoButton->setEnabled(false);
                //Get the configure value.
                setWidgetValue(m_defaultValue);
                //Update the signal.
                emit valueChanged();
            });
    //Initial the highlight.
    m_highlight.setColorAt(0, pal.color(QPalette::AlternateBase));
    m_highlight.setColorAt(1, pal.color(QPalette::Window));

    //Link the value changed.
    connect(this, &KNPreferencePanelItem::valueChanged,
            this, &KNPreferencePanelItem::updateConfigureData);

    //Update the panel.
    m_backgroundAnime->setUpdateInterval(16);
    connect(m_backgroundAnime, &QTimeLine::frameChanged,
            this, &KNPreferencePanelItem::onActionChangeHighlight);
    onActionChangeHighlight(0);
}

void KNPreferencePanelItem::setConfig(const QVariant &config)
{
    //Default, the preference item doesn't need any configure.
    Q_UNUSED(config)
}

void KNPreferencePanelItem::setValue(const QVariant &value)
{
    //Check the value data.
    setWidgetValue(value.isNull() ? m_defaultValue : value);
}

void KNPreferencePanelItem::setPreferenceOption(
        const PreferencePanelOption &option)
{
    //Update the height.
    setFixedHeight(knDpi->height(itemHeightMultiple() *
                                 PreferenceSingleItemHeight));
    //Update the title.
    m_titleLabel->setText(option.title);
    //Set the hint.
    m_hintLabel->setDisplayContent(!option.explain.isEmpty());
    m_hintLabel->setToolTip(option.explain);
    //Save the path.
    m_path=option.path;
    //Set the configure data.
    setConfig(option.configure);
    //Save the default value.
    m_defaultValue=getValueFromConfig(option.defaultValue);
    //Get the configure value.
    initialValue(m_defaultValue);
    //Disable the value.
    m_undoButton->setEnabled(false);
}

int KNPreferencePanelItem::itemHeightMultiple() const
{
    return 1;
}

void KNPreferencePanelItem::paintEvent(QPaintEvent *event)
{
    //Update the widget.
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing |
                           QPainter::TextAntialiasing |
                           QPainter::SmoothPixmapTransform,
                           true);
    //Draw the rect.
    painter.setOpacity(m_highLightOpacity);
    //Paint the background.
    painter.fillRect(QRect(0, 0,
                           width(), knDpi->height(PreferenceSingleItemHeight)),
                     m_highlight);
    //Paint other things.
    QWidget::paintEvent(event);
}

void KNPreferencePanelItem::enterEvent(QEvent *event)
{
    //Do original the event.
    QWidget::enterEvent(event);
    //Start the mouse in anime.
    startAnime(100);
}

void KNPreferencePanelItem::leaveEvent(QEvent *event)
{
    //Do original the event.
    QWidget::leaveEvent(event);
    //Start the mouse out anime.
    startAnime(0);
}

void KNPreferencePanelItem::buildWidgetLayout(QLayout *layout)
{
    //Create the widget layout, and set the layout to the widget.
    setLayout(createWidgetLayout(layout));
}

QLayout *KNPreferencePanelItem::createWidgetLayout(QLayout *layout)
{
    //Construct the layout.
    QBoxLayout *mainLayout=new QBoxLayout(QBoxLayout::LeftToRight);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    //Add the basic widgets.
    mainLayout->addWidget(m_titleLabel, 0, Qt::AlignVCenter);
    //Add the undo button.
    mainLayout->addWidget(m_undoButton, 0, Qt::AlignVCenter);
    //Add the content layout to widget.
    mainLayout->addLayout(layout, 1);
    //Add the helper widgets.
    mainLayout->addWidget(m_hintLabel, 0, Qt::AlignVCenter);
    //Give back the main layout.
    return mainLayout;
}

QVariant KNPreferencePanelItem::getValueFromConfig(const QVariant &defaultValue)
{
    //Get the default value from the configure.
    return knConf->configureValue(m_path, defaultValue);
}

void KNPreferencePanelItem::initialValue(QVariant &defaultValue)
{
    //For the default item, it is simply set the value.
    setWidgetValue(defaultValue);
}

QString KNPreferencePanelItem::titleText() const
{
    return m_titleLabel->text();
}

QString KNPreferencePanelItem::valueName() const
{
    return m_path.last();
}

void KNPreferencePanelItem::updateConfigureData()
{
    //Get the current value.
    QVariant currentValue=value();
    //Check the value state.
    m_undoButton->setEnabled(!isEqual(currentValue, m_defaultValue));
    //Save the value.
    knConf->setConfigureValue(m_path, currentValue);
}

void KNPreferencePanelItem::onActionChangeHighlight(int frame)
{
    //Change the highlight opacity.
    m_highLightOpacity=(qreal)frame/100.0;
    //Update the background.
    update();
}

inline void KNPreferencePanelItem::startAnime(int endFrame)
{
    //Stop the time line.
    m_backgroundAnime->stop();
    //Set the end frame.
    m_backgroundAnime->setFrameRange(m_highLightOpacity * 100,
                                     endFrame);
    //Start the anime.
    m_backgroundAnime->start();
}
