// CReaction
//
// Derived from Gepasi's cstep.cpp
// (C) Pedro Mendes 1995-2000
//
// Converted for Copasi by Stefan Hoops

#define  COPASI_TRACE_CONSTRUCTION

#include <algorithm>
#include <stdio.h>

#include "copasi.h"
#include "utilities/CGlobals.h"
#include "CReaction.h"
#include "CCompartment.h"
#include "CModel.h"
#include "utilities/readwrite.h"
#include "utilities/CCopasiMessage.h"
#include "utilities/CCopasiException.h"
#include "utilities/utility.h"
#include "utilities/CMethodParameter.h"
#include "function/CFunctionDB.h"
#include "report/CCopasiObjectReference.h"
#include "report/CKeyFactory.h"

C_FLOAT64 CReaction::mDefaultScalingFactor = 1.0;

CReaction::CReaction(const std::string & name,
                     const CCopasiContainer * pParent):
    CCopasiContainer(name, pParent, "Reaction"),
    mKey(CKeyFactory::add("Reaction", this)),
    mName(mObjectName),
    mChemEq("Chemical Equation", this),
    mpFunction(NULL),
    //mParameterDescription(),
    mFlux(0),
    mScaledFlux(0),
    mScalingFactor(&mDefaultScalingFactor),
    mScalingFactor2(&mDefaultScalingFactor),
    mpFunctionCompartment(NULL),
    //mCompartmentNumber(1),
    //mReversible(true),
    mId2Substrates("Substrates", this),
    mId2Products("Products", this),
    mId2Modifiers("Modifiers", this),
    mId2Parameters("Parameters", this),
    mParameters("Parameters", this)
    //mCallParameters(),
    //mCallParameterObjects()
{
  CONSTRUCTOR_TRACE;
  initObjects();
}

CReaction::CReaction(const CReaction & src,
                     const CCopasiContainer * pParent):
    CCopasiContainer(src, pParent),
    mKey(CKeyFactory::add("Reaction", this)),
    mName(mObjectName),
    mChemEq(src.mChemEq, this),
    mpFunction(src.mpFunction),
    //mParameterDescription(src.mParameterDescription),
    mFlux(src.mFlux),
    mScaledFlux(src.mScaledFlux),
    mScalingFactor(src.mScalingFactor),
    mScalingFactor2(src.mScalingFactor2),
    mpFunctionCompartment(src.mpFunctionCompartment),
    //mCompartmentNumber(src.mCompartmentNumber),
    //mReversible(src.mReversible),
    mId2Substrates(src.mId2Substrates, this),
    mId2Products(src.mId2Products, this),
    mId2Modifiers(src.mId2Modifiers, this),
    mId2Parameters(src.mId2Parameters, this),
    mParameters(src.mParameters, this)
    //mCallParameters(src.mCallParameters),
    //mCallParameterObjects(src.mCallParameterObjects)
{
  CONSTRUCTOR_TRACE;
  initObjects();
  if (mpFunction)
    {
      mMap.initializeFromFunctionParameters(mpFunction->getParameters());
    }
}

CReaction::~CReaction()
{
  CKeyFactory::remove(mKey);
  cleanup();
  DESTRUCTOR_TRACE;
}

void CReaction::cleanup()
{
  mId2Substrates.cleanup();
  mId2Products.cleanup();
  mId2Modifiers.cleanup();
  mId2Parameters.cleanup();

  // TODO: mMap.cleanup();

  //mParameterDescription.cleanup();
}

C_INT32 CReaction::load(CReadConfig & configbuffer)
{
  C_INT32 Fail = 0;

  std::string KinType;

  if ((Fail = configbuffer.getVariable("Step", "string", &mName,
                                       CReadConfig::SEARCH)))
    return Fail;

  std::string ChemEq;

  if ((Fail = configbuffer.getVariable("Equation", "string", &ChemEq)))
    return Fail;

  setChemEq(ChemEq);

  if ((Fail = configbuffer.getVariable("KineticType", "string", &KinType)))
    return Fail;

  setFunction(KinType);

  if (mpFunction == NULL)
    return Fail = 1;

  bool revers;
  if ((Fail = configbuffer.getVariable("Reversible", "bool", &revers,
                                       CReadConfig::SEARCH)))
    return Fail;
  mChemEq.setReversibility(revers); // TODO: this should be consistent with the ChemEq string

  if (configbuffer.getVersion() < "4")
    Fail = loadOld(configbuffer);
  else
    Fail = loadNew(configbuffer);

  return Fail;
}

C_INT32 CReaction::save(CWriteConfig & configbuffer)
{
  C_INT32 Fail = 0;
  C_INT32 Size = 0;
  C_INT32 i = 0;

  if ((Fail = configbuffer.setVariable("Step", "string", &mName)))
    return Fail;

  std::string ChemEq = mChemEq.getChemicalEquation();
  if ((Fail = configbuffer.setVariable("Equation", "string", &ChemEq)))
    return Fail;

  std::string KinType = mpFunction->getName();

  if ((Fail = configbuffer.setVariable("KineticType", "string", &KinType)))
    return Fail;

  /*if ((Fail = configbuffer.setVariable("Reversible", "bool", &mReversible)))
    return Fail;*/ //TODO: check: this info should be in the chemEq

  Size = mId2Substrates.size();

  if ((Fail = configbuffer.setVariable("Substrates", "C_INT32", &Size)))
    return Fail;

  for (i = 0; i < Size; i++)
    {
      if ((Fail = configbuffer.setVariable("Identifier", "string",
                                           &mId2Substrates[i]->mIdentifierName)))
        return Fail;

      if ((Fail = configbuffer.setVariable("Compartment", "string",
                                           &mId2Substrates[i]->mCompartmentName)))
        return Fail;

      if ((Fail = configbuffer.setVariable("Metabolite", "string",
                                           &mId2Substrates[i]->mMetaboliteName)))
        return Fail;
    }

  Size = mId2Products.size();

  if ((Fail = configbuffer.setVariable("Products", "C_INT32", &Size)))
    return Fail;

  for (i = 0; i < Size; i++)
    {
      if ((Fail = configbuffer.setVariable("Identifier", "string",
                                           &mId2Products[i]->mIdentifierName)))
        return Fail;

      if ((Fail = configbuffer.setVariable("Compartment", "string",
                                           &mId2Products[i]->mCompartmentName)))
        return Fail;

      if ((Fail = configbuffer.setVariable("Metabolite", "string",
                                           &mId2Products[i]->mMetaboliteName)))
        return Fail;
    }

  Size = mId2Modifiers.size();

  if ((Fail = configbuffer.setVariable("Modifiers", "C_INT32", &Size)))
    return Fail;

  for (i = 0; i < Size; i++)
    {
      if ((Fail = configbuffer.setVariable("Identifier", "string",
                                           &mId2Modifiers[i]->mIdentifierName)))
        return Fail;

      if ((Fail = configbuffer.setVariable("Compartment", "string",
                                           &mId2Modifiers[i]->mCompartmentName)))
        return Fail;

      if ((Fail = configbuffer.setVariable("Metabolite", "string",
                                           &mId2Modifiers[i]->mMetaboliteName)))
        return Fail;
    }

  Size = mId2Parameters.size();

  if ((Fail = configbuffer.setVariable("Constants", "C_INT32", &Size)))
    return Fail;

  for (i = 0; i < Size; i++)
    {
      if ((Fail = configbuffer.setVariable("Identifier", "string",
                                           &mId2Parameters[i]->mIdentifierName)))
        return Fail;

      if ((Fail = configbuffer.setVariable("Value", "C_FLOAT64",
                                           &mId2Parameters[i]->mValue)))
        return Fail;
    }

  return Fail;
}

C_INT32 CReaction::saveOld(CWriteConfig & configbuffer,
                           const CCopasiVectorN< CMetab > &metabolites)
{
  C_INT32 Fail = 0;
  C_INT32 Size = 0;
  C_INT32 i = 0, j = 0, s = 0, c = -1;
  char strtmp[32];
  CCopasiVector < CChemEqElement > reactants;
  s = metabolites.size();
  if ((Fail = configbuffer.setVariable("Step", "string", &mName)))
    return Fail;
  if ((Fail = configbuffer.setVariable("Equation", "string", &mChemEq)))
    return Fail;
  std::string KinType = mpFunction->getName();
  if ((Fail = configbuffer.setVariable("KineticType", "string", &KinType)))
    return Fail;
  if ((Fail = configbuffer.setVariable("Flux", "C_FLOAT64", &mFlux)))
    return Fail;

  bool revers = mChemEq.getReversibility();
  if ((Fail = configbuffer.setVariable("Reversible", "bool", &revers)))
    return Fail;
  Size = mChemEq.getSubstrates().size();
  if ((Fail = configbuffer.setVariable("Substrates", "C_INT32", &Size)))
    return Fail;
  Size = mChemEq.getProducts().size();
  if ((Fail = configbuffer.setVariable("Products", "C_INT32", &Size)))
    return Fail;
  Size = mId2Modifiers.size();
  if ((Fail = configbuffer.setVariable("Modifiers", "C_INT32", &Size)))
    return Fail;
  Size = mId2Parameters.size();
  if ((Fail = configbuffer.setVariable("Constants", "C_INT32", &Size)))
    return Fail;
  reactants = mChemEq.getSubstrates();
  Size = reactants.size();
  for (i = 0; i < Size; i++)
    {
      for (j = 0, c = -1; j < s; j++)
        if (reactants[i]->getMetabolite().getName() == metabolites[j]->getName())
          {
            c = j;
            break;
          }
      if (c == -1)
        return - 1;
      sprintf(strtmp, "Subs%ld", i);
      if ((Fail = configbuffer.setVariable(strtmp, "C_INT32", (void *) & c)))
        return Fail;
    }
  reactants = mChemEq.getProducts();
  Size = reactants.size();
  for (i = 0; i < Size; i++)
    {
      for (j = 0, c = -1; j < s; j++)
        if (reactants[i]->getMetabolite().getName() == metabolites[j]->getName())
          {
            c = j;
            break;
          }
      if (c == -1)
        return - 1;
      sprintf(strtmp, "Prod%ld", i);
      if ((Fail = configbuffer.setVariable(strtmp, "C_INT32", (void *) & c)))
        return Fail;
    }
  Size = mId2Modifiers.size();
  for (i = 0; i < Size; i++)
    {
      for (j = 0, c = -1; j < s; j++)
        if (mId2Modifiers[i]->mMetaboliteName == metabolites[j]->getName())
          {
            c = j;
            break;
          }
      if (c == -1)
        return - 1;
      sprintf(strtmp, "Modf%ld", i);
      if ((Fail = configbuffer.setVariable(strtmp, "C_INT32", (void *) & c)))
        return Fail;
    }
  Size = mId2Parameters.size();
  for (i = 0; i < Size; i++)
    {
      sprintf(strtmp, "Param%ld", i);
      if ((Fail = configbuffer.setVariable(strtmp, "C_FLOAT64", (void *) & mId2Parameters[i]->mValue)))
        return Fail;
    }
  return Fail;
}

void CReaction::saveSBML(std::ofstream &fout, C_INT32 r)
{
  std::string tmpstr, tmpstr2;
  unsigned C_INT32 i;
  CCopasiVector < CChemEqElement > rr;

  FixSName(mName, tmpstr);
  fout << "\t\t\t<reaction name=\"" << tmpstr << "\"";
  fout << " reversible=\"";
  if (mChemEq.getReversibility())
    fout << "true";
  else
    fout << "false";
  fout << "\">" << std::endl;
  fout << "\t\t\t\t<listOfReactants>" << std::endl;
  // check if we need to reference the dummy metabolite
  rr = mChemEq.getSubstrates();
  if (rr.size() == 0)
    fout << "\t\t\t\t\t<specieReference specie=\"_void_\" stoichiometry=\"1\"/>" << std::endl;
  else
    {
      // write them out
      for (i = 0; i < rr.size(); i++)
        {
          tmpstr2 = rr[i]->getMetaboliteName();
          FixSName(tmpstr2, tmpstr);
          fout << "\t\t\t\t\t<specieReference specie=\"" << tmpstr << "\"";
          fout << " stoichiometry=\"" << rr[i]->getMultiplicity() << "\"/>" << std::endl;
        }
    }
  fout << "\t\t\t\t</listOfReactants>" << std::endl;
  fout << "\t\t\t\t<listOfProducts>" << std::endl;
  // check if we need to reference the dummy metabolite
  rr = mChemEq.getProducts();
  if (rr.size() == 0)
    fout << "\t\t\t\t\t<specieReference specie=\"_void_\" stoichiometry=\"1\"/>" << std::endl;
  else
    {
      // write them out
      for (i = 0; i < rr.size(); i++)
        {
          tmpstr2 = rr[i]->getMetaboliteName();
          FixSName(tmpstr2, tmpstr);
          fout << "\t\t\t\t\t<specieReference specie=\"" << tmpstr << "\"";
          fout << " stoichiometry=\"" << rr[i]->getMultiplicity() << "\"/>" << std::endl;
        }
    }
  fout << "\t\t\t\t</listOfProducts>" << std::endl;
  fout << "\t\t\t\t<kineticLaw formula=\"";
  // kinetic function string
  tmpstr2 = StringPrint("_%ld", r);
  //fout << mpFunction->getSBMLString(mCallParameterObjects, tmpstr2); //TODO
  fout << "\">" << std::endl;
  fout << "\t\t\t\t\t<listOfParameters>" << std::endl;
  for (i = 0; i < mId2Parameters.size(); i++)
    {
      FixSName(mId2Parameters[i]->mIdentifierName, tmpstr);
      fout << "\t\t\t\t\t\t<parameter name=\"" + tmpstr;
      fout << "_" << r << "\" value=\"" << mId2Parameters[i]->mValue << "\"/>" << std::endl;
    }
  fout << "\t\t\t\t\t</listOfParameters>" << std::endl;
  fout << "\t\t\t\t</kineticLaw>" << std::endl;
  fout << "\t\t\t</reaction>" << std::endl;
}

C_INT32 CReaction::getSubstrateNumber() const
  {return mChemEq.getSubstrates().size();}

C_INT32 CReaction::getProductNumber() const
  {return mChemEq.getProducts().size();}

CCopasiVector < CReaction::CId2Metab > &CReaction::getId2Substrates()
{return mId2Substrates;}

CCopasiVector < CReaction::CId2Metab > &CReaction::getId2Products()
{return mId2Products;}

CCopasiVectorN < CReaction::CId2Metab > &CReaction::getId2Modifiers()
{return mId2Modifiers;}

const CCopasiVectorN < CReaction::CId2Param > &CReaction::getId2Parameters() const
  {return mId2Parameters;}

CCopasiVectorN < CReaction::CId2Param > &CReaction::getId2Parameters()
{return mId2Parameters;}

std::string CReaction::getKey() const
  {return mKey;}

const std::string & CReaction::getName() const
  {return mName;}

const CChemEq & CReaction::getChemEq() const
  {return mChemEq;}

const CFunction & CReaction::getFunction() const
  {return *mpFunction;}

const C_FLOAT64 & CReaction::getFlux() const
  {return mFlux;}

const C_FLOAT64 & CReaction::getScaledFlux() const
  {return mScaledFlux;}

bool CReaction::isReversible() const
  {return mChemEq.getReversibility();}

bool CReaction::setName(const std::string & name)
{
  CCopasiContainer * pParent = getObjectParent();
  if (pParent)
    if (pParent->isNameVector())
      if (pParent->getIndex(name) != C_INVALID_INDEX)
        return false;

  mName = name;
  return true;
}

void CReaction::setChemEq(const std::string & chemEq)
{/*mReversible = */mChemEq.setChemicalEquation(chemEq);}

void CReaction::setReversible(bool reversible)
{mChemEq.setReversibility(reversible);}

void CReaction::setFunction(const std::string & functionName)
{
  mpFunction = Copasi->pFunctionDB->findLoadFunction(functionName);
  if (!mpFunction)
    CCopasiMessage(CCopasiMessage::ERROR, MCReaction + 1, functionName.c_str());

  mMap.initializeFromFunctionParameters(mpFunction->getParameters());
  initializeParameters();
  //initCallParameters();
  //initCallParameterObjects();
}

void CReaction::setParameter(const std::string & parameterName, C_FLOAT64 value)
{
  mParameters[parameterName]->setValue(value);
}

void CReaction::initializeParameters()
{
  unsigned C_INT32 i;
  unsigned C_INT32 imax = mMap.getFunctionParameters().getNumberOfParametersByUsage("PARAMETER");
  unsigned C_INT32 pos;
  std::string name;
  CParameter param;

  param.setType(CParameter::DOUBLE);
  param.setValue(1.0);

  for (i = 0, pos = 0; i < imax; ++i)
    {
      name = mMap.getFunctionParameters().getParameterByUsage("PARAMETER", pos).getName();
      param.setName(name);
      mParameters.add(param);
    }
}

void CReaction::compile(const CCopasiVectorNS < CCompartment > & compartments)
{
  unsigned C_INT32 i;

  mChemEq.compile(compartments);

  for (i = 0; i < mId2Substrates.size(); i++)
    mId2Substrates[i]->mpMetabolite =
      compartments[mId2Substrates[i]->mCompartmentName]->
      getMetabolites()[mId2Substrates[i]->mMetaboliteName];

  for (i = 0; i < mId2Products.size(); i++)
    mId2Products[i]->mpMetabolite =
      compartments[mId2Products[i]->mCompartmentName]->
      getMetabolites()[mId2Products[i]->mMetaboliteName];

  for (i = 0; i < mId2Modifiers.size(); i++)
    mId2Modifiers[i]->mpMetabolite =
      compartments[mId2Modifiers[i]->mCompartmentName]->
      getMetabolites()[mId2Modifiers[i]->mMetaboliteName];

  if (mpFunction)
    {
      mMap.initCallParameters();

      //initCallParameters();
      setCallParameters();
      //checkCallParameters();

      //initCallParameterObjects();
      setCallParameterObjects();
      //checkCallParameterObjects();

      mMap.checkCallParameters();
    }

  setScalingFactor();
}

C_INT32 CReaction::loadNew(CReadConfig & configbuffer)
{
  C_INT32 Fail = 0;
  C_INT32 Size;
  C_INT32 i;

  if ((Fail = configbuffer.getVariable("Substrates", "C_INT32", &Size)))
    return Fail;

  mId2Substrates.resize(Size);

  for (i = 0; i < Size; i++)
    {
      if ((Fail = configbuffer.getVariable("Identifier", "string",
                                           &mId2Substrates[i]->mIdentifierName)))
        return Fail;

      if ((Fail = configbuffer.getVariable("Compartment", "string",
                                           &mId2Substrates[i]->mCompartmentName)))
        return Fail;

      if ((Fail = configbuffer.getVariable("Metabolite", "string",
                                           &mId2Substrates[i]->mMetaboliteName)))
        return Fail;
    }

  if ((Fail = configbuffer.getVariable("Products", "C_INT32", &Size)))
    return Fail;

  mId2Products.resize(Size);

  for (i = 0; i < Size; i++)
    {
      if ((Fail = configbuffer.getVariable("Identifier", "string",
                                           &mId2Products[i]->mIdentifierName)))
        return Fail;

      if ((Fail = configbuffer.getVariable("Compartment", "string",
                                           &mId2Products[i]->mCompartmentName)))
        return Fail;

      if ((Fail = configbuffer.getVariable("Metabolite", "string",
                                           &mId2Products[i]->mMetaboliteName)))
        return Fail;
    }

  if ((Fail = configbuffer.getVariable("Modifiers", "C_INT32", &Size)))
    return Fail;

  mId2Modifiers.resize(Size);

  for (i = 0; i < Size; i++)
    {
      if ((Fail = configbuffer.getVariable("Identifier", "string",
                                           &mId2Modifiers[i]->mIdentifierName)))
        return Fail;

      if ((Fail = configbuffer.getVariable("Compartment", "string",
                                           &mId2Modifiers[i]->mCompartmentName)))
        return Fail;

      if ((Fail = configbuffer.getVariable("Metabolite", "string",
                                           &mId2Modifiers[i]->mMetaboliteName)))
        return Fail;
    }

  if ((Fail = configbuffer.getVariable("Constants", "C_INT32", &Size)))
    return Fail;

  mId2Parameters.resize(Size);

  for (i = 0; i < Size; i++)
    {
      if ((Fail = configbuffer.getVariable("Identifier", "string",
                                           &mId2Parameters[i]->mIdentifierName)))
        return Fail;

      if ((Fail = configbuffer.getVariable("Value", "C_FLOAT64",
                                           &mId2Parameters[i]->mValue)))
        return Fail;
    }

  return Fail;
}

C_INT32 CReaction::loadOld(CReadConfig & configbuffer)
{
  std::string name;

  C_INT32 Fail = 0;
  unsigned C_INT32 SubstrateSize, ProductSize, ModifierSize, ParameterSize;
  unsigned C_INT32 i, imax, pos;
  C_INT32 index;

  CFunctionParameter::DataType Type;

  configbuffer.getVariable("Substrates", "C_INT32", &SubstrateSize);
  mId2Substrates.resize(std::min(SubstrateSize, usageRangeSize("SUBSTRATE")));

  configbuffer.getVariable("Products", "C_INT32", &ProductSize);
  mId2Products.resize(std::min(ProductSize, usageRangeSize("PRODUCT")));

  configbuffer.getVariable("Modifiers", "C_INT32", &ModifierSize);
  mId2Modifiers.resize(std::min(ModifierSize, usageRangeSize("MODIFIER")));

  configbuffer.getVariable("Constants", "C_INT32", &ParameterSize);
  mId2Parameters.resize(std::min(ParameterSize, usageRangeSize("PARAMETER")));

  // Construct Id2Substrate
  imax = mId2Substrates.size();

  for (i = 0, pos = 0, Type = CFunctionParameter::INT32; i < imax; i++)
    {
      name = StringPrint("Subs%d", i);
      configbuffer.getVariable(name, "C_INT32", &index);

      mId2Substrates[i]->mMetaboliteName =
        (*Copasi->pOldMetabolites)[index]->getName();

      if (Type < CFunctionParameter::VINT32)
        Type =
          mMap.getFunctionParameters().getParameterByUsage("SUBSTRATE", pos).getType();

      mId2Substrates[i]->mIdentifierName =
        mMap.getFunctionParameters()[pos - 1]->getName();

      if (Type >= CFunctionParameter::VINT32)
        mId2Substrates[i]->mIdentifierName += StringPrint("_%ld", i);
    }

  for (i = imax; i < SubstrateSize; i++)
    {
      name = StringPrint("Subs%d", i);
      configbuffer.getVariable(name, "C_INT32", &index);
    }

  // Construct Id2Product
  imax = mId2Products.size();

  for (i = 0, pos = 0, Type = CFunctionParameter::INT32; i < imax; i++)
    {
      name = StringPrint("Prod%d", i);
      configbuffer.getVariable(name, "C_INT32", &index);

      mId2Products[i]->mMetaboliteName =
        (*Copasi->pOldMetabolites)[index]->getName();

      if (Type < CFunctionParameter::VINT32)
        Type =
          mMap.getFunctionParameters().getParameterByUsage("PRODUCT", pos).getType();

      mId2Products[i]->mIdentifierName =
        mMap.getFunctionParameters()[pos - 1]->getName();

      if (Type >= CFunctionParameter::VINT32)
        mId2Products[i]->mIdentifierName += StringPrint("_%ld", i);
    }

  for (i = imax; i < ProductSize; i++)
    {
      name = StringPrint("Prod%d", i);
      configbuffer.getVariable(name, "C_INT32", &index);
    }

  // Construct Id2Modifier
  imax = mId2Modifiers.size();

  for (i = 0, pos = 0, Type = CFunctionParameter::INT32; i < imax; i++)
    {
      name = StringPrint("Modf%d", i);
      configbuffer.getVariable(name, "C_INT32", &index);

      mId2Modifiers[i]->mMetaboliteName =
        (*Copasi->pOldMetabolites)[index]->getName();

      if (Type < CFunctionParameter::VINT32)
        Type =
          mMap.getFunctionParameters().getParameterByUsage("MODIFIER", pos).getType();

      mId2Modifiers[i]->mIdentifierName =
        mMap.getFunctionParameters()[pos - 1]->getName();

      if (Type >= CFunctionParameter::VINT32)
        mId2Modifiers[i]->mIdentifierName += StringPrint("_%ld", i);
    }

  for (i = imax; i < ModifierSize; i++)
    {
      name = StringPrint("Modf%d", i);
      configbuffer.getVariable(name, "C_INT32", &index);
    }

  // Construct Id2Parameter
  imax = mId2Parameters.size();

  for (i = 0, pos = 0, Type = CFunctionParameter::INT32; i < imax; i++)
    {
      name = StringPrint("Param%d", i);
      configbuffer.getVariable(name, "C_FLOAT64",
                               &mId2Parameters[i]->mValue);

      if (Type < CFunctionParameter::VINT32)
        Type =
          mMap.getFunctionParameters().getParameterByUsage("PARAMETER", pos).getType();

      mId2Parameters[i]->mIdentifierName =
        mMap.getFunctionParameters()[pos - 1]->getName();

      if (Type >= CFunctionParameter::VINT32)
        mId2Parameters[i]->mIdentifierName += StringPrint("_%ld", i);
    }

  for (i = imax; i < ParameterSize; i++)
    {
      name = StringPrint("Param%d", i);
      configbuffer.getVariable(name, "C_INT32", &index);
    }

  return Fail;
}

CReaction::CId2Metab::CId2Metab(const std::string & name,
                                const CCopasiContainer * pParent):
    CCopasiContainer(name, pParent, "Reaction"),
    mIdentifierName(mObjectName),
    mMetaboliteName(),
    mCompartmentName(),
    mpMetabolite(NULL)
{}

CReaction::CId2Metab::CId2Metab(const CId2Metab & src,
                                const CCopasiContainer * pParent):
    CCopasiContainer(src, pParent),
    mIdentifierName(mObjectName),
    mMetaboliteName(src.mMetaboliteName),
    mCompartmentName(src.mCompartmentName),
    mpMetabolite(src.mpMetabolite)
{}

CReaction::CId2Metab::~CId2Metab() {}

void CReaction::CId2Metab::cleanup() {}

CReaction::CId2Param::CId2Param(const std::string & name,
                                const CCopasiContainer * pParent):
    CCopasiContainer(name, pParent, "Reaction"),
    mIdentifierName(mObjectName),
    mValue(0)
{}

CReaction::CId2Param::CId2Param(const CId2Param & src,
                                const CCopasiContainer * pParent):
    CCopasiContainer(src, pParent),
    mIdentifierName(mObjectName),
    mValue(src.mValue)
{}

CReaction::CId2Param::~CId2Param() {}

void CReaction::CId2Param::cleanup() {}

void CReaction::old2New(const CCopasiVector< CMetab > & metabolites)
{
  std::string Name;
  unsigned C_INT32 i, j, s, z;

  z = metabolites.size();
  s = mId2Substrates.size();
  for (i = 0; i < s; i++)
    {
      Name = mId2Substrates[i]->mMetaboliteName;
      for (j = 0; j < z; j++)
        if (Name == metabolites[j]->getName())
          break;
      mId2Substrates[i]->mCompartmentName =
        metabolites[j]->getCompartment()->getName();
    }

  s = mId2Products.size();
  for (i = 0; i < s; i++)
    {
      Name = mId2Products[i]->mMetaboliteName;
      for (j = 0; j < z; j++)
        if (Name == metabolites[j]->getName())
          break;
      mId2Products[i]->mCompartmentName =
        metabolites[j]->getCompartment()->getName();
    }

  s = mId2Modifiers.size();
  for (i = 0; i < s; i++)
    {
      Name = mId2Modifiers[i]->mMetaboliteName;
      for (j = 0; j < z; j++)
        if (Name == metabolites[j]->getName())
          break;
      mId2Modifiers[i]->mCompartmentName =
        metabolites[j]->getCompartment()->getName();
    }
}

C_FLOAT64 CReaction::calculate()
{
  mFlux = *mScalingFactor * mpFunction->calcValue(mMap.getPointers());
  mScaledFlux = *mScalingFactor2 * mFlux;
  return mFlux;
}

C_FLOAT64 CReaction::calculatePartialDerivative(C_FLOAT64 & xi,
    const C_FLOAT64 & derivationFactor,
    const C_FLOAT64 & resolution)
{
  if (mpFunction->dependsOn(&xi, mMap.getPointers()))
    {
      C_FLOAT64 store = xi;
      C_FLOAT64 f1, f2;
      C_FLOAT64 tmp =
        (store < resolution) ? resolution * (1.0 + derivationFactor) : store;

      xi = tmp * (1.0 + derivationFactor);
      f1 = calculate();

      xi = tmp * (1.0 - derivationFactor);
      f2 = calculate();

      xi = store;

      return *mScalingFactor * (f1 - f2) / (2.0 * tmp * derivationFactor);
    }
  else
    return 0.0;
}

void CReaction::CId2Metab::setIdentifierName(const std::string & identifierName)
{
  mIdentifierName = identifierName;
}

const std::string & CReaction::CId2Metab::getIdentifierName() const
  {
    return mIdentifierName;
  }

void CReaction::CId2Metab::setMetaboliteName(const std::string & metaboliteName)
{
  mMetaboliteName = metaboliteName;
}

const std::string & CReaction::CId2Metab::getMetaboliteName() const
  {
    return mMetaboliteName;
  }

void CReaction::CId2Metab::setCompartmentName(const std::string & compartmentName)
{
  mCompartmentName = compartmentName;
}

const std::string & CReaction::CId2Metab::getCompartmentName() const
  {
    return mCompartmentName;
  }

const CMetab * CReaction::CId2Metab::getMetabolite() const
  {
    return mpMetabolite;
  }

void CReaction::CId2Param::setIdentifierName(const std::string & identifierName)
{
  mIdentifierName = identifierName;
}

const std::string & CReaction::CId2Param::getName() const
  {return mIdentifierName;}

void CReaction::CId2Param::setValue(const C_FLOAT64 & value)
{
  mValue = value;
}

const C_FLOAT64 & CReaction::CId2Param::getValue() const
  {
    return mValue;
  }

/**
 * Returns the address of mValue
 */ 
//void* CReaction::CId2Param::getValueAddr()
//{
//  return &mValue;
//}

/**
 * Returns the address of mFlux
 */ 
//void* CReaction::getFluxAddr()
//{
//  return &mFlux;
//}

/**
 * Returns the index of the parameter
 */
C_INT32 CReaction::findPara(std::string &Target) const
  {
    unsigned C_INT32 i;
    std::string name;

    for (i = 0; i < mId2Parameters.size(); i++)
      {
        name = mId2Parameters[i]->getName();

        if (name == Target)
          return i;
      }

    return - 1;
  }

void CReaction::setCallParameters()
{
  unsigned C_INT32 i, imax;
  unsigned C_INT32 Index;

  CFunctionParameter::DataType dataType;

  imax = mId2Substrates.size();

  for (i = 0; i < imax; i++)
    {
      Index = mMap.findParameterByName(mId2Substrates[i]->mIdentifierName, dataType);

      if (dataType < CFunctionParameter::VINT32)
        mMap.getPointers()[Index] =
          & mId2Substrates[i]->mpMetabolite->getConcentration();
      else
        ((std::vector<const void *> *) mMap.getPointers()[Index])->
        push_back(& mId2Substrates[i]->mpMetabolite->getConcentration());
    }

  imax = mId2Products.size();

  for (i = 0; i < imax; i++)
    {
      Index = mMap.findParameterByName(mId2Products[i]->mIdentifierName, dataType);

      if (dataType < CFunctionParameter::VINT32)
        mMap.getPointers()[Index] =
          & mId2Products[i]->mpMetabolite->getConcentration();
      else
        ((std::vector< const void * > *) mMap.getPointers()[Index])->
        push_back(& mId2Products[i]->mpMetabolite->getConcentration());
    }

  imax = mId2Modifiers.size();

  for (i = 0; i < imax; i++)
    {
      Index = mMap.findParameterByName(mId2Modifiers[i]->mIdentifierName, dataType);

      if (dataType < CFunctionParameter::VINT32)
        mMap.getPointers()[Index] =
          & mId2Modifiers[i]->mpMetabolite->getConcentration();
      else
        ((std::vector< const void * > *) mMap.getPointers()[Index])->
        push_back(& mId2Modifiers[i]->mpMetabolite->getConcentration());
    }

  imax = mId2Parameters.size();

  for (i = 0; i < imax; i++)
    {
      Index = mMap.findParameterByName(mId2Parameters[i]->mIdentifierName, dataType);

      if (dataType < CFunctionParameter::VINT32)
        mMap.getPointers()[Index] = &mId2Parameters[i]->mValue;
      else
        ((std::vector< void * > *) mMap.getPointers()[Index])->
        push_back(&mId2Parameters[i]->mValue);
    }
}

void CReaction::setCallParameterObjects()
{
  unsigned C_INT32 i, imax;
  unsigned C_INT32 Index;
  CFunctionParameter::DataType dataType;

  imax = mId2Substrates.size();
  for (i = 0; i < imax; i++)
    {
      Index = mMap.findParameterByName(mId2Substrates[i]->mIdentifierName, dataType);
      if (dataType < CFunctionParameter::VINT32)
        mMap.getObjects()[Index] = mId2Substrates[i]->getMetabolite();
      else
        {
          ((std::vector<const CCopasiObject *> *) mMap.getObjects()[Index])->
          push_back(mId2Substrates[i]->getMetabolite());
        }
    }

  imax = mId2Products.size();

  for (i = 0; i < imax; i++)
    {
      Index = mMap.findParameterByName(mId2Products[i]->mIdentifierName, dataType);
      if (dataType < CFunctionParameter::VINT32)
        mMap.getObjects()[Index] = mId2Products[i]->getMetabolite();
      else
        ((std::vector< const CCopasiObject * > *) mMap.getObjects()[Index])->
        push_back(mId2Products[i]->getMetabolite());
    }

  imax = mId2Modifiers.size();

  for (i = 0; i < imax; i++)
    {
      Index = mMap.findParameterByName(mId2Modifiers[i]->mIdentifierName, dataType);
      if (dataType < CFunctionParameter::VINT32)
        mMap.getObjects()[Index] = mId2Modifiers[i]->getMetabolite();
      else
        ((std::vector< const CCopasiObject * > *) mMap.getObjects()[Index])->
        push_back(mId2Modifiers[i]->getMetabolite());
    }

  imax = mId2Parameters.size();

  for (i = 0; i < imax; i++)
    {
      Index = mMap.findParameterByName(mId2Parameters[i]->mIdentifierName, dataType);
      if (dataType < CFunctionParameter::VINT32)
        mMap.getObjects()[Index] = mId2Parameters[i];
      else
        ((std::vector< CCopasiObject * > *) mMap.getObjects()[Index])->
        push_back(mId2Parameters[i]);
    }
}

unsigned C_INT32 CReaction::usageRangeSize(const std::string & usage) const
  {
    const CUsageRange * pUsageRange = NULL;
    C_INT32 Size = 0;

    try
      {
        pUsageRange = mMap.getFunctionParameters().getUsageRanges()[usage];
      }

    catch (CCopasiException Exception)
      {
        if ((MCCopasiVector + 1) != Exception.getMessage().getNumber())
          throw Exception;

        pUsageRange = NULL;
      }

    if (pUsageRange)
      Size = std::max(pUsageRange->getLow(), pUsageRange->getHigh());

    return Size;
  }

// Added by cvg
const CMetab * CReaction::findSubstrate(std::string ident_name) const
  {
    for (unsigned C_INT32 i = 0; i < mId2Substrates.size(); i++)
      {
        if (mId2Substrates[i]->mIdentifierName == ident_name)
          {
            // found it
            return mId2Substrates[i]->mpMetabolite;
          }
      }
    // If we get here, we found nothing
    return NULL;
  }

const CMetab * CReaction::findModifier(std::string ident_name) const
  {
    for (unsigned C_INT32 i = 0; i < mId2Modifiers.size(); i++)
      {
        if (mId2Modifiers[i]->mIdentifierName == ident_name)
          {
            // found it
            return mId2Modifiers[i]->mpMetabolite;
          }
      }

    // If we get here, we found nutting
    return 0;
  }

unsigned C_INT32 CReaction::getCompartmentNumber() const
  {return mChemEq.getCompartmentNumber();}

void CReaction::setScalingFactor()
{
  if (1 == getCompartmentNumber())
    mScalingFactor = & mChemEq.getBalances()[0]->getMetabolite().getCompartment()->getVolume();
  else
    mScalingFactor = &mDefaultScalingFactor;

#ifdef XXXX
  if (mpFunctionCompartment)
    {
      // should propably check if the compartment appears in the chemical equation
      mScalingFactor = & mpFunctionCompartment->getVolume();
    }
  else
    {
      try
      {mScalingFactor = & mChemEq.CheckAndGetFunctionCompartment()->getVolume();}
      catch (CCopasiException Exc)
        {
          unsigned C_INT32 nr = Exc.getMessage().getNumber();
          if ((MCChemEq + 2 == nr) || (MCChemEq + 3 == nr))
            CCopasiMessage(CCopasiMessage::ERROR, MCReaction + 2, getName().c_str());
          if (MCChemEq + 1 == nr)
            CCopasiMessage(CCopasiMessage::ERROR, MCReaction + 3, getName().c_str());
          throw;
        }
    }
#endif // XXXX

  CModel * pModel = (CModel *) getObjectAncestor("Model");
  if (pModel)
    mScalingFactor2 = & pModel->getQuantity2NumberFactor();
  else
    mScalingFactor2 = & mDefaultScalingFactor;
}

void CReaction::setReactantsFromChemEq()
{
  C_INT32 i, nsub, nprod;
  unsigned C_INT32 pos;
  CCopasiVector <CChemEqElement > sub;
  CCopasiVector <CChemEqElement > prod;
  CFunctionParameter::DataType Type = CFunctionParameter::FLOAT64;

  if (1) //(mChemEq.initialized()) TODO: is this necessary?
    {
      sub = mChemEq.getSubstrates();
      nsub = sub.size();
      prod = mChemEq.getProducts();
      nprod = prod.size();
      mId2Substrates.resize(nsub);
      for (i = 0, pos = 0; i < nsub; i++)
        {
          mId2Substrates[i]->mMetaboliteName = sub[i]->getMetaboliteName();
          mId2Substrates[i]->mCompartmentName = sub[i]->getMetabolite().getCompartment()->getName();
          if (Type < CFunctionParameter::VINT32)
            Type = mMap.getFunctionParameters().getParameterByUsage("SUBSTRATE", pos).getType();
          mId2Substrates[i]->mIdentifierName = mMap.getFunctionParameters()[pos - 1]->getName();
          if (Type >= CFunctionParameter::VINT32)
            mId2Substrates[i]->mIdentifierName += StringPrint("_%ld", i);
        }
      mId2Products.resize(nprod);
      for (i = 0, pos = 0; i < nprod; i++)
        {
          mId2Products[i]->mMetaboliteName = prod[i]->getMetaboliteName();
          mId2Products[i]->mCompartmentName = prod[i]->getMetabolite().getCompartment()->getName();
          if (Type < CFunctionParameter::VINT32)
            Type = mMap.getFunctionParameters().getParameterByUsage("PRODUCT", pos).getType();
          mId2Products[i]->mIdentifierName = mMap.getFunctionParameters()[pos - 1]->getName();
          if (Type >= CFunctionParameter::VINT32)
            mId2Products[i]->mIdentifierName += StringPrint("_%ld", i);
        }
    }
}

void CReaction::compileChemEq(const CCopasiVectorN < CCompartment > & compartments)
{mChemEq.compile(compartments);}

void CReaction::setFunctionCompartment(const CCompartment* comp)
{mpFunctionCompartment = comp;}

const CCompartment* CReaction::getFunctionCompartment() const
  {return mpFunctionCompartment;}

const CCallParameterPointers & CReaction::getCallParameterObjects() const
  {return mMap.getObjects();}

void CReaction::initObjects()
{
  addObjectReference("Name", mName);
  addObjectReference("ChemicalEquation", mChemEq);
  //addObjectReference("FunctionParameters", mParameterDescription);
  addObjectReference("Flux", mFlux);
  addObjectReference("ScaledFlux", mScaledFlux);
  //addObjectReference("Reversible", mReversible);
  add(&mId2Substrates);
  add(&mId2Products);
  add(&mId2Modifiers);
  //add(&mId2Parameters);
  add(&mParameters);
  //addObjectReference("CallParameters", mCallParameters);
  //addObjectReference("CallParameterObjects", mCallParameterObjects);
  //add(&mMap);
}
