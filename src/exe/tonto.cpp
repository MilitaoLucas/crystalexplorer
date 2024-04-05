#include "tonto.h"
#include "exefileutilities.h"

TontoTask::TontoTask(QObject *parent) : ExternalProgramTask(parent) {
    setExecutable("/Users/285699f/git/crystalexplorer/resources/tonto");
}

void TontoTask::appendBasisSetDirectoryBlock(QString &result) {
    // basis set information
    result.append("\n    basis_directory= \"" + basisSetDirectory() + "\"\n");
    QString slaterBasis = slaterBasisName();
    if (!slaterBasis.isEmpty()) {
	result.append("    slaterbasis_name= \"" + slaterBasis + "\"\n");
    }
}

void TontoTask::appendHeaderBlock(const QString &header, QString &result) {
    result.append("{\n    ! " + header + "\n");
}

void TontoTask::appendFooterBlock(QString &result) {
    result.append("\n}\n");
}


void TontoTask::appendCifDataBlock(const QString &dataBlockName, QString &result) {
    result.append("\n    ! Read the CIF and data block ...\n\n");
    result.append("    CIF= {\n        file_name= \"" + cifFileName() + "\"\n");
    if (!dataBlockName.isEmpty()) {
	result.append("        data_block_name= \"" + dataBlockName + "\"\n");
    }

  if (overrideBondLengths()) {
      // TODO fix settings
      result.append(QString("        CH_bond_length= %1 angstrom\n").arg(1.083f));
      result.append(QString("        NH_bond_length= %1 angstrom\n").arg(1.009f));
      result.append(QString("        OH_bond_length= %1 angstrom\n").arg(0.983f));
      result.append(QString("        BH_bond_length= %1 angstrom\n").arg(1.180f));
  }
  result.append("    }\n");
}

void TontoTask::appendChargeMultiplicityBlock(QString &dest) {
    dest.append(QString("\n    charge= %1\n    multiplicity= %2\n").arg(charge()).arg(multiplicity()));
}

bool TontoTask::overrideBondLengths() const {
    return properties().value("override_bond_lengths", true).toBool();
}

int TontoTask::charge() const {
    return properties().value("charge", 0).toInt();
}

int TontoTask::multiplicity() const {
    return properties().value("multiplicity", 1).toInt();
}

QString TontoTask::cifFileName() const {
    return properties().value("cif", "file.cif").toString();
}

QString TontoTask::basisSetDirectory() const {
    return properties().value("basis_directory", ".").toString();
}

QString TontoTask::slaterBasisName() const {
    return properties().value("slaterbasis_name", "").toString();
}

QString TontoTask::cxcFileName() const {
    return properties().value("cxc", "file.cxc").toString();
}

QString TontoTask::cxsFileName() const {
    return properties().value("cxs", "file.cxs").toString();
}

QString TontoTask::crystalName() const {
    return properties().value("crystal_name", "").toString();
}

void TontoTask::start() {
    QString name = baseName();

    if(!exe::writeTextFile("stdin", getInputText())) {
	emit errorOccurred("Could not write input file");
	return;
    }
    emit progressText("Wrote Tonto stdin file");

    FileDependencyList fin = requirements();
    fin.append(FileDependency("stdin"));

    FileDependencyList fout = outputs();
    fout.append(FileDependency("stdout"));

    setRequirements(fin);
    setOutputs(fout);

    emit progressText("Starting Tonto process");

    ExternalProgramTask::start();

    qDebug() << "Finish Tonto task start";
}

// CIF Processing

QString TontoCifProcessingTask::getInputText() {
    QString result;
    result.reserve(2048);
    appendHeaderBlock("Tonto input file for CIF Processing.", result);
    appendBasisSetDirectoryBlock(result);
    appendCifDataBlock("", result);
    result.append("    cx_uses_angstrom= true\n");
    result.append(QString("    CX_file_name= \"%1\"\n").arg(cxcFileName()));
    result.append("    process_CIF_for_CX\n");
    appendFooterBlock(result);
    return result;
}

TontoCifProcessingTask::TontoCifProcessingTask(QObject *parent) : TontoTask(parent) {}

void TontoCifProcessingTask::start() {
    setRequirements({FileDependency(cifFileName())});
    setOutputs({FileDependency(cxcFileName())});
    TontoTask::start();
}