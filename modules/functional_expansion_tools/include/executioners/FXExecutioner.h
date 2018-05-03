//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef FXEXECUTIONER_H
#define FXEXECUTIONER_H

#include "Transient.h"

#include "MutableCoefficientsInterface.h"

class FXExecutioner;

template <>
InputParameters validParams<FXExecutioner>();

class FXExecutioner : public Transient, public MutableCoefficientsInterface
{
public:
  FXExecutioner(const InputParameters & parameters);

protected:
  /**
   * Must be defined by an implementor to export the coefficients from the coefficient array into a
   * format digestible by the wrapped process
   */
  virtual void exportCoefficients(const std::vector<Real> & out_coefficients) = 0;

  /**
   * Must be defiined by an implementor to import the coefficients from the wrapped process and load
   * them into the coefficient array
   */
  virtual void importCoefficients(std::vector<Real> & array_to_fill) = 0;

  /**
   * Calls importCoefficients()
   */
  virtual void postExecute() final;

  /**
   * Calls exportCoefficients()
   */
  virtual void preExecute() final;

  /// A flag that indicates if this is the first run or not
  bool _first_run;

private:
  /*
   * Hide the setter methods to donwstream implementors: we want them to rely solely on the members
   * exportCoefficients() and importCoefficients().
   *
   * MutliAppFXTransfer will still be able to access these through the MutableCoefficientsInterface
   * reference
   */
  using MutableCoefficientsInterface::_characteristics;
  using MutableCoefficientsInterface::_coefficients;
  using MutableCoefficientsInterface::_enforce_size;
  using MutableCoefficientsInterface::coefficientsChanged;
  using MutableCoefficientsInterface::enforceSize;
  using MutableCoefficientsInterface::importCoefficients;
  using MutableCoefficientsInterface::resize;
  using MutableCoefficientsInterface::setCharacteristics;
  using MutableCoefficientsInterface::setCoefficients;
};

#endif // FXEXECUTIONER_H
