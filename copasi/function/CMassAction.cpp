/* Begin CVS Header
   $Source: /Volumes/Home/Users/shoops/cvs/copasi_dev/copasi/function/CMassAction.cpp,v $
   $Revision: 1.21 $
   $Name:  $
   $Author: ssahle $ 
   $Date: 2004/06/22 16:06:02 $
   End CVS Header */

/**
 * CMassAction
 * 
 * Created for Copasi by Stefan Hoops
 * (C) Stefan Hoops 2001
 */

#include "copasi.h"
#include "CMassAction.h"
#include "utilities/utility.h"

#define COPASI_TRACE_CONSTRUCTION
CMassAction::CMassAction(const std::string & name,
                         const CCopasiContainer * pParent):
    CFunction(name, pParent)
{
  CONSTRUCTOR_TRACE;
  setType(CFunction::MassAction);
}

CMassAction::CMassAction(const CFunction & src,
                         const CCopasiContainer * pParent):
    CFunction(src, pParent)
{CONSTRUCTOR_TRACE;}

CMassAction::CMassAction(const TriLogic & reversible,
                         const CCopasiContainer * pParent):
    CFunction((reversible == TriTrue) ?
              "Mass action (reversible)" :
              "Mass action (irreversible)",
              pParent)
{
  CONSTRUCTOR_TRACE;
  if (reversible != TriFalse && reversible != TriTrue)
    CCopasiMessage(CCopasiMessage::ERROR, MCMassAction + 1);

  setType(CFunction::MassAction);
  setReversible(reversible);

  getParameters().add("k1",
                      CFunctionParameter::FLOAT64,
                      "PARAMETER");
  getParameters().add("substrate",
                      CFunctionParameter::VFLOAT64,
                      "SUBSTRATE");
  addUsage("SUBSTRATES", 1, CRange::Infinity);

  if (isReversible() == TriTrue)
    {
      // setName("Mass action (reversible)");
      setDescription("k1 * PRODUCT <substrate_i> "
                     "- k2 * PRODUCT <product_j>");
      getParameters().add("k2",
                          CFunctionParameter::FLOAT64,
                          "PARAMETER");
      getParameters().add("product",
                          CFunctionParameter::VFLOAT64,
                          "PRODUCT");
      addUsage("PRODUCTS", 1, CRange::Infinity);
    }
  else
    {
      // setName("Mass action (irreversible)");
      setDescription("k1 * PRODUCT <substrate_i>");
    }
}
CMassAction::~CMassAction(){DESTRUCTOR_TRACE;}

/*unsigned C_INT32 CMassAction::getParameterPosition(const std::string & name) const
{
  if (isReversible() != TriFalse && isReversible() != TriTrue)
    CCopasiMessage(CCopasiMessage::ERROR, MCMassAction + 1);
 
  if (name == "k1")
    return 0;
  if (name.substr(0, strlen("substrate")) == "substrate")
    return 1;
  if (name == "k2" &&
      isReversible() == TriTrue)
    return 2;
  if (name.substr(0, strlen("product")) == "product" &&
      isReversible() == TriTrue)
    return 3;
 
  return (unsigned C_INT32) - 1;
}*/

/*std::string CMassAction::getSBMLString(const std::vector< std::vector< std::string > > & callParameterNames,
                                       const std::string &r) const
  {
    std::string sf, tmpstr;
    unsigned C_INT32 i, imax;
    const std::vector<std::string> * pFactors;
 
    pFactors = &(callParameterNames[1]);   // first substr.
    imax = pFactors->size();   // NoSubstrates
    if (imax)
      {
        sf = callParameterNames[0][0] + r;           // k1
        for (i = 0; i < imax; i++)
          {
            FixSName((*pFactors)[i], tmpstr);
            sf += "*" + tmpstr;
          }
      }
 
    if (isReversible() == TriFalse)
      return sf;
 
    pFactors = &(callParameterNames[3]);
    imax = pFactors->size();
    if (imax)
      {
        sf += "-" + callParameterNames[2][0] + r;
        for (i = 0; i < imax; i++)
          {
            FixSName((*pFactors)[i], tmpstr);
            sf += "*" + tmpstr;
          }
      }
 
    return sf;
  }*/

C_FLOAT64 CMassAction::calcValue(const CCallParameterPointers & callParameters) const
  {
    unsigned C_INT32 i, imax;
    C_FLOAT64 **Factor;
    C_FLOAT64 Substrates = 0.0, Products = 0.0;

    imax = ((std::vector< C_FLOAT64 *> *)callParameters[1])->size();   // NoSubstrates
    if (imax)
      {
        Substrates = *(C_FLOAT64 *) callParameters[0];           // k1
        Factor =
          &*((std::vector< C_FLOAT64*>*)callParameters[1])->begin();   // first substr.

        for (i = 0; i < imax; i++)
          Substrates *= **(Factor++);
      }

    if (isReversible() == TriFalse)
      return Substrates;

    imax = ((std::vector< C_FLOAT64 *> *)callParameters[3])->size();   // NoProducts
    if (imax)
      {
        Products = *(C_FLOAT64 *) callParameters[2];             // k2
        Factor =
          &*((std::vector< C_FLOAT64*>*)callParameters[3])->begin();   // first product

        for (i = 0; i < imax; i++)
          Products *= **(Factor++);
      }

    return Substrates - Products;
  }

bool CMassAction::dependsOn(const void * parameter,
                            const CCallParameterPointers & callParameters) const
  {
    if (parameter == callParameters[0]) return true;

    std::vector< C_FLOAT64 * >::const_iterator it;
    std::vector< C_FLOAT64 * >::const_iterator end;

    it = ((std::vector< C_FLOAT64 * > *) callParameters[1])->begin();
    end = ((std::vector< C_FLOAT64 * > *) callParameters[1])->end();

    for (; it != end; it++) if (parameter == *it) return true;

    if (isReversible() == TriFalse) return false;

    if (parameter == callParameters[2]) return true;

    it = ((std::vector< C_FLOAT64 * > *) callParameters[3])->begin();
    end = ((std::vector< C_FLOAT64 * > *) callParameters[3])->end();

    for (; it != end; it++) if (parameter == *it) return true;

    return false;
  }
