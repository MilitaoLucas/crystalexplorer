#include "externalprogram.h"
#include "exefileutilities.h"
#include <QByteArray>
#include <QDebug>
#include <QFileInfo>
#include <QProcess>
#include <QTextStream>

namespace exe {

QString errorString(const QProcess::ProcessError &errorType) {
  switch (errorType) {
  case QProcess::FailedToStart:
    return "Process failed to start";
    break;
  case QProcess::Crashed:
    return "Process crashed during execution";
    break;
  case QProcess::Timedout:
    return "Process timeout";
    break;
  case QProcess::ReadError:
    return "Process read error";
    break;
  case QProcess::WriteError:
    return "Process write error";
    break;
  case QProcess::UnknownError:
  default:
    return "Unknown process error";
    break;
  }
}

} // namespace exe

ExternalProgramTask::ExternalProgramTask(QObject *parent)
    : Task(parent), m_environment(QProcessEnvironment::systemEnvironment()) {}

void ExternalProgramTask::cleanupResources() {
  // Delete the temporary directory if it exists
  if (m_tempDir) {
    delete m_tempDir;
    m_tempDir = nullptr;
  }

  // Clear any other resources if needed
  m_requirements.clear();
  m_outputs.clear();
}

ExternalProgramTask::~ExternalProgramTask() { cleanupResources(); }

QString ExternalProgramTask::getInputFilePropertyName(QString filename) {
  return "inp: " + filename;
}

QString ExternalProgramTask::getOutputFilePropertyName(QString filename) {
  return "out: " + filename;
}
void ExternalProgramTask::setExecutable(const QString &exe) {
  m_executable = exe;
}

void ExternalProgramTask::setArguments(const QStringList &args) {
  m_arguments = args;
}

void ExternalProgramTask::updateStdoutStderr(QProcess &process) {
  QString out = properties().value("stdout", "").toString();
  QString err = properties().value("stderr", "").toString();
  out += process.readAllStandardOutput();
  err += process.readAllStandardError();
  setProperty("stdout", out);
  setProperty("stderr", err);
  emit stdoutChanged();
}

void ExternalProgramTask::setupProcessConnectionsPrivate(QProcess &process) {
  QObject::connect(
      &process, &QProcess::finished,
      [this, &process](int exitCode, QProcess::ExitStatus exitStatus) {
        if (exitStatus == QProcess::CrashExit) {
          setErrorMessage("Process crashed");
        } else {
          m_exitCode = exitCode;
          updateStdoutStderr(process);
        }
      });

  QObject::connect(&process, &QProcess::errorOccurred, this,
                   [this](QProcess::ProcessError error) {
                     // Populate errorMessage based on the type of error
                     switch (error) {
                     case QProcess::Timedout:
                       // timing out is not an error due to the way we're
                       // running
                       break;
                     default:
                       setErrorMessage(exe::errorString(error));
                       break;
                     }
                   });

  QObject::connect(this, &ExternalProgramTask::stopProcess, &process,
                   &QProcess::terminate);
}

bool ExternalProgramTask::copyRequirements(const QString &path) {
  bool force = overwrite();
  for (const auto &[input, input_dest] : m_requirements) {
    QString dest = path + QDir::separator() + QFileInfo(input_dest).fileName();
    qDebug() << "Copying " << input << "to" << dest;
    if (!io::copyFile(input, dest, force)) {
      setErrorMessage(
          QString("Failed to copy input file to temporary directory: %1 -> %2")
              .arg(input)
              .arg(dest));
      qDebug() << errorMessage();
      return false;
    }
    setProperty(getInputFilePropertyName(input), exe::readFileContents(input));
  }
  return true;
}

bool ExternalProgramTask::copyResults(const QString &path) {
  bool force = overwrite();
  // List contents of the temp directory
  QDir tempDir(path);
  qDebug() << "Contents of temporary directory" << path << ":";
  for (const auto &entry :
       tempDir.entryList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot)) {
    qDebug() << "  " << entry;
  }
  for (const auto &[output, output_dest] : m_outputs) {
    QString tmpOutput = path + QDir::separator() + QFileInfo(output).fileName();
    qDebug() << "Copying " << tmpOutput << "to" << output_dest;
    if (!io::copyFile(tmpOutput, output_dest, force)) {
      setErrorMessage(
          QString(
              "Failed to copy output file from temporary directory. %1 -> %2")
              .arg(tmpOutput)
              .arg(output_dest));
      qDebug() << errorMessage();
      return false;
    }
    setProperty(getOutputFilePropertyName(output),
                exe::readFileContents(tmpOutput));
  }
  return true;
}

bool ExternalProgramTask::deleteRequirements() {
  for (const auto &[input, input_dest] : m_requirements) {
    if (!io::deleteFile(input)) {
      setErrorMessage(QString("Failed to delete working files %1").arg(input));
      qDebug() << errorMessage();
      return false;
    }
  }
  return true;
}

void ExternalProgramTask::preProcess() {
  // Default implementation - can be overridden in derived classes
}

void ExternalProgramTask::postProcess() {
  // Default implementation - can be overridden in derived classes
}

bool ExternalProgramTask::runExternalProgram(QPromise<void> &promise) {
  QString exe = m_executable;
  QStringList args = m_arguments;
  {
    QProcess process;
    qDebug() << "In task logic";

    if (m_tempDir) {
      delete m_tempDir; // Clean up any previous instance
    }

    m_tempDir = new QTemporaryDir();

    if (!m_tempDir->isValid()) {
      setErrorMessage("Cannot create temporary directory");
      promise.finish();
      return false;
    }
    promise.setProgressValueAndText(1, "Temporary directory created");

    process.setProcessEnvironment(m_environment);
    process.setWorkingDirectory(m_tempDir->path());
    promise.setProgressValueAndText(2, "Process environment set");

    setupProcessConnectionsPrivate(process);

    if (!copyRequirements(m_tempDir->path())) {
      setErrorMessage(
          "Could not copy necessary files into temporary directory");
      return false;
    }
    promise.setProgressValueAndText(3, "Copied files to temporary directory");
    process.start(exe, args);
    promise.setProgressValueAndText(4, "Starting background process");
    process.waitForStarted();
    promise.setProgressValueAndText(5, "Background process started");

    int timeTaken = 0;

    while (!process.waitForFinished(m_timeIncrement)) {
      timeTaken += m_timeIncrement;
      updateStdoutStderr(process);
      promise.setProgressValueAndText(timeTaken / m_timeIncrement,
                                      QString("Running %1").arg(m_executable));
      if (promise.isCanceled()) {
        setErrorMessage("Promise was canceled");
        process.kill();
        promise.setProgressValueAndText(100, "Promise canceled");
        return false;
      }
      if (m_timeout > 0) {
        if (timeTaken > m_timeout) {
          setErrorMessage("Process timeout");
          promise.setProgressValueAndText(
              100, "Background process canceled due to timeout");
          process.kill();
          return false;
        }
      }

      if (process.error() != QProcess::Timedout) {
        promise.setProgressValueAndText(100,
                                        "Background process failed: " +
                                            exe::errorString(process.error()));
        process.kill();
        return false;
      }
    }
    promise.setProgressValueAndText(90, "Background process complete");
    updateStdoutStderr(process);

    // Explicitly close all channels before process object is destroyed
    process.closeReadChannel(QProcess::StandardOutput);
    process.closeReadChannel(QProcess::StandardError);
    process.closeWriteChannel();

    // Make sure the process has terminated
    if (process.state() != QProcess::NotRunning) {
      process.terminate();
      if (!process.waitForFinished(3000)) { // 3 second timeout
        process.kill();
        process.waitForFinished(1000);
      }
    }
  }

  // SUCCESS
  if (m_exitCode == 0) {
    if (!copyResults(m_tempDir->path())) {
      setErrorMessage("Could not copy results out of temporary directory");
    }
  } else {
    setErrorMessage(QString("Failed with exit code: %1").arg(m_exitCode));
  }
  return true;
}

// In the cpp file:
void ExternalProgramTask::start() {
  auto taskLogic = [this](QPromise<void> &promise) {
    preProcess();

    if (!runExternalProgram(promise)) {
      promise.finish();
      return;
    }

    promise.setProgressValueAndText(95, "Begin any post-processing steps");
    postProcess();

    if (deleteWorkingFiles())
      deleteRequirements();

    promise.setProgressValueAndText(100, "Task complete");
    qDebug() << "Task " << property("name").toString() << " finished"
             << errorMessage();

    promise.finish();
  };

  Task::run(taskLogic);
}

void ExternalProgramTask::stop() { emit stopProcess(); }

void ExternalProgramTask::setEnvironment(const QProcessEnvironment &env) {
  m_environment = env;
}

void ExternalProgramTask::setRequirements(
    const FileDependencyList &requirements) {
  m_requirements = requirements;
}

void ExternalProgramTask::setOutputs(const FileDependencyList &outputs) {
  m_outputs = outputs;
}

QString ExternalProgramTask::baseName() const {
  const auto &props = properties();
  const auto loc = props.find("basename");
  if (loc != props.end()) {
    return loc->toString();
  }
  return "external_calculation";
}

QString ExternalProgramTask::hashedBaseName() const {
  return QString::number(qHash(baseName()), 16);
}

void ExternalProgramTask::setOverwrite(bool overwrite) {
  setProperty("overwrite", true);
}

bool ExternalProgramTask::overwrite() const {
  return properties().value("overwrite", true).toBool();
}
