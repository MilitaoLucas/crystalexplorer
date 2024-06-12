#pragma once
#include <QObject>
#include "taskmanager.h"
#include "chemicalstructure.h"
#include "isosurface_parameters.h"

namespace volume {

class IsosurfaceCalculator: public QObject {
    Q_OBJECT
public:
    IsosurfaceCalculator(QObject * parent = nullptr);
    
    void setTaskManager(TaskManager *);
    void start(isosurface::Parameters);

signals:
    void calculationComplete(isosurface::Result);

private slots:
    void surfaceComplete();

private:
    TaskManager * m_taskManager{nullptr};
    ChemicalStructure * m_structure{nullptr};
    QString m_name;
    QString m_filename;
    std::vector<GenericAtomIndex> m_atomsInside;
    std::vector<GenericAtomIndex> m_atomsOutside;
    occ::IVec m_nums_inside;
    occ::IVec m_nums_outside;
};

}
