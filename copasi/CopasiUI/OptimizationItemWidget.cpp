/****************************************************************************
 ** Form implementation generated from reading ui file '.\OptimizationItemWidget.ui'
 **
 ** Created: Mon Sep 29 00:08:08 2003
 **      by: The User Interface Compiler ($Id: OptimizationItemWidget.cpp,v 1.11 2003/10/04 19:08:00 lixu1 Exp $)
 **
 ** WARNING! All changes made in this file will be lost!
 ****************************************************************************/

#include "OptimizationItemWidget.h"
#include "ScanItemWidget.h"
#include "FunctionItemWidget.h"

#include <qvariant.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qcombobox.h>
#include <qframe.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>

/*
 *  Constructs a OptimizationItemWidget as a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f'.
 */
OptimizationItemWidget::OptimizationItemWidget(QWidget* parent, const char* name, WFlags fl)
    : QWidget(parent, name, fl)
{
  if (!name)
    setName("OptimizationItemWidget");
  OptimizationItemWidgetLayout = new QGridLayout(this, 1, 1, 11, 6, "OptimizationItemWidgetLayout");

  textLabel2 = new QLabel(this, "textLabel2");

  OptimizationItemWidgetLayout->addWidget(textLabel2, 0, 0);

  layout13 = new QHBoxLayout(0, 0, 6, "layout13");

  lineLower = new ScanLineEdit(this, "lineLower");
  layout13->addWidget(lineLower);

  buttonLowerEdit = new QPushButton(this, "buttonLowerEdit");
  layout13->addWidget(buttonLowerEdit);

  OptimizationItemWidgetLayout->addLayout(layout13, 5, 2);

  comboBoxUpperOp = new QComboBox(FALSE, this, "comboBoxUpperOp");

  OptimizationItemWidgetLayout->addWidget(comboBoxUpperOp, 2, 1);

  textLabel3 = new QLabel(this, "textLabel3");

  OptimizationItemWidgetLayout->addWidget(textLabel3, 2, 0);

  comboBoxLowerOp = new QComboBox(FALSE, this, "comboBoxLowerOp");

  OptimizationItemWidgetLayout->addWidget(comboBoxLowerOp, 5, 1);

  checkLowerInf = new ScanCheckBox(this, "checkLowerInf");

  OptimizationItemWidgetLayout->addWidget(checkLowerInf, 6, 2);

  layout14 = new QHBoxLayout(0, 0, 6, "layout14");

  lineUpper = new ScanLineEdit(this, "lineUpper");
  layout14->addWidget(lineUpper);

  buttonUpperEdit = new QPushButton(this, "buttonUpperEdit");
  layout14->addWidget(buttonUpperEdit);

  OptimizationItemWidgetLayout->addLayout(layout14, 2, 2);

  textLabel4 = new QLabel(this, "textLabel4");

  OptimizationItemWidgetLayout->addWidget(textLabel4, 5, 0);

  line10 = new QFrame(this, "line10");
  line10->setFrameShape(QFrame::HLine);
  line10->setFrameShadow(QFrame::Sunken);
  line10->setFrameShape(QFrame::HLine);

  OptimizationItemWidgetLayout->addMultiCellWidget(line10, 4, 4, 0, 2);

  checkUpperInf = new ScanCheckBox(this, "checkUpperInf");

  OptimizationItemWidgetLayout->addWidget(checkUpperInf, 3, 2);

  line11 = new QFrame(this, "line11");
  line11->setFrameShape(QFrame::HLine);
  line11->setFrameShadow(QFrame::Sunken);
  line11->setFrameShape(QFrame::HLine);

  OptimizationItemWidgetLayout->addMultiCellWidget(line11, 1, 1, 0, 2);

  ObjectName = new ScanLineEdit(this, "ObjectName");
  ObjectName->setEnabled(FALSE);

  OptimizationItemWidgetLayout->addMultiCellWidget(ObjectName, 0, 0, 1, 2);
  languageChange();
  resize(QSize(390, 173).expandedTo(minimumSizeHint()));
  clearWState(WState_Polished);

  // signals and slots connections
  connect(checkUpperInf, SIGNAL(clicked()), this, SLOT(slotPosInfClicked()));
  connect(checkLowerInf, SIGNAL(clicked()), this, SLOT(slotNegInfClicked()));
  connect(buttonUpperEdit, SIGNAL(clicked()), this, SLOT(slotUpperEdit()));
  connect(buttonLowerEdit, SIGNAL(clicked()), this, SLOT(slotLowerEdit()));

  // tab order
  setTabOrder(ObjectName, comboBoxUpperOp);
  setTabOrder(comboBoxUpperOp, lineUpper);
  setTabOrder(lineUpper, buttonUpperEdit);
  setTabOrder(buttonUpperEdit, checkUpperInf);
  setTabOrder(checkUpperInf, comboBoxLowerOp);
  setTabOrder(comboBoxLowerOp, lineLower);
  setTabOrder(lineLower, buttonLowerEdit);
  setTabOrder(buttonLowerEdit, checkLowerInf);

  checkUpperInf->setChecked(true);
  checkLowerInf->setChecked(true);

  buttonUpperEdit->setEnabled(false);
  buttonLowerEdit->setEnabled(false);

  lineUpper->setEnabled(false);
  lineLower->setEnabled(false);
}

/*
 *  Destroys the object and frees any allocated resources
 */
OptimizationItemWidget::~OptimizationItemWidget()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void OptimizationItemWidget::languageChange()
{
  setCaption(tr("Optimization Item"));
  textLabel2->setText(tr("Parameter"));
  buttonLowerEdit->setText(tr("edit"));
  comboBoxUpperOp->clear();
  comboBoxUpperOp->insertItem(tr("<"));
  comboBoxUpperOp->insertItem(tr("<="));
  comboBoxUpperOp->insertItem(tr("=="));
  textLabel3->setText(tr("Upper"));
  comboBoxLowerOp->clear();
  comboBoxLowerOp->insertItem(tr(">"));
  comboBoxLowerOp->insertItem(tr(">="));
  comboBoxLowerOp->insertItem(tr("=="));
  checkLowerInf->setText(tr("- Inf"));
  buttonUpperEdit->setText(tr("edit"));
  textLabel4->setText(tr("Lower"));
  checkUpperInf->setText(tr("+ Inf"));
}

void OptimizationItemWidget::slotPosInfClicked()
{
  lineUpper->setEnabled(!checkUpperInf->isChecked());
  buttonUpperEdit->setEnabled(!checkUpperInf->isChecked());
}

void OptimizationItemWidget::slotLowerEdit()
{
  //qWarning("OptimizationItemWidget::slotLowerEdit(): Not implemented yet");
  std::string strFunction;
  FunctionItemWidget* pFuncDlg = new FunctionItemWidget(this);
  strFunction = lineLower->text().latin1();
  pFuncDlg->setStrFunction(&strFunction);
  if (pFuncDlg->exec () == QDialog::Accepted)
    {
      lineLower->setText(strFunction.c_str());
    }
}

void OptimizationItemWidget::slotNegInfClicked()
{
  lineLower->setEnabled(!checkLowerInf->isChecked());
  buttonLowerEdit->setEnabled(!checkLowerInf->isChecked());
}

void OptimizationItemWidget::slotUpperEdit()
{
  //qWarning("OptimizationItemWidget::slotUpperEdit(): Not implemented yet");
  std::string strFunction;
  FunctionItemWidget* pFuncDlg = new FunctionItemWidget(this);
  strFunction = lineUpper->text().latin1();
  pFuncDlg->setStrFunction(&strFunction);
  if (pFuncDlg->exec () == QDialog::Accepted)
    {
      lineUpper->setText(strFunction.c_str());
    }
}

std::string OptimizationItemWidget::getItemUpperLimit()
{
  if (checkUpperInf->isChecked())
    return "+inf";
  else
    return lineUpper->text().latin1();
}

std::string OptimizationItemWidget::getItemLowerLimit()
{
  if (checkUpperInf->isChecked())
    return "+inf";
  else
    return lineUpper->text().latin1();
}

CCopasiObject* OptimizationItemWidget::getCopasiObject()
{
  return mpObject;
}

void OptimizationItemWidget::setCopasiObjecPtr (CCopasiObject* sourceObject)
{
  if (!sourceObject) // NULL pointer
    return;
  mpObject = sourceObject;
  ObjectName->setText(mpObject->getObjectUniqueName().c_str());
}

OptimizationItemWidget::setItemUpperLimit(std::string strUpperLimit)
{
  if (strUpperLimit == "+inf")
    {
      checkUpperInf->setChecked(true);
      buttonUpperEdit->setEnabled(false);
      lineUpper->setEnabled(false);
    }
  else
    {
      checkUpperInf->setChecked(false);
      buttonUpperEdit->setEnabled(true);
      lineUpper->setEnabled(true);
      lineUpper->setText(strUpperLimit.c_str());
    }
}

OptimizationItemWidget::setItemLowerLimit(std::string strLowerLimit)
{
  if (strLowerLimit == "-inf")
    {
      checkLowerInf->setChecked(true);
      buttonLowerEdit->setEnabled(false);
      lineLower->setEnabled(false);
    }
  else
    {
      checkLowerInf->setChecked(false);
      buttonLowerEdit->setEnabled(true);
      lineLower->setEnabled(true);
      lineLower->setText(strLowerLimit.c_str());
    }
}
