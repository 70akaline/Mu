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
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KNMAINWINDOW_H
#define KNMAINWINDOW_H

#include <QMainWindow>

class QPropertyAnimation;
class QSequentialAnimationGroup;
class KNAbstractMusicPlugin;
class KNConfigure;
class KNCategoryPlugin;
class KNMainWindowContainer;
class KNMainWindowHeaderBase;
class KNNotificationCenter;
class KNOpacityAnimeButton;
class KNPreferencePlugin;
/*!
 * \brief The KNMainWindow class provides a main application window for mu.\n
 * It contains several parts: header, main widget and preference.
 *  * Header\n
 * The header parts of the application. Managed header player and search box.
 *  * Main widget\n
 * The main part of the widget. It contains the categories and music shops. It
 * should be a kind of tab manager.
 *  * Preference\n
 * The preference panel. It contains all the preference settings.
 */
class KNMainWindow : public QMainWindow
{
    Q_OBJECT
public:
    /*!
     * \brief Construct KNMainWindow class.
     * \param parent The parent widget object.
     */
    explicit KNMainWindow(QWidget *parent = 0);

    /*!
     * \brief Set the header widget.
     * \param header The header widget pointer. It will only save the first
     * widget you set.
     */
    void setHeader(KNMainWindowHeaderBase *header);

    /*!
     * \brief Set the main widget.
     * \param mainWidget The main widget pointer. It will only save the first
     * widget you set.
     */
    void setMainWidget(KNCategoryPlugin *mainWidget);

    /*!
     * \brief Set the preference panel widget.
     * \param preferencePanel The perference panel widget pointer. It will only
     * save the first widget you set.
     */
    void setPreferencePanel(KNPreferencePlugin *preferencePanel);

    /*!
     * \brief Set the music plugin to main window for main player and close
     * checking.
     * \param musicPlugin The music plugin plugin pointer.
     */
    void setMusicPlugin(KNAbstractMusicPlugin *musicPlugin);

    /*!
     * \brief Add one widget to the header status bar.
     * \param widget The widget pointer.
     */
    void addStatusWidget(QWidget *widget);

    /*!
     * \brief Add the notification widget to the header.
     * \param widget The widget pointer.
     */
    void addNotificationWidget(QWidget *widget);

signals:
    /*!
     * \brief When the main window is showing, this signal will be emitted.
     */
    void mainWindowShown();

    /*!
     * \brief When the notification widget has been shown completely, this
     * signal will be emitted to notify the notification backends.
     */
    void notificationComplete();

    /*!
     * \brief Ask to save the configurations to the hard drive.
     */
    void requireSaveConfigure();

public slots:
    /*!
     * \brief Show up the main player widget.
     */
    void showMainPlayer();

    /*!
     * \brief Hide the main player widget.
     */
    void hideMainPlayer();

    /*!
     * \brief This slot is called when the system tray called to close the
     * window.
     */
    void forceClose();

    /*!
     * \brief Save the main window configuration, it will also ask the category
     * plugin to save the configure.
     */
    void saveConfigure();

protected:
    /*!
     * \brief Reimplemented from QMainWindow::event().
     */
    bool event(QEvent *event) Q_DECL_OVERRIDE;

    /*!
     * \brief Reimplemented from QMainWindow::showEvent().
     */
    void showEvent(QShowEvent *event) Q_DECL_OVERRIDE;

    /*!
     * \brief Reimplemented from QMainWindow::resizeEvent().
     */
    void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;

    /*!
     * \brief Reimplemented from QMainWindow::closeEvent().
     */
    void closeEvent(QCloseEvent *event) Q_DECL_OVERRIDE;

private slots:
    void onActionFullScreen();
    void onActionShowNotificationCenter();
    void onActionHideComplete();
    void onActionPopupNotification();

private:
    inline void recoverGeometry();
    inline void backupGeometry();
    inline QPropertyAnimation *generateAnime();
    inline void updateAnimeStartAndEnd();
    inline int getCacheValue(const QString &valueName);
    inline void setCacheValue(const QString &valueName, int value);
    inline void zoomParameter(int &parameter, const qreal &ratio);

    QIcon m_fullScreenIcon, m_fullScreenOffIcon;
    QRect m_geometryBeforeMinimize;
    KNMainWindowHeaderBase *m_header;
    KNAbstractMusicPlugin *m_musicPlugin;
    KNConfigure *m_cacheConfigure, *m_globalConfigure, *m_trayConfigure;
    KNMainWindowContainer *m_container;
    KNCategoryPlugin *m_categoryPlugin;
    KNNotificationCenter *m_notificationCenter;
    QPropertyAnimation *m_inAnime, *m_outAnime;
    QSequentialAnimationGroup *m_outAndInAnime;
    QTimer *m_notificationWaiter;
    KNOpacityAnimeButton *m_fullScreen;
    Qt::WindowStates m_originalWindowState;
    bool m_ignoreTrayClose, m_initialShown;
};

#endif // KNMAINWINDOW_H
