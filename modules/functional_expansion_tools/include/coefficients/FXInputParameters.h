//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef FXINPUTPARAMETERS_H
#define FXINPUTPARAMETERS_H

#include "InputParameters.h"

#include "CompositeSeriesBasisInterface.h"

class FXInputParameters;

template <>
InputParameters validParams<FXInputParameters>();

class FXInputParameters
{
public:
  FXInputParameters(const InputParameters & parameters);

protected:
  /// The vector holding the orders of each single series
  const std::vector<std::size_t> _orders;

  /// The physical bounds of the function series
  const std::vector<Real> _physical_bounds;

  /// Stores a pointer to the functional series object
  std::unique_ptr<CompositeSeriesBasisInterface> _series_type;

  /// Stores the name of the current functional series type
  const MooseEnum & _series_type_name;

  /*
   * Enumerations of the possible series types for the different spatial expansions. Not all of
   * these will be provided for any one series.
   */
  /// Stores the name of the single function series to use in the x direction
  const MooseEnum & _x;
  /// Stores the name of the single function series to use in the y direction
  const MooseEnum & _y;
  /// Stores the name of the single function series to use in the z direction
  const MooseEnum & _z;
  /// Stores the name of the single function series to use for a unit disc
  const MooseEnum & _disc;

private:
  /**
   * Static function to convert an array of `unsigned int` to `std::size_t`. The MOOSE parser has
   * issues reading a list of integers in as `std::size_t` (unsigned long), so this workaround is
   * required in order to set `_orders` in the constructor initializer list.
   */
  static std::vector<std::size_t> convertOrders(const std::vector<unsigned int> & orders);
};

#endif // FXINPUTPARAMETERS_H
