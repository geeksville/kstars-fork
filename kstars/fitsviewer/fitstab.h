/*
    SPDX-FileCopyrightText: 2012 Jasem Mutlaq <mutlaqja@ikarustech.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "fitscommon.h"
#include "fitshistogrameditor.h"

#include <QUndoStack>
#include <QSplitter>
#include <QToolBox>
#include <QUrl>
#include <QWidget>
#include "ui_fitsheaderdialog.h"
#include "ui_statform.h"
#include "ui_platesolve.h"
#include "ui_livestacking.h"
#include "ui_catalogobject.h"
#include "ui_catalogobjecttypefilter.h"
#include <QFuture>
#include <QPointer>
#include <QListWidget>
#include <QLabel>
#include <QPushButton>
#include <memory>
#include "ekos/auxiliary/solverutils.h"
#include <KConfigDialog>
#include <QNetworkAccessManager>
#include <QStandardItemModel>

class FITSHistogramEditor;
class FITSView;
class FITSViewer;
class FITSData;
class FITSStretchUI;

namespace Ekos
{
class StellarSolverProfileEditor;
}

/**
 * @brief The FITSTab class holds information on the current view (drawing area) in addition to the undo/redo stacks
 *  and status of current document (clean or dirty). It also creates the corresponding histogram associated with the
 *  image data that is stored in the FITSView class.
 * @author Jasem Mutlaq
 */
class FITSTab : public QWidget
{
        Q_OBJECT
    public:
        explicit FITSTab(FITSViewer *parent);
        virtual ~FITSTab() override;

        enum
        {
            STAT_WIDTH,
            STAT_HEIGHT,
            STAT_BITPIX,
            STAT_HFR,
            STAT_MIN,
            STAT_MAX,
            STAT_MEAN,
            STAT_MEDIAN,
            STAT_STDDEV
        };

        void clearRecentFITS();
        void selectRecentFITS(int i);
        void loadFile(const QUrl &imageURL, FITSMode mode = FITS_NORMAL, FITSScale filter = FITS_NONE);
        // JEE
        void loadStack(const QString &dir, FITSMode mode = FITS_NORMAL, FITSScale filter = FITS_NONE);
        bool loadData(const QSharedPointer<FITSData> &data, FITSMode mode = FITS_NORMAL, FITSScale filter = FITS_NONE);

        // Methods to setup and control blinking--loading a directory of images one-by-one
        // into a single tab.
        void initBlink(const QList<QString> &filenames)
        {
            m_BlinkFilenames = filenames;
        }
        const QList<QString> &blinkFilenames() const
        {
            return m_BlinkFilenames;
        }
        int blinkUpto() const
        {
            return m_BlinkIndex;
        };
        void setBlinkUpto(int index)
        {
            if (index >= 0 && index < m_BlinkFilenames.size())
                m_BlinkIndex = index;
        };
        bool saveImage(const QString &filename);

        inline QUndoStack *getUndoStack()
        {
            return undoStack;
        }
        inline QUrl *getCurrentURL()
        {
            return &currentURL;
        }
        inline const QSharedPointer<FITSView> &getView()
        {
            return m_View;
        }
        inline QPointer<FITSHistogramEditor> getHistogram()
        {
            return m_HistogramEditor;
        }
        inline QPointer<FITSViewer> getViewer()
        {
            return viewer;
        }

        bool saveFile();
        bool saveFileAs();
        void copyFITS();
        void loadFITSHeader();
        void loadCatalogObjects();
        void queriedCatalogObjects();
        void catQueryFailed(const QString text);
        void catReset();
        void catHighlightChanged(const int highlight);
        void catHighlightRow(const int row);
        void headerFITS();
        void histoFITS();
        void statFITS();

        Q_SCRIPTABLE void setStretchValues(double shadows, double midtones, double highlights);
        Q_SCRIPTABLE void setAutoStretch();

        void setUID(int newID)
        {
            uid = newID;
        }
        int getUID()
        {
            return uid;
        }

        void saveUnsaved();
        void tabPositionUpdated();
        void selectGuideStar();

        QString getPreviewText() const;
        void setPreviewText(const QString &value);
        bool shouldComputeHFR() const;

    public slots:
        void modifyFITSState(bool clean = true, const QUrl &imageURL = QUrl());
        void ZoomIn();
        void ZoomOut();
        void ZoomDefault();
        void displayStats(bool roi = false);
        void extractImage();
        void solveImage();
        void liveStack();
    protected:
        virtual void closeEvent(QCloseEvent *ev) override;

    private:
        bool setupView(FITSMode mode, FITSScale filter);
        void processData();
        void imageSolved(bool success);

        /** Ask user whether he wants to save changes and save if he do. */

        /// The FITSTools Toolbox
        QPointer<QToolBox> fitsTools;
        /// The Splitter for th FITSTools Toolbox
        QPointer<QSplitter> fitsSplitter;
        /// The FITS Header Panel
        QPointer<QDialog> fitsHeaderDialog;
        Ui::fitsHeaderDialog header;
        /// The Statistics Panel
        QPointer<QDialog> statWidget;
        Ui::statForm stat;
        /// The Plate Solving UI
        QPointer<QDialog> m_PlateSolveWidget;
        Ui::PlateSolveUI m_PlateSolveUI;
        /// The Live Stacking UI
        QPointer<QDialog> m_LiveStackingWidget;
        Ui::LiveStackingUI m_LiveStackingUI;
        /// Catalog Object UI
        QPointer<QDialog> m_CatalogObjectWidget;
        Ui::CatalogObjectUI m_CatalogObjectUI;
        QPointer<QDialog> m_CatObjTypeFilterDialog;
        Ui::CatalogObjectTypeFilterUI m_CatObjTypeFilterUI;
        /// FITS Histogram
        QPointer<FITSHistogramEditor> m_HistogramEditor;
        QPointer<FITSViewer> viewer;

        QPointer<QListWidget> recentImages;

        /// FITS image object
        QSharedPointer<FITSView> m_View;

        /// History for undo/redo
        QUndoStack *undoStack { nullptr };
        /// FITS File name and path
        QUrl currentURL;

        bool mDirty { false };
        QString previewText;
        int uid { 0 };

        std::unique_ptr<FITSStretchUI> stretchUI;

        // Used for solving an image.
        void setupSolver(bool extractOnly = false);
        void solverDone(bool timedOut, bool success, const FITSImage::Solution &solution, double elapsedSeconds);
        void extractorDone(bool timedOut, bool success, const FITSImage::Solution &solution, double elapsedSeconds);
        void initSolverUI();
        void setupProfiles(int profileIndex);
        int getProfileIndex(int moduleIndex);
        void setProfileIndex(int moduleIndex, int profileIndex);

        // Used for catalog table processing
        typedef enum { CAT_NUM,
                       CAT_CDSPORTAL,
                       CAT_SIMBAD,
                       CAT_NED,
                       CAT_OBJECT,
                       CAT_TYPECODE,
                       CAT_TYPELABEL,
                       CAT_COORDS,
                       CAT_DISTANCE,
                       CAT_MAGNITUDE,
                       CAT_SIZE,
                       CAT_MAX_COLS
                     } CatCols;

        typedef enum { CATTYPE_CODE,
                       CATTYPE_CANDCODE,
                       CATTYPE_LABEL,
                       CATTYPE_DESCRIPTION,
                       CATTYPE_COMMENTS,
                       CATTYPE_MAX_COLS
                     } CatTypeCols;

        void catRowChanged(const QModelIndex &current, const QModelIndex &previous);
        void catCellDoubleClicked(const QModelIndex &index);
        void launchCatTypeFilterDialog();
        void showCatObjNames(bool enabled);
        void launchSimbad(QString name);
        void launchCDS(QString name);
        void launchNED(QString name);
        void initCatalogObject();
        void setupCatObjTypeFilter();
        void initLiveStacking();
        void applyTypeFilter();
        void checkAllTypeFilter();
        void uncheckAllTypeFilter();
        void typeFilterItemChanged(QTreeWidgetItem *item, int column);
        QPushButton *m_CheckAllButton;
        QPushButton *m_UncheckAllButton;
        int m_CatalogObjectItem { 0 };
        QStandardItemModel m_CatObjModel;
        // JEE
        int m_LiveStackingItem { 0 };

        QSharedPointer<SolverUtils> m_Solver;

        QList<QString> m_BlinkFilenames;
        int m_BlinkIndex { 0 };

        // The StellarSolverProfileEditor is shared among all tabs of all FITS Viewers.
        // They all edit the same (align) profiles.
        static QPointer<Ekos::StellarSolverProfileEditor> m_ProfileEditor;
        static QPointer<KConfigDialog> m_EditorDialog;
        static QPointer<KPageWidgetItem> m_ProfileEditorPage;

        // JEE
        void plateSolveImage(const double ra, const double dec, const double pixScale);
        bool m_Stack { false };
        QString m_liveStackDir;

    signals:
        void debayerToggled(bool);
        void newStatus(const QString &msg, FITSBar id);
        void changeStatus(bool clean, const QUrl &imageUrl);
        void loaded();
        void updated();
        void failed(const QString &errorMessage);
};
