#pragma once
#include <QStringList>
#include <QToolButton>
#include <QWidget>

#include "fingerprintplot.h" // FingerprintFilterMode, PlotRange enums
#include "ui_fingerprintoptions.h"

const QString NONE_ELEMENT_LABEL = "All";

class FingerprintOptions : public QWidget, public Ui::FingerprintOptions {
  Q_OBJECT

public:
  FingerprintOptions(QWidget *parent = 0);
  void setElementList(QStringList);
  void resetOptions();

signals:
  void plotRangeChanged(FingerprintPlotRange);
  void filterChanged(FingerprintFilterOptions);
  void saveFingerprint(QString);
  void closeClicked();

public slots:
  void updateSurfaceAreaProgressBar(double);

private slots:
  void updatePlotRange(int);
  void updateFilterMode();
  void updateFilterSettings();
  void getFilenameAndSaveFingerprint();
  void updateVisibilityOfFilterWidgets(int);

private:
  void init();
  QStringList filterOptions();
  QStringList plotRangeLabels();
  void initConnections();
  void enableSignalsForWidgets(bool);
  void resetFilter();
  void updateVisibilityOfFilterWidgets(FingerprintFilterMode);
  void setVisibleElementFilteringWidgets(bool);
  void setVisibleSelectionFilteringWidgets(bool);
  void setVisibleCommonFilteringWidgets(bool);
  void setVisibleRangeFilteringWidgets(bool);
  QColor getButtonColor(QToolButton *);
  void setButtonColor(QToolButton *, QColor);
  FingerprintFilterMode getFilterMode();
  void resetElementFilterOptions();
};
