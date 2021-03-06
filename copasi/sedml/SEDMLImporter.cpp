// Copyright (C) 2013 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., University of Heidelberg, and The University
// of Manchester.
// All rights reserved.

/**
 * SEDMLImporter.cpp
 * $Rev:               $:  Revision of last commit
 * $Author:            $:  Author of last commit
 * $Date:              $:  Date of last commit
 * $HeadURL:       $
 * $Id:        $
 */

#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <map>
#include <limits>
#include <cmath>
#include <algorithm>

#include <sedml/SedReader.h>
#include <sedml/SedDocument.h>
#include <sedml/SedModel.h>
#include <sedml/SedSimulation.h>
#include <sedml/SedUniformTimeCourse.h>
#include <sedml/SedDataGenerator.h>
#include <sedml/SedPlot2D.h>
//#include <sedml/SedOutput.h>

#include <sedml/SedTypes.h>

#include "copasi.h"

#include "report/CKeyFactory.h"
#include "model/CModel.h"
#include "model/CCompartment.h"
#include "model/CMetab.h"
#include "model/CReaction.h"
#include "model/CModelValue.h"
#include "model/CEvent.h"
#include "function/CNodeK.h"
#include "function/CFunctionDB.h"
#include "function/CEvaluationTree.h"
#include "function/CExpression.h"
#include "function/CFunctionParameters.h"
#include "report/CCopasiObjectReference.h"
#include "utilities/CCopasiTree.h"
#include "utilities/CNodeIterator.h"
#include "CopasiDataModel/CCopasiDataModel.h"
#include "report/CCopasiRootContainer.h"
#include "MIRIAM/CRDFGraphConverter.h"
#include "compareExpressions/CEvaluationNodeNormalizer.h"
#include "commandline/CLocaleString.h"
#include "commandline/COptions.h"

#include "utilities/CProcessReport.h"
#include "copasi/commandline/CConfigurationFile.h"

#include "utilities/CCopasiMessage.h"

//TODO SEDML
#include "trajectory/CTrajectoryTask.h"
#include "trajectory/CTrajectoryProblem.h"
#include "sbml/SBMLImporter.h"
#include "utilities/CDirEntry.h"
#include "utilities/CCopasiException.h"
#include "utilities/CCopasiTask.h"
#include "plot/COutputDefinitionVector.h"
#include "plot/CPlotSpecification.h"

#include "SEDMLImporter.h"
#include "SEDMLUtils.h"

// static
C_FLOAT64 SEDMLImporter::round(const C_FLOAT64 & x)
{
  return
    x < 0.0 ? -floor(-x + 0.5) : floor(x + 0.5);
}

void SEDMLImporter::setImportHandler(CProcessReport* pHandler)
{mpImportHandler = pHandler;}

CProcessReport* SEDMLImporter::getImportHandlerAddr()
{return mpImportHandler;}

const std::string SEDMLImporter::getArchiveFileName()
{return mArchiveFileName;}

/**
 * Creates and returns a COPASI CTrajectoryTask from the SEDML simulation
 * given as argument.
 */
CTrajectoryTask* SEDMLImporter::createCTrajectoryTaskFromSimulation(SedSimulation* sedmlsim,
    std::map<CCopasiObject*, SedBase*>& copasi2sedmlmap)
{

  CTrajectoryTask *tTask = new CTrajectoryTask();

  CTrajectoryProblem* tProblem = static_cast<CTrajectoryProblem*>(tTask->getProblem());

  //SedSimulation* current = doc->getSimulation(0);
  switch (sedmlsim->getTypeCode())
    {
      case SEDML_SIMULATION_UNIFORMTIMECOURSE:
      {

        ;             SedUniformTimeCourse* tc = static_cast<SedUniformTimeCourse*>(sedmlsim);
        tProblem->setOutputStartTime(tc->getOutputStartTime());
        tProblem->setDuration(tc->getOutputEndTime() - tc->getOutputStartTime());
        tProblem->setStepNumber(tc->getNumberOfPoints());
        //  tMethod->setProblem(tProblem);

        break;
      }

      default:
        CCopasiMessage(CCopasiMessage::EXCEPTION, "SEDMLImporter Error: encountered unknown simulation.");
        break;
    }

  return tTask;
}

void SEDMLImporter::readListOfPlotsFromSedMLOutput(
  COutputDefinitionVector *pLotList, CModel* pModel,
  SedDocument *pSEDMLDocument,
  std::map<CCopasiObject*, SedBase*>& copasi2sedmlmap)
{
  size_t i, numOutput = pSEDMLDocument->getNumOutputs();

  for (i = 0; i < numOutput; ++i)
    {
      SedOutput* current = pSEDMLDocument->getOutput(i);

      switch (current->getTypeCode())
        {
          case SEDML_OUTPUT_PLOT2D: //get the curves data
          {
            SedPlot2D* p = static_cast<SedPlot2D*>(current);
            CPlotSpecification* pPl = pLotList->createPlotSpec(
                                        current->isSetName() ? current->getName() :
                                        current->getId()
                                        , CPlotItem::plot2d);

            //need Time for all Time Courses
            const CCopasiObject* pTime = static_cast<const CCopasiObject *>(pModel->getObject(CCopasiObjectName("Reference=Time")));

            for (unsigned int ic = 0; ic < p->getNumCurves(); ++ic)
              {
                SedCurve *curve = p->getCurve(ic);
                std::string SBMLType, yAxis, yDataReference;
                yDataReference = curve->getYDataReference();

                yAxis = getDataGeneratorModelItemRefrenceId(pSEDMLDocument, yDataReference, SBMLType);

                //create the curves
                CPlotDataChannelSpec name1 = pTime->getCN();
                CPlotDataChannelSpec name2;
                std::string itemTitle;
                CPlotItem * plItem;
                const CCopasiObject * tmp;

                if (SBMLType == "species")
                  {
                    size_t iMet, imax = pModel->getMetabolites().size();

                    for (iMet = 0; iMet < imax; ++iMet)
                      {
                        pModel->getMetabolites()[iMet]->setInitialConcentration(0.896901);

                        if (pModel->getMetabolites()[iMet]->getSBMLId() == yAxis)
                          {
                            tmp = pModel->getMetabolites()[iMet]->getConcentrationReference();
                            name2 = tmp->getCN();
                            itemTitle = tmp->getObjectDisplayName();
                            plItem = pPl->createItem(itemTitle, CPlotItem::curve2d);
                            plItem->addChannel(name1);
                            plItem->addChannel(name2);
                            break;
                          }
                      }
                  }
                else if (SBMLType == "reaction")
                  {
                    size_t iMet, imax = pModel->getReactions().size();

                    for (iMet = 0; iMet < imax; ++iMet)
                      {
                        if (pModel->getReactions()[iMet]->getSBMLId() == yAxis)
                          {
                            tmp = pModel->getReactions()[iMet]->getFluxReference();
                            name2 = tmp->getCN();
                            itemTitle = tmp->getObjectDisplayName();
                            plItem = pPl->createItem(itemTitle, CPlotItem::curve2d);
                            plItem->addChannel(name1);
                            plItem->addChannel(name2);
                            break;
                          }
                      }
                  }
                else if (SBMLType == "parameter")
                  {
                    size_t iMet, imax = pModel->getModelValues().size();

                    for (iMet = 0; iMet < imax; ++iMet)
                      {
                        if (pModel->getModelValues()[iMet]->getSBMLId() == yAxis)
                          {
                            tmp = pModel->getModelValues()[iMet]->getValueReference();
                            name2 = tmp->getCN();
                            itemTitle = tmp->getObjectDisplayName();
                            plItem = pPl->createItem(itemTitle, CPlotItem::curve2d);
                            plItem->addChannel(name1);
                            plItem->addChannel(name2);
                            break;
                          }
                      }
                  }
              }

            pPl->setLogX(false);
            pPl->setLogY(false);
            break;
          }

          default:
            CCopasiMessage(CCopasiMessage::EXCEPTION, "SEDMLImporter Error: No support for this plot: typecode = %d", current->getTypeCode());
            break;
        }
    }
}

std::string SEDMLImporter::translateTargetXpathInSBMLId(const std::string &xpath, std::string & SBMLType)
{

  std::vector<std::string> xpathStrings;
  std::string id, nextString;
  SEDMLUtils utils;

  char delim = ':';
  utils.splitStrings(xpath, delim, xpathStrings);
  nextString = xpathStrings[xpathStrings.size() - 1];

  delim = '[';
  utils.splitStrings(nextString, delim, xpathStrings);
  SBMLType = xpathStrings[0];
  nextString = xpathStrings[xpathStrings.size() - 1];

  delim = '=';
  utils.splitStrings(nextString, delim, xpathStrings);
  id = xpathStrings[xpathStrings.size() - 1];

  //remove the remaining unwanted characters
  char chars[] = "']'";

  for (unsigned int i = 0; i < strlen(chars); ++i)
    {

      id.erase(std::remove(id.begin(), id.end(), chars[i]), id.end());
    }

  return id;
}

std::string SEDMLImporter::getDataGeneratorModelItemRefrenceId(SedDocument *pSEDMLDocument, std::string &dataReference, std::string & SBMLType)
{
  std::string modelReferenceId;

  size_t i, iMax = pSEDMLDocument->getNumDataGenerators();

  for (i = 0; i < iMax; ++i)
    {
      SedDataGenerator* current = pSEDMLDocument->getDataGenerator(i);

      if (dataReference == current->getId()) //check if the data generator of interest
        {
          //modelReferenceId = SBML_formulaToString(current->getMath());

          //assumed only one variable
          size_t ii, iiMax = current->getNumVariables();

          for (ii = 0; ii < iiMax; ++ii)
            {
              SedVariable *var =  current->getVariable(ii);
              modelReferenceId = translateTargetXpathInSBMLId(var->getTarget(), SBMLType);
            }
        }
    }

  return modelReferenceId;
}

/**
 * Function reads an SEDML file with libsedml and converts it to a Copasi CModel
 */
CModel* SEDMLImporter::readSEDML(std::string filename,
                                 CProcessReport* pImportHandler,
                                 CFunctionDB* funDB,
                                 SBMLDocument *& pSBMLDocument,
                                 SedDocument*& pSedDocument,
                                 std::map<CCopasiObject*, SedBase*>& copasi2sedmlmap,
                                 std::map<CCopasiObject*, SBase*>& copasi2sbmlmap,
                                 CListOfLayouts *& prLol,
                                 CTrajectoryTask *& trajTask,
                                 COutputDefinitionVector * &plotList,
                                 CCopasiDataModel* pDataModel)
{
  // convert filename to the locale encoding
  std::ifstream file(CLocaleString::fromUtf8(filename).c_str());

  if (!file)
    {
      CCopasiMessage(CCopasiMessage::EXCEPTION, MCSEDML + 5, filename.c_str());
    }

  std::ostringstream stringStream;
  char c;

  while (file.get(c))
    {
      stringStream << c;
    }

  file.clear();
  file.close();

  //using libzip to read SEDML file
  /*  SEDMLUtils utils;
    std::string SEDMLFileName, fileContent("");
    SEDMLFileName = "sedml.xml";

    int success = utils.processArchive(filename, SEDMLFileName, fileContent);*/

  pDataModel->setSEDMLFileName(filename);

  return this->parseSEDML(stringStream.str(), pImportHandler, funDB,
                          pSBMLDocument, pSedDocument, copasi2sedmlmap, copasi2sbmlmap, prLol, trajTask, plotList, pDataModel);
}
/**
 * Function parses an SEDML document with libsedml and converts it to a COPASI CModel
 * object which is returned. Deletion of the returned pointer is up to the
 * caller.
 */
CModel*
SEDMLImporter::parseSEDML(const std::string& sedmlDocumentText,
                          CProcessReport* pImportHandler,
                          CFunctionDB* funDB,
                          SBMLDocument *& pSBMLDocument,
                          SedDocument *& pSEDMLDocument,
                          std::map<CCopasiObject*, SedBase*>& copasi2sedmlmap,
                          std::map<CCopasiObject*, SBase*>& copasi2sbmlmap,
                          CListOfLayouts *& prLol,
                          CTrajectoryTask *& trajTask,
                          COutputDefinitionVector * & pPlotList,
                          CCopasiDataModel* pDataModel)
{

  this->mUsedSEDMLIdsPopulated = false;

  mpDataModel = pDataModel;
  assert(mpDataModel != NULL);

  this->mpCopasiModel = NULL;

  if (funDB != NULL)
    {
      this->functionDB = funDB;
      SedReader* reader = new SedReader();

      mImportStep = 0;

      if (mpImportHandler)
        {
          mpImportHandler->setName("Importing SEDML file...");
          mTotalSteps = 11;
          mhImportStep = mpImportHandler->addItem("Step", mImportStep,
                                                  &mTotalSteps);
        }

      unsigned C_INT32 step = 0, totalSteps = 0;
      size_t hStep = C_INVALID_INDEX;

      if (this->mpImportHandler != 0)
        {
          step = 0;
          totalSteps = 1;
          hStep = mpImportHandler->addItem("Reading SEDML file...", step,
                                           &totalSteps);
        }

      SedDocument* sedmlDoc = reader->readSedMLFromString(sedmlDocumentText);

      assert(sedmlDoc != NULL);

      if (mpImportHandler)
        mpImportHandler->finishItem(hStep);

      if (this->mpImportHandler != 0)
        {
          step = 0;
          totalSteps = 1;
          hStep = mpImportHandler->addItem("Checking consistency...", step,
                                           &totalSteps);
        }

      //    if (CCopasiRootContainer::getConfiguration()->validateUnits())

      if (mpImportHandler)
        mpImportHandler->finishItem(hStep);

      int fatal = -1;
      unsigned int i, iMax = sedmlDoc->getNumErrors();
      //  std::cout<<"Errors: "<<iMax<<std::endl;

      for (i = 0; (i < iMax) && (fatal == -1); ++i)
        {
          const SedError* pSEDMLError = sedmlDoc->getError(i);

          CCopasiMessage::Type messageType = CCopasiMessage::RAW;

          switch (pSEDMLError->getSeverity())
            {

              case LIBSEDML_SEV_WARNING:

                if (mIgnoredSEDMLMessages.find(pSEDMLError->getErrorId())
                    != mIgnoredSEDMLMessages.end())
                  {
                    messageType = CCopasiMessage::WARNING_FILTERED;
                  }
                else
                  {
                    messageType = CCopasiMessage::WARNING;
                  }

                CCopasiMessage(messageType, MCSEDML + 6, "WARNING",
                               pSEDMLError->getErrorId(), pSEDMLError->getLine(),
                               pSEDMLError->getColumn(),
                               pSEDMLError->getMessage().c_str());
                break;

              case LIBSEDML_SEV_ERROR:

                if (mIgnoredSEDMLMessages.find(pSEDMLError->getErrorId())
                    != mIgnoredSEDMLMessages.end())
                  {
                    messageType = CCopasiMessage::ERROR_FILTERED;
                  }

                CCopasiMessage(messageType, MCSEDML + 6, "ERROR",
                               pSEDMLError->getErrorId(), pSEDMLError->getLine(),
                               pSEDMLError->getColumn(),
                               pSEDMLError->getMessage().c_str());
                break;

              case LIBSEDML_SEV_FATAL:

                // treat unknown as fatal
              default:

                if (pSEDMLError->getErrorId() == 10804)
                  {
                    // this error indicates a problem with a notes element
                    // although libsedml flags this as fatal, we would still
                    // like to read the model
                    CCopasiMessage(messageType, MCSEDML + 6, "ERROR",
                                   pSEDMLError->getErrorId(), pSEDMLError->getLine(),
                                   pSEDMLError->getColumn(),
                                   pSEDMLError->getMessage().c_str());
                  }
                else
                  {
                    fatal = i;
                  }

                break;
            }
        }

      if (fatal != -1)
        {
          const XMLError* pSEDMLError = sedmlDoc->getError(fatal);
          CCopasiMessage Message(CCopasiMessage::EXCEPTION, MCXML + 2,
                                 pSEDMLError->getLine(), pSEDMLError->getColumn(),
                                 pSEDMLError->getMessage().c_str());

          if (mpImportHandler)
            mpImportHandler->finishItem(mhImportStep);

          return NULL;
        }

      if (sedmlDoc->getListOfModels() == NULL)
        {
          CCopasiMessage Message(CCopasiMessage::ERROR, MCSEDML + 2);

          if (mpImportHandler)
            mpImportHandler->finishItem(mhImportStep);

          return NULL;
        }

      //delete reader;
      pSEDMLDocument = sedmlDoc;
      this->mLevel = pSEDMLDocument->getLevel();

      this->mOriginalLevel = this->mLevel;
      this->mVersion = pSEDMLDocument->getVersion();

      std::string SBMLFileName, fileContent;

      unsigned int ii, iiMax = pSEDMLDocument->getListOfModels()->size();

      if (iiMax < 1)
        {
          CCopasiMessage(CCopasiMessage::EXCEPTION, MCSEDML + 2);
        }

      std::string modelSource = ""; //must be taken from SEDML document.
      std::string modelId = ""; // to ensure only one model is imported since only one model in SEDML file is supported

      for (ii = 0; ii < iiMax; ++ii)
        {
          SedModel* sedmlModel = sedmlDoc->getModel(ii);

          if (sedmlModel->getLanguage() != "urn:sedml:language:sbml") CCopasiMessage(CCopasiMessage::EXCEPTION, "Sorry only SBML model is presently supported.");

          if (sedmlModel->getSource() != modelId)
            {
              modelId = sedmlModel->getId();

              if ((sedmlModel->getListOfChanges()->size()) > 0)CCopasiMessage(CCopasiMessage::WARNING, "Currently no support for"
                    " changing model entities. Changes will not be made to the imported model.");

              modelSource = sedmlModel->getSource();
            }
        }

      assert(modelSource != "");

      //process the archive file and get the SBML model file
      //SEDMLUtils utils;
      //int success = utils.processArchive(pDataModel->getSEDMLFileName(), SBMLFileName, fileContent);

      //std::cout<<fileContent<<std::endl;
      //experiment SEDML
      //  pDataModel->getSEDMLFileName();
      std::string FileName;
      FileName = CDirEntry::dirName(pDataModel->getSEDMLFileName())
                 + CDirEntry::Separator + modelSource;

      std::ifstream file(CLocaleString::fromUtf8(FileName).c_str());

      if (!file)
        {
          CCopasiMessage(CCopasiMessage::EXCEPTION, MCSEDML + 4,
                         FileName.c_str());
        }

      //set the SBML file name for later use
      pDataModel->setSBMLFileName(FileName);
      std::ostringstream sbmlStringStream;
      char c;

      while (file.get(c))
        {
          sbmlStringStream << c;
        }

      file.clear();
      file.close();

      std::ifstream File(CLocaleString::fromUtf8(FileName).c_str());

      SBMLImporter importer;
      // Right now we always import the COPASI MIRIAM annotation if it is there.
      // Later this will be settable by the user in the preferences dialog
      importer.setImportCOPASIMIRIAM(true);
      importer.setImportHandler(pImportHandler);

      CModel* pModel = NULL;

      SBMLDocument * pSBMLDocument = NULL;
      std::map<CCopasiObject*, SBase*> Copasi2SBMLMap;

      try
        {
          pModel = importer.parseSBML(sbmlStringStream.str(),
                                      CCopasiRootContainer::getFunctionList(), pSBMLDocument,
                                      Copasi2SBMLMap, prLol, mpDataModel);
        }

      catch (CCopasiException & except)
        {
          importer.restoreFunctionDB();
          importer.deleteCopasiModel();
          //    popData();

          throw except;
        }

      if (pModel == NULL)
        {
          importer.restoreFunctionDB();
          importer.deleteCopasiModel();
          //   popData();
          return NULL;
        }

      pPlotList = new COutputDefinitionVector("OutputDefinitions", mpDataModel);
      readListOfPlotsFromSedMLOutput(pPlotList, pModel, pSEDMLDocument, copasi2sedmlmap);

      this->mpCopasiModel = pModel;

      //  this->mpCopasiModel = this->createCModelFromSEDMLDocument(sedmlDoc, copasi2sedmlmap);

      //  CCopasiTask *task = mpDataModel->addTask(CCopasiTask::timeCourse);

      SedSimulation* sedmlsim = sedmlDoc->getSimulation(0);

      SedSimulation* current = sedmlDoc->getSimulation(0);
      //  CTrajectoryTask *task1= (CTrajectoryTask)mpDataModel->addTask(CCopasiTask::timeCourse);

      //CTrajectoryTask *trajTask;
      trajTask = this->createCTrajectoryTaskFromSimulation(sedmlsim, copasi2sedmlmap);

      //static_cast<CTrajectoryProblem*>(tTask->getProblem());
      //  CTrajectoryProblem* tProblem = static_cast<CTrajectoryProblem*>(trajTask->getProblem());
      //  std::cout<<"problem-Duration: "<<tProblem->getDuration()<< "OutputStartTime: "<< tProblem->getOutputStartTime() <<std::endl;

      if (mpImportHandler)
        mpImportHandler->finishItem(mhImportStep);

      return this->mpCopasiModel;
      delete reader;
    }
  return NULL;
}

/**
 * Constructor that initializes speciesMap and the FunctionDB object
 */
SEDMLImporter::SEDMLImporter():
  mIgnoredSEDMLMessages(),
  functionDB(NULL),
  mIncompleteModel(false),
  mLevel(0),
  mOriginalLevel(0),
  mVersion(0),
  sedmlIdMap(),
  mUsedFunctions(),
  mpDataModel(NULL),
  mpCopasiModel(NULL),
  mFunctionNameMapping(),
  mpImportHandler(NULL),
  mImportStep(0),
  mhImportStep(C_INVALID_INDEX),
  mTotalSteps(0),
  mUsedSEDMLIds(),
  mUsedSEDMLIdsPopulated(false),
  mKnownCustomUserDefinedFunctions()
{
  this->functionDB = NULL;
  this->mIncompleteModel = false;
  this->mpImportHandler = NULL;

  this->mIgnoredSEDMLMessages.insert(10501);
}

void SEDMLImporter::restoreFunctionDB()
{
  // set all the old sbml ids
  std::map<CFunction*, std::string>::iterator it = this->sedmlIdMap.begin();
  std::map<CFunction*, std::string>::iterator endIt = this->sedmlIdMap.end();

  //TODO
  /*   while (it != endIt)
       {
         it->first->setSEDMLId(it->second);
         ++it;
       }
  */
  // remove all the functions that were added during import
  std::set<std::string>::iterator it2 = this->mUsedFunctions.begin();
  std::set<std::string>::iterator endIt2 = this->mUsedFunctions.end();

  while (it2 != endIt2)
    {
      CEvaluationTree* pTree = this->functionDB->findFunction(*it2);
      assert(pTree);

      if (pTree->getType() == CEvaluationTree::UserDefined)
        {
          this->functionDB->removeFunction(pTree->getKey());
        }

      ++it2;
    }
}

/**
 * Destructor that does nothing.
 */
SEDMLImporter::~SEDMLImporter()
{}

void SEDMLImporter::deleteCopasiModel()
{
  if (this->mpCopasiModel != NULL)
    {
      delete this->mpCopasiModel;
      this->mpCopasiModel = NULL;
    }
}
