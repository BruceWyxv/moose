//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef MULTIAPPMUTABLECOEFFICIENTSTRANSFER_H
#define MULTIAPPMUTABLECOEFFICIENTSTRANSFER_H

#include "MultiAppTransfer.h"

#include "MutableCoefficientsInterface.h"

class MultiAppFXTransfer;

template <>
InputParameters validParams<MultiAppFXTransfer>();

/**
 * Transfers mutable coefficient arrays between supported object types
 */
class MultiAppFXTransfer : public MultiAppTransfer
{
public:
  MultiAppFXTransfer(const InputParameters & parameters);

  // Overrides from MultiAppTransfer
  virtual void execute() override;
  virtual void initialSetup() override;

protected:
  /// Name of the MutableCoefficientsInterface-derived object in the creating app
  const std::string _this_app_object_name;

  /// Name of the MutableCoefficientsInterface-derived object in the MultiApp
  const std::string _multi_app_object_name;

private:
  /**
   * Gets a MutableCoefficientsInterface-based Executioner, intented for use via function pointer
   */
  MutableCoefficientsInterface & getMutableCoefficientsExecutioner(
      FEProblemBase & base, const std::string & object_name = "", THREAD_ID thread = 0) const;

  /**
   * Gets a MutableCoefficientsInterface-based Function, intented for use via function pointer
   */
  MutableCoefficientsInterface & getMutableCoefficientsFunction(FEProblemBase & base,
                                                                const std::string & object_name,
                                                                THREAD_ID thread) const;

  /**
   * Gets a MutableCoefficientsInterface-based UserObject, intended for use via function pointer
   */
  MutableCoefficientsInterface & getMutableCoefficientsUserOject(FEProblemBase & base,
                                                                 const std::string & object_name,
                                                                 THREAD_ID thread) const;

  /**
   * Function pointer typedef for functions used to find, convert, and return the appropriate
   * MutableCoefficientsInterface object from an FEProblemBase.
   */
  typedef MutableCoefficientsInterface & (MultiAppFXTransfer::*GetProblemObject)(
      FEProblemBase & base, const std::string & object_name, THREAD_ID thread) const;

protected:
  /**
   * Searches an FEProblemBase for a MutableCoefficientsInterface-based object and returns a
   * function pointer to the matched function type.
   */
  virtual GetProblemObject scanProblemBaseForObject(FEProblemBase & base,
                                                    const std::string & object_name,
                                                    const std::string & app_name) const;

  /// Function pointer for grabbing the MultiApp object
  GetProblemObject getMultiAppObject;

  /// Function pointer for grabbing the SubApp object
  GetProblemObject getSubAppObject;
};

#endif // MULTIAPPMUTABLECOEFFICIENTSTRANSFER_H
