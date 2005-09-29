/* Begin CVS Header
   $Source: /Volumes/Home/Users/shoops/cvs/copasi_dev/copasi/parameterFitting/CExperiment.cpp,v $
   $Revision: 1.7 $
   $Name:  $
   $Author: shoops $ 
   $Date: 2005/09/29 19:50:10 $
   End CVS Header */

#include <fstream>
#include <limits>

#include <math.h>
#include "copasi.h"

#include "CExperiment.h"
#include "CExperimentObjectMap.h"

#include "report/CKeyFactory.h"
#include "utilities/CTableCell.h"
#include "utilities/CSort.h"
#include "utilities/utility.h"

const std::string CExperiment::TypeName[] =
  {
    "ignored",
    "independent",
    "dependent",
    "Time"
  };

const char* CExperiment::XMLType[] =
  {
    "ignored",
    "independent",
    "dependent",
    "time",
    NULL
  };

CExperiment::CExperiment(const std::string & name,
                         const CCopasiContainer * pParent):
    CCopasiParameterGroup(name, pParent),
    mpFileName(NULL),
    mpFirstRow(NULL),
    mpTaskType(NULL),
    mpSeparator(NULL),
    mpRowOriented(NULL),
    mpNameRow(NULL),
    mpNumRows(NULL),
    mpNumColumns(NULL),
    mpColumnType(NULL),
    mColumnName(),
    mpObjectMap(NULL),
    mDataTime(0),
    mDataIndependent(0, 0),
    mDataDependent(0, 0),
    mMeans(0),
    mVariancesInv(0),
    mDependentValues(0),
    mIndependentUpdateMethods(0)
{initializeParameter();}

CExperiment::CExperiment(const CExperiment & src,
                         const CCopasiContainer * pParent):
    CCopasiParameterGroup(src, pParent),
    mpFileName(NULL),
    mpFirstRow(NULL),
    mpTaskType(NULL),
    mpSeparator(NULL),
    mpRowOriented(NULL),
    mpNameRow(NULL),
    mpNumRows(NULL),
    mpNumColumns(NULL),
    mpColumnType(NULL),
    mColumnName(src.mColumnName),
    mpObjectMap(NULL),
    mDataTime(src.mDataTime),
    mDataIndependent(src.mDataIndependent),
    mDataDependent(src.mDataDependent),
    mMeans(src.mMeans),
    mVariancesInv(src.mVariancesInv),
    mDependentValues(src.mDependentValues),
    mIndependentUpdateMethods(src.mIndependentUpdateMethods)
{initializeParameter();}

CExperiment::CExperiment(const CCopasiParameterGroup & group,
                         const CCopasiContainer * pParent):
    CCopasiParameterGroup(group, pParent),
    mpFileName(NULL),
    mpFirstRow(NULL),
    mpTaskType(NULL),
    mpSeparator(NULL),
    mpRowOriented(NULL),
    mpNameRow(NULL),
    mpNumRows(NULL),
    mpNumColumns(NULL),
    mpColumnType(NULL),
    mColumnName(),
    mpObjectMap(NULL),
    mDataTime(0),
    mDataIndependent(0, 0),
    mDataDependent(0, 0),
    mMeans(0),
    mVariancesInv(0),
    mDependentValues(0),
    mIndependentUpdateMethods(0)
{initializeParameter();}

CExperiment::~CExperiment() {}

void CExperiment::initializeParameter()
{
  GlobalKeys.remove(mKey);
  mKey = GlobalKeys.add("Experiment", this);

  if (!getParameter("Key"))
    addParameter("Key", CCopasiParameter::KEY, mKey);
  else
    setValue("Key", mKey);

  if (!getParameter("File Name"))
    addParameter("File Name", CCopasiParameter::STRING, std::string(""));
  mpFileName = getValue("File Name").pSTRING;

  if (!getParameter("Position in File"))
    addParameter("Position in File", CCopasiParameter::UINT, (unsigned C_INT32) C_INVALID_INDEX);
  mpFirstRow = getValue("").pUINT;

  if (!getParameter("Experiment Type"))
    addParameter("Experiment Type", CCopasiParameter::UINT, (unsigned C_INT32) CCopasiTask::unset);
  mpTaskType = (CCopasiTask::Type *) getValue("Experiment Type").pUINT;

  if (!getParameter("Seperator"))
    addParameter("Seperator", CCopasiParameter::STRING, std::string("\t"));
  mpSeparator = getValue("Seperator").pSTRING;

  if (!getParameter("Data is Row Oriented"))
    addParameter("Data is Row Oriented", CCopasiParameter::BOOL, (bool) true);
  mpRowOriented = getValue("Data is Row Oriented").pBOOL;

  if (!getParameter("Row containing Names"))
    addParameter("Row containing Names", CCopasiParameter::UINT, (unsigned C_INT32) C_INVALID_INDEX);
  mpNameRow = getValue("Row containing Names").pUINT;

  if (!getParameter("Number of Rows"))
    addParameter("Number of Rows", CCopasiParameter::UINT, (unsigned C_INT32) 0);
  mpNumRows = getValue("Number of Rows").pUINT;

  if (!getParameter("Number of Columns"))
    addParameter("Number of Columns", CCopasiParameter::UINT, (unsigned C_INT32) 0);
  mpNumColumns = getValue("Number of Columns").pUINT;

  if (getGroup("Column Role")) addGroup("Column Role");

  if (getGroup("Object Map")) addGroup("Object Map");

  elevateChildren();
}

bool CExperiment::elevateChildren()
{
  mpColumnType =
    elevate<CCopasiParameterGroup, CCopasiParameterGroup>(getGroup("Column Role"));
  if (!mpColumnType) return false;

  mpObjectMap =
    elevate<CExperimentObjectMap, CCopasiParameterGroup>(getGroup("Object Map"));
  if (!mpObjectMap) return false;

  return true;
}

C_FLOAT64 CExperiment::sumOfSquares(const unsigned C_INT32 & index)
{
  C_FLOAT64 s = 0.0;

  unsigned C_INT32 i , imax = mDataDependent.numCols();

  for (i = 0; i < imax; i++)
    s += pow(mDataDependent(index, i) - *mDependentValues[i], 2) * mVariancesInv[i];

  return s;
}

bool CExperiment::compile(const std::vector< CCopasiContainer * > listOfContainer)
{
  bool success = true;

  if (!mpObjectMap->compile(listOfContainer))
    success = false;

  unsigned C_INT32 LastMappedColumn = mpObjectMap->getLastColumn();
  const CVector< CCopasiObject * > & Objects = mpObjectMap->getMappedObjects();

  unsigned C_INT32 i, imax = mpColumnType->size();
  if (*mpNumColumns < imax)
    success = false; // More column types specified than we have data columns

  if (imax < LastMappedColumn)
    success = false; // An unspecified column, i.e, ignored column is mapped

  unsigned C_INT32 IndependentCount = mDataIndependent.numCols();
  unsigned C_INT32 DependentCount = mDataDependent.numCols();

  mDependentValues.resize(DependentCount);
  mIndependentUpdateMethods.resize(IndependentCount);

  IndependentCount = 0;
  DependentCount = 0;

  bool TimeFound = false;

  for (i = 0; i < imax; i++)
    switch (*mpColumnType->getValue(StringPrint("%d", i)).pUINT)
      {
      case ignore:
        if (i < LastMappedColumn && Objects[i])
          success = false; // An ignored column is mapped.
        break;

      case independent:
        if ((i < LastMappedColumn && !Objects[i]) || // Object not found
            i >= LastMappedColumn) success = false;  // The column must be mapped
        mIndependentUpdateMethods[IndependentCount++] =
          Objects[i]->getUpdateMethod();
        break;

      case dependent:
        if ((i < LastMappedColumn && !Objects[i]) || // Object not found
            i >= LastMappedColumn) success = false;  // The column must be mapped
        mDependentValues[DependentCount++] =
          (C_FLOAT64 *) Objects[i]->getReference();
        break;

      case time:
        TimeFound = true;
        break;
      }

  /* We need to check whether a column is mapped to time */
  if (!TimeFound && *mpTaskType == CCopasiTask::timeCourse)
    success = false;

  return success;
}

bool CExperiment::read(std::istream & in,
                       unsigned C_INT32 & currentLine)
{
  // Allocate for reading
  unsigned C_INT32 i, imax = mpColumnType->size();
  if (*mpNumColumns < imax)
    return false; // More column types specified than we have data columns

  unsigned C_INT32 IndependentCount = 0;
  unsigned C_INT32 DependentCount = 0;

  for (i = 0; i < imax; i++)
    switch (*mpColumnType->getValue(StringPrint("%d", i)).pUINT)
      {
      case ignore:
        break;

      case independent:
        IndependentCount++;
        break;

      case dependent:
        DependentCount++;
        break;

      case time:
        break;
      }

  unsigned C_INT32 NumRows =
    (*mpNameRow != C_INVALID_INDEX) ? *mpNumRows - 1 : *mpNumRows;

  mDataTime.resize(NumRows);
  mDataIndependent.resize(NumRows, IndependentCount);
  mDataDependent.resize(NumRows, DependentCount);
  mColumnName.resize(IndependentCount + DependentCount);

  CTableRow Row(*mpNumColumns, (*mpSeparator)[0]);
  const std::vector< CTableCell > & Cells = Row.getCells();

  unsigned C_INT32 j;

  if (currentLine > *mpFirstRow) return false; // We are past our first line

  // forwind to our first line
  for (j = currentLine; j < *mpFirstRow && !in.fail(); j++)
    {
      in.ignore(LONG_MAX, '\x0a');
      currentLine++;
    }

  for (j = 0; j < NumRows && !in.fail(); j++)
    {
      in >> Row;
      currentLine++;

      if (j == *mpNameRow)
        {
          j--;
          for (i = 0; i < *mpNumColumns; i++)
            mColumnName[i] = Cells[i].getName();

          continue;
        }

      IndependentCount = 0;
      DependentCount = 0;

      for (i = 0; i < imax; i++)
        {
          switch (*mpColumnType->getValue(StringPrint("%d", i)).pUINT)
            {
            case ignore:
              break;

            case independent:
              mDataIndependent[j][IndependentCount++] =
                Cells[i].getValue();
              break;

            case dependent:
              mDataDependent[j][DependentCount++] =
                Cells[i].getValue();
              break;

            case time:
              mDataTime[j] = Cells[i].getValue();
              break;
            }
        }
    }

  if ((in.fail() && !in.eof()) ||
      j != NumRows) return false;

  // We need to calculate the means and variances
  mMeans.resize(DependentCount);
  mVariancesInv.resize(DependentCount);
  unsigned C_INT32 Count;

  for (i = 0; i < DependentCount; i++)
    {
      C_FLOAT64 & Mean = mMeans[i];
      Mean = 0.0;
      Count = 0;

      for (j = 0; j < NumRows; j++)
        {
          C_FLOAT64 & Data = mDataDependent[j][i];
          if (!isnan(Data))
            {
              Count++;
              Mean += Data;
            }
        }

      if (Count)
        Mean /= Count;
      else
        Mean = std::numeric_limits<C_FLOAT64>::quiet_NaN();

      C_FLOAT64 & Variance = mVariancesInv[i];
      Variance = 0;

      for (j = 0; j < NumRows; j++)
        {
          C_FLOAT64 & Data = mDataDependent[j][i];
          if (!isnan(Data))
            Variance += pow((Mean - Data), 2);
        }

      if (Count > 1)
        Variance = (Count - 1.0) / Variance;
      else
        Variance = 1;
    }

  // If it is a time course this is the place to sort
  // :TODO: We need to check that all other independent data is not changing.
  if (*mpTaskType == CCopasiTask::timeCourse)
    {
      CVector<unsigned C_INT32> Pivot;
      sortWithPivot(mDataTime.array(), mDataTime.array() + mDataTime.size(), Pivot);

      mDataTime.applyPivot(Pivot);
      mDataIndependent.applyPivot(Pivot);
      mDataDependent.applyPivot(Pivot);
    }

  return true;
}

const CCopasiTask::Type & CExperiment::getExperimentType() const
  {return *mpTaskType;}

const CVector< C_FLOAT64 > & CExperiment::getTimeData() const
  {return mDataTime;}

const CMatrix< C_FLOAT64 > & CExperiment::getIndependentData() const
  {return mDataIndependent;}

const CMatrix< C_FLOAT64 > & CExperiment::getDependentData() const
  {return mDataDependent;}

const std::string & CExperiment::getFileName() const
  {return *mpFileName;}

bool CExperiment::setFileName(const std::string & fileName)
{
  *mpFileName = fileName;
  return true;
}

bool CExperiment::addColumnType(const unsigned C_INT32 & index, const Type & type)
{
  std::string Index = StringPrint("%d", index);
  if (mpColumnType->getParameter(Index)) return false;

  return mpColumnType->addParameter(Index, CCopasiParameter::UINT,
                                    (unsigned C_INT32) type);
}

bool CExperiment::removeColumnType(const unsigned C_INT32 & index)
{return removeParameter(StringPrint("%d", index));}

const CExperiment::Type & CExperiment::getColumnType(const unsigned C_INT32 & index) const
  {
    std::string Index = StringPrint("%d", index);
    return * static_cast<CExperiment::Type *>(mpColumnType->getValue(Index).pVOID);
  }

bool CExperiment::setColumnType(const unsigned C_INT32 & index,
                                const CExperiment::Type & type)
{
  std::string Index = StringPrint("%d", index);

  return mpColumnType->setValue(Index, (unsigned C_INT32) type);
}

const unsigned C_INT32 & CExperiment::getNumColumns() const
  {return *mpNumColumns;}

bool CExperiment::setNumColumns(const unsigned C_INT32 & cols)
{
  *mpNumColumns = cols;
  return true;
}

const unsigned C_INT32 & CExperiment::getNumRows() const
  {return *mpNumRows;}

bool CExperiment::setNumRows(const unsigned C_INT32 & rows)
{
  *mpNumRows = rows;
  return true;
}

const std::string & CExperiment::getSeperator() const
  {return *mpSeparator;}

bool CExperiment::setSeperator(const std::string & seperator)
{
  *mpSeparator = seperator;
  return true;
}

const bool & CExperiment::isRowOriented() const
  {return *mpRowOriented;}

bool CExperiment::setIsRowOriented(const bool & isRowOriented)
{
  *mpRowOriented = isRowOriented;
  return true;
}

bool CExperiment::compare(const CExperiment * lhs,
                          const CExperiment * rhs)
{
  return (*lhs->mpFileName < *rhs->mpFileName ||
          (*lhs->mpFileName == *rhs->mpFileName &&
           *lhs->mpFirstRow < *rhs->mpFirstRow));
}
