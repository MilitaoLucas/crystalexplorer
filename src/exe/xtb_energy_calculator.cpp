#include "xtb_energy_calculator.h"
#include "exefileutilities.h"
#include "io_utilities.h"
#include "settings.h"
#include "xtb.h"
#include "xtbtask.h"
#include <QFile>
#include <QTextStream>
#include <occ/core/element.h>

XtbEnergyCalculator::XtbEnergyCalculator(QObject *parent) : QObject(parent) {
  m_xtbExecutable =
      settings::readSetting(settings::keys::XTB_EXECUTABLE).toString();
  m_environment = QProcessEnvironment::systemEnvironment();
  m_environment.insert("OMP_NUM_THREADS", "1");
  m_deleteWorkingFiles =
      settings::readSetting(settings::keys::DELETE_WORKING_FILES).toBool();
}

void XtbEnergyCalculator::setTaskManager(TaskManager *mgr) {
  m_taskManager = mgr;
}

void XtbEnergyCalculator::start(xtb::Parameters params) {
  if (!params.structure) {
    qDebug() << "Found nullptr for chemical structure in XtbEnergyCalculator";
    return;
  }
  auto idx = params.structure->atomsWithFlags(AtomFlag::Selected);
  occ::IVec nums = params.structure->atomicNumbersForIndices(idx);
  occ::Mat3N pos = params.structure->atomicPositionsForIndices(idx);

  if (params.name == "XtbCalculation") {
    params.name = xtb::methodToString(params.method);
  }
  if(params.userEditRequested) {
    params.userInputContents = io::requestUserTextEdit("XTB input", xtbCoordString(params));
    // TODO report to user that the job will be canceled
    if(params.userInputContents.isEmpty()) return;
  }
  auto *task = new XtbTask();
  task->setParameters(params);
  task->setProperty("name", params.name);
  task->setProperty("basename", params.name);
  task->setExecutable(m_xtbExecutable);
  task->setEnvironment(m_environment);
  task->setDeleteWorkingFiles(m_deleteWorkingFiles);

  auto taskId = m_taskManager->add(task);
  connect(
      task, &Task::completed,
      [&, taskPtr = QPointer<XtbTask>(task)]() {
        if (taskPtr) {
          this->handleFinishedTask(taskPtr->getParameters(), taskPtr->getResult());
        } else {
          qWarning() << "Task pointer for" << params.name
                     << "already deleted when completed signal was received?";
        }
      });
}

void XtbEnergyCalculator::handleFinishedTask(xtb::Parameters params,
                                             xtb::Result result) {
  qDebug() << "Task" << result.name << "finished in XtbEnergyCalculator";
  emit calculationComplete(params, result);
}
