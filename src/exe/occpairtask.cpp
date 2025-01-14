#include "occpairtask.h"
#include "exefileutilities.h"
#include "filedependency.h"
#include "settings.h"

OccPairTask::OccPairTask(QObject *parent) : ExternalProgramTask(parent) {
  setExecutable(
      settings::readSetting(settings::keys::ORCA_EXECUTABLE).toString());
}

void OccPairTask::setParameters(const pair_energy::Parameters &params) {
  m_parameters = params;
  setProperty("basename", params.deriveName());
}

void OccPairTask::appendTransformArguments(QStringList &args) {
  const auto ta = m_parameters.transformA.matrix();

  qDebug() << "Matrix A:";
  for (int i = 0; i < 4; i++) {
    qDebug() << ta(i, 0) << ta(i, 1) << ta(i, 2) << ta(i, 3);
  }

  for (int i = 0; i < 3; i++) {
    args << QString("--translation-a=%1").arg(ta(i, 3));
  }

  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++) {
      args << QString("--rotation-a=%1").arg(ta(i, j));
    }
  }

  const auto tb = m_parameters.transformB.matrix();

  qDebug() << "Matrix B:";
  for (int i = 0; i < 4; i++) {
    qDebug() << tb(i, 0) << tb(i, 1) << tb(i, 2) << tb(i, 3);
  }

  for (int i = 0; i < 3; i++) {
    args << QString("--translation-b=%1").arg(tb(i, 3));
  }

  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++) {
      args << QString("--rotation-b=%1").arg(tb(i, j));
    }
  }
}

QString OccPairTask::jsonFilename() const {
  return hashedBaseName() + "_energies.json";
}

void OccPairTask::start() {
  if (!(m_parameters.wfnA && m_parameters.wfnB)) {
    qWarning() << "Invalid wavefunctions specified";
    return;
  }

  QString name = hashedBaseName();

  QString nameA =
      QString("%1_A%2").arg(name).arg(m_parameters.wfnA->fileSuffix());
  QString nameB =
      QString("%1_B%2").arg(name).arg(m_parameters.wfnB->fileSuffix());

  const QString jsonName = jsonFilename();

  emit progressText("Writing wavefunction files to disk");

  qDebug() << "Writing " << nameA;
  m_parameters.wfnA->writeToFile(nameA);
  qDebug() << "Writing " << nameB;
  m_parameters.wfnB->writeToFile(nameB);

  QStringList args{"pair", "-a", nameA, "-b", nameB};

  QList<FileDependency> reqs{FileDependency(nameA), FileDependency(nameB)};

  args << QString("--threads=%1").arg(threads());
  args << QString("--model=%1").arg(m_parameters.model);
  // args << QString("--verbosity=4");
  args << QString("--json=%1").arg(jsonName);

  appendTransformArguments(args);

  qDebug() << "Arguments:" << args;
  setArguments(args);
  setRequirements(reqs);
  setOutputs({FileDependency(jsonName, jsonName)});

  emit progressText("Starting OCC process");
  ExternalProgramTask::start();
  qDebug() << "Finish occ task start";
}

int OccPairTask::threads() const {
  return properties().value("threads", 6).toInt();
}
