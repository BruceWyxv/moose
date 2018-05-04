//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef FUNCTIONSERIES_H
#define FUNCTIONSERIES_H

#include "FXInputParameters.h"
#include "MutableCoefficientsFunctionInterface.h"

class FunctionSeries;

template <>
InputParameters validParams<FunctionSeries>();

/**
 * This class uses implementations of CompositeSeriesBasisInterface to generate a function based on
 * convolved function series. Its inheritance tree includes MutableCoefficientsInterface, which
 * enables easy MultiApp transfers of coefficients.
 */
class FunctionSeries : public FXInputParameters, public MutableCoefficientsFunctionInterface
{
public:
  FunctionSeries(const InputParameters & parameters);

  /**
   * Static function to cast a Function to SeriesFunction
   */
  static FunctionSeries & checkAndConvertFunction(Function & function,
                                                  const std::string & typeName,
                                                  const std::string & objectName);

  // Override from MemoizedFunctionInterface
  virtual Real evaluateValue(Real t, const Point & p) override;

  /**
   * Expand the function series at the current location and with the current coefficients
   */
  Real expand();

  /**
   * Expand the function using the provided coefficients at the current location
   */
  Real expand(const std::vector<Real> & coefficients);

  /**
   * Returns the number of terms (coefficients) in the underlying function series
   */
  std::size_t getNumberOfTerms() const;

  /**
   * Returns the volume of evaluation in the functional series standardized space
   */
  Real getStandardizedFunctionVolume() const;

  /**
   * Returns a vector of the functional orders in the underlying functional series
   */
  const std::vector<std::size_t> & getOrders() const;

  /**
   * Returns a vector of the generation-evaluated functional series at the current location
   */
  const std::vector<Real> & getGeneration();

  /**
   * Returns a vector of the expansion-evaluated functional series at the current location
   */
  const std::vector<Real> & getExpansion();

  /**
   * Returns true if the provided point is within the set physical boundaries
   */
  bool isInPhysicalBounds(const Point & point) const;

  /**
   * Set the current evaluation location
   */
  void setLocation(const Point & point);

  /**
   * Returns a tabularized text stream of the currently stored coefficients
   */
  friend std::ostream & operator<<(std::ostream & stream, const FunctionSeries & me);

protected:
  /// The normalization type for expansion
  const MooseEnum & _expansion_type;
  /// The normalization type for generation
  const MooseEnum & _generation_type;
};

#endif
