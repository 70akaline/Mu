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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#include "knpreferencepanel.h"

#include "knpreference.h"

KNPreference::KNPreference(QObject *parent) :
    KNPreferencePlugin(parent)
{
    m_preferencePanel=new KNPreferencePanel;
    connect(m_preferencePanel, &KNPreferencePanel::requireHidePreference,
            this, &KNPreference::requireHidePreference);
}

QWidget *KNPreference::preferencePanel()
{
    return m_preferencePanel;
}

void KNPreference::addCategory(const QString &title,
                               const QPixmap &icon,
                               const QPixmap &headerIcon,
                               QWidget *contentWidget)
{
    m_preferencePanel->addCategory(title, icon, headerIcon, contentWidget);
}

void KNPreference::setCurrentIndex(const int &index)
{
    m_preferencePanel->setCurrentIndex(index);
}
