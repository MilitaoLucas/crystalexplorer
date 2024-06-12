#pragma once

#include "colordelegate.h"
#include "crystalplane.h"
#include <occ/crystal/spacegroup.h>
#include <QDialog>
#include <QStandardItemModel>

namespace Ui {
class PlaneGenerationDialog;
}

class CrystalPlanesModel : public QAbstractTableModel {
  Q_OBJECT
public:
  CrystalPlanesModel(QObject *parent = nullptr) : QAbstractTableModel(parent) {}
  QList<CrystalPlane> planes;

  int rowCount(const QModelIndex &parent = QModelIndex()) const override;
  int columnCount(const QModelIndex &parent = QModelIndex()) const override;

  QVariant data(const QModelIndex &index,
                int role = Qt::DisplayRole) const override;
  QVariant headerData(int section, Qt::Orientation orientation,
                      int role = Qt::DisplayRole) const override;

  bool setData(const QModelIndex &index, const QVariant &value,
               int role = Qt::EditRole) override;

  Qt::ItemFlags flags(const QModelIndex &index) const override;

  void addPlane(const CrystalPlane &plane);
  void addPlanes(const std::vector<CrystalPlane> &);
  void removePlane(int row);
  void clear();
};

class PlaneGenerationDialog : public QDialog {
  Q_OBJECT

public:
  explicit PlaneGenerationDialog(QWidget *parent = nullptr);
  ~PlaneGenerationDialog();

  QColor color() const { return m_color; }
  int h() const;
  int k() const;
  int l() const;
  double offset() const;

  std::vector<CrystalPlane> planes() const;
  void loadPlanes(const std::vector<CrystalPlane> &planes);

  void setSpaceGroup(const occ::crystal::SpaceGroup &);

signals:
  void createSurfaceGeometry(CrystalPlane);

private slots:
  void onColorButtonClicked();
  void addPlaneFromCurrentSettings();
  void removeSelectedPlane();
  void removeAllPlanes();
  void createSurfaceGeometryButtonClicked();

private:
  Ui::PlaneGenerationDialog *ui;
  QColor m_color;
  CrystalPlanesModel *m_planesModel{nullptr};
  ColorDelegate *m_colorDelegate{nullptr};
  occ::crystal::SpaceGroup m_spaceGroup;
};
