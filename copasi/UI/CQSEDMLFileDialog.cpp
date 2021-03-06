/*
 * CQSEDMLFileDialog.cpp
 *
 *  Created on: 31 Jul 2013
 *      Author: dada
 */

#include <QtCore/QRegExp>

#include "copasi.h"

#include "CQSEDMLFileDialog.h"
#include "CopasiFileDialog.h"

// static
std::pair< QString, std::pair< unsigned C_INT32, unsigned C_INT32 > > CQSEDMLFileDialog::getSaveFileName(QWidget * parent,
    const char * name,
    const QString & startWith,
    const QString & caption,
    unsigned int sedmlLevel,
    unsigned int sedmlVersion)
{
  std::pair< QString, std::pair< unsigned C_INT32, unsigned C_INT32 > > NameAndVersion;

  QString Filter = "Level 1 Version 1 (*.xml)";

  QString SelectedFilter =
    QString("Level %1 Version %1 (*.xml)").arg(QString::number(sedmlLevel)).arg(QString::number(sedmlVersion));;

  // The default export is L1V1
  if (Filter.indexOf(SelectedFilter) == -1)
    {
      SelectedFilter = "Level 1 Version 1 (*.xml)";
    }

  // We need to avoid the KDE dialog at least under Qt 4.7 and KDE 4.5
  // See: Bug 1651
  QFileDialog::Options DontUseNativeDialog = 0;

#ifdef Linux
  DontUseNativeDialog = QFileDialog::DontUseNativeDialog;
#endif // Linux

  NameAndVersion.first =
    CopasiFileDialog::getSaveFileName(parent, name, startWith, Filter, caption, &SelectedFilter, DontUseNativeDialog);

  QRegExp Pattern("Level (\\d) Version (\\d) \\(\\*\\.xml\\)");

  if (Pattern.exactMatch(SelectedFilter))
    {
      NameAndVersion.second.first = Pattern.cap(1).toInt();
      NameAndVersion.second.second = Pattern.cap(1).toInt();
    }
  else
    {
      NameAndVersion.second.first = 1;
      NameAndVersion.second.second = 1;
    }

  return NameAndVersion;
}
